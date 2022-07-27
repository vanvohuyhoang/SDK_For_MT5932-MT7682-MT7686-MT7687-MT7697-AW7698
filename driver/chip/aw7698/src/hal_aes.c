/* Copyright Statement:
 *
 * (C) 2019  Airoha Technology Corp. All rights reserved.
 *
 * This software/firmware and related documentation ("Airoha Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to Airoha Technology Corp. ("Airoha") and/or its licensors.
 * Without the prior written permission of Airoha and/or its licensors,
 * any reproduction, modification, use or disclosure of Airoha Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 * You may only use, reproduce, modify, or distribute (as applicable) Airoha Software
 * if you have agreed to and been bound by the applicable license agreement with
 * Airoha ("License Agreement") and been granted explicit permission to do so within
 * the License Agreement ("Permitted User").  If you are not a Permitted User,
 * please cease any access or use of Airoha Software immediately.
 * BY OPENING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 * THAT AIROHA SOFTWARE RECEIVED FROM AIROHA AND/OR ITS REPRESENTATIVES
 * ARE PROVIDED TO RECEIVER ON AN "AS-IS" BASIS ONLY. AIROHA EXPRESSLY DISCLAIMS ANY AND ALL
 * WARRANTIES, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR NONINFRINGEMENT.
 * NEITHER DOES AIROHA PROVIDE ANY WARRANTY WHATSOEVER WITH RESPECT TO THE
 * SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY, INCORPORATED IN, OR
 * SUPPLIED WITH AIROHA SOFTWARE, AND RECEIVER AGREES TO LOOK ONLY TO SUCH
 * THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO. RECEIVER EXPRESSLY ACKNOWLEDGES
 * THAT IT IS RECEIVER'S SOLE RESPONSIBILITY TO OBTAIN FROM ANY THIRD PARTY ALL PROPER LICENSES
 * CONTAINED IN AIROHA SOFTWARE. AIROHA SHALL ALSO NOT BE RESPONSIBLE FOR ANY AIROHA
 * SOFTWARE RELEASES MADE TO RECEIVER'S SPECIFICATION OR TO CONFORM TO A PARTICULAR
 * STANDARD OR OPEN FORUM. RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND AIROHA'S ENTIRE AND
 * CUMULATIVE LIABILITY WITH RESPECT TO AIROHA SOFTWARE RELEASED HEREUNDER WILL BE,
 * AT AIROHA'S OPTION, TO REVISE OR REPLACE AIROHA SOFTWARE AT ISSUE,
 * OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE CHARGE PAID BY RECEIVER TO
 * AIROHA FOR SUCH AIROHA SOFTWARE AT ISSUE.
 */
 
#include "hal_aes.h"

#ifdef HAL_AES_MODULE_ENABLED

#include <string.h>
#include "hal_define.h"
#include "aw7698.h"
#include "crypt_aes.h"
#include "hal_log.h"
#include "hal_crypt_internal.h"
#include "hal_nvic.h"
#include "hal_clock.h"

/* global lock used to protect the crypto engine */
bool g_crypt_lock = CRYPT_UNLOCK;

/* below variables will be used for crypto hardware so must be placed in physical memory address*/
ATTR_RWDATA_IN_NONCACHED_SYSRAM_4BYTE_ALIGN uint8_t *init_vector2;
ATTR_RWDATA_IN_NONCACHED_SYSRAM_4BYTE_ALIGN uint8_t data_block[HAL_AES_BLOCK_SIZES];

/* internal function for getting lock, -100 means the crypto engine is not available */
int32_t hal_crypt_lock_take(void)
{
    uint32_t irq_status;
    irq_status = save_and_set_interrupt_mask();
    if (g_crypt_lock == CRYPT_LOCK) {
        restore_interrupt_mask(irq_status);
        return -100;
    }
    g_crypt_lock = CRYPT_LOCK;
    restore_interrupt_mask(irq_status);
    return 0;
}


/* internal function for releasing lock */
void hal_crypt_lock_give(void)
{
    g_crypt_lock = CRYPT_UNLOCK;
}

volatile static bool g_aes_op_done = false;

/* It's about 10s at 192MHz CPU clock */
#define HAL_AES_MAX_WAIT_COUNT 0x10000000


static void aes_operation_done(hal_nvic_irq_t irq_number)
{
    uint32_t irq_status;

    hal_nvic_disable_irq((IRQn_Type)CRYPTO_IRQn);
    irq_status = save_and_set_interrupt_mask();

//    hal_nvic_clear_pending_irq((IRQn_Type)CRYPTO_IRQn);

    g_aes_op_done = true;

    // clear interrupt status bit
    *CRYPTO_ENGINE_STA_BASE |= (1 << CRYPTO_ENGINE_STA_INT_CLR);

    restore_interrupt_mask(irq_status);
    hal_nvic_enable_irq((IRQn_Type)CRYPTO_IRQn);
}

static hal_aes_status_t do_aes_encrypt(uint8_t *encrypt_buffer,
                                       uint32_t encrypt_buffer_length,
                                       uint8_t *plain_buffer,
                                       uint32_t plain_buffer_length,
                                       uint8_t init_vector[HAL_AES_CBC_IV_LENGTH])
{
    g_aes_op_done = false;
    int32_t ret_val = aes_operate(encrypt_buffer,
                                  encrypt_buffer_length,
                                  plain_buffer,
                                  plain_buffer_length,
                                  init_vector,
                                  AES_MODE_ENCRYPT);
    if (ret_val < 0) {
        log_hal_error("aes_operate fail.");
        return HAL_AES_STATUS_ERROR;
    }

#if 1    
    uint32_t wait_count = 0;
    while (!g_aes_op_done) {
        //simple wait
        wait_count++;
        if (wait_count > HAL_AES_MAX_WAIT_COUNT) {
            log_hal_error("wait for encrypt timeout.");
            return HAL_AES_STATUS_ERROR;
        }
    }

    // clear interrupt status bit, done in the interrupt callback
#else
    while((*CRYPTO_ENGINE_CTRL_BASE) & (1 << ENGINE_CTRL_START_OFFSET));

    // clear interrupt status bit
    *CRYPTO_ENGINE_STA_BASE |= (1 << CRYPTO_ENGINE_STA_INT_CLR);
#endif
    return HAL_AES_STATUS_OK;
}

static hal_aes_status_t do_aes_decrypt(hal_aes_buffer_t *plain_text,
                                       hal_aes_buffer_t *encrypted_text,
                                       uint8_t init_vector[HAL_AES_CBC_IV_LENGTH])
{
    g_aes_op_done = false;

    int32_t ret_val = aes_operate(encrypted_text->buffer,
                                  encrypted_text->length,
                                  plain_text->buffer,
                                  plain_text->length,
                                  init_vector,
                                  AES_MODE_DECRYPT);
    if (ret_val < 0) {
        log_hal_error("aes_operate fail.");
        return HAL_AES_STATUS_ERROR;
    }

#if 1
    uint32_t wait_count = 0;
    while (!g_aes_op_done) {
        //simple wait
        wait_count++;
        if (wait_count > HAL_AES_MAX_WAIT_COUNT) {
            log_hal_error("wait for decrypt timeout.");
            return HAL_AES_STATUS_ERROR;
        }
    }

    // clear interrupt status bit, done in the interrupt callback
#else
    while((*CRYPTO_ENGINE_CTRL_BASE) & (1 << ENGINE_CTRL_START_OFFSET));

    // clear interrupt status bit
    *CRYPTO_ENGINE_STA_BASE |= (1 << CRYPTO_ENGINE_STA_INT_CLR);
#endif
    return HAL_AES_STATUS_OK;
}

/* internal common function */
hal_aes_status_t hal_aes_encrypt_with_padding(hal_aes_buffer_t *encrypted_text,
                                     hal_aes_buffer_t *plain_text,
                                     hal_aes_buffer_t *key,
                                     uint8_t init_vector[HAL_AES_CBC_IV_LENGTH],
                                     uint8_t mode)
{
    if ((NULL == encrypted_text)
            || (NULL == plain_text)
            || (NULL == key)
            || (NULL == init_vector)) {
        log_hal_error("NULL input.");
        return HAL_AES_STATUS_ERROR;
    }
    uint32_t last_block_size = plain_text->length % HAL_AES_BLOCK_SIZES;
    uint32_t block_num = plain_text->length / HAL_AES_BLOCK_SIZES;
    uint8_t padding_size = HAL_AES_BLOCK_SIZES - last_block_size;
    uint8_t *iv;

    if (encrypted_text->length < (plain_text->length + padding_size)) {
        log_hal_error("Inadequate encrypted buffer.");
        return HAL_AES_STATUS_ERROR;
    }

    if (hal_crypt_lock_take() == -100)
        return HAL_AES_STATUS_BUSY;
    
    hal_nvic_register_isr_handler((IRQn_Type)CRYPTO_IRQn, aes_operation_done);
//    hal_nvic_set_priority((IRQn_Type)CRYPTO_IRQn, CM4_MTK_CRYPTO_PRI);
    hal_nvic_enable_irq((IRQn_Type)CRYPTO_IRQn);

    aes_set_key((uint8_t)key->length, key->buffer);

    if (AES_TYPE_CBC == mode)  /* AES_TYPE_CBC */
    {
        aes_configure(1, 1); /* 1:software 0:ECB and 1:CBC */
        //aes_configure(2, 1); /* 2:hw key 0:ECB and 1:CBC */
    } else { /* AES_TYPE_CBC */
        aes_configure(1, 0); /* 1:software 0:ECB and 1:CBC */
        //aes_configure(2, 0); /* 2:hw key 0:ECB and 1:CBC */
    }

    if (block_num > 0) {
        uint32_t first_encypt_size = block_num * HAL_AES_BLOCK_SIZES;
        if (HAL_AES_STATUS_OK != do_aes_encrypt(encrypted_text->buffer,
                                                encrypted_text->length,
                                                plain_text->buffer,
                                                first_encypt_size,
                                                init_vector)) {
            log_hal_error("do_aes_encrypt fail.");
            hal_crypt_lock_give();
            return HAL_AES_STATUS_ERROR;
        }

        memset(data_block, 0x00, HAL_AES_BLOCK_SIZES);
        memcpy(data_block, plain_text->buffer + first_encypt_size, last_block_size);
        memset(data_block + last_block_size, padding_size, padding_size);

        if (AES_TYPE_CBC == mode)  /* do 2nd aes cbc operation need to input newer iv */
        {
            init_vector2 = encrypted_text->buffer + first_encypt_size - HAL_AES_BLOCK_SIZES;
            iv = init_vector2;
        } else {
            iv = init_vector;
        }

        if (HAL_AES_STATUS_OK != do_aes_encrypt(encrypted_text->buffer + first_encypt_size,
                                                encrypted_text->length,
                                                data_block,
                                                HAL_AES_BLOCK_SIZES,
                                                iv)) {
            log_hal_error("do_aes_encrypt fail.");
            hal_crypt_lock_give();
            return HAL_AES_STATUS_ERROR;
        }
    } else {
        memset(data_block, 0x00, HAL_AES_BLOCK_SIZES);
        memcpy(data_block, plain_text->buffer, plain_text->length);
        memset(data_block + last_block_size, padding_size, padding_size);
        if (HAL_AES_STATUS_OK != do_aes_encrypt(encrypted_text->buffer,
                                                encrypted_text->length,
                                                data_block,
                                                HAL_AES_BLOCK_SIZES,
                                                init_vector)) {
            log_hal_error("do_aes_encrypt fail.");
            hal_crypt_lock_give();
            return HAL_AES_STATUS_ERROR;
        }
    }

    encrypted_text->length = (block_num + 1) * HAL_AES_BLOCK_SIZES;

    hal_crypt_lock_give();
    return HAL_AES_STATUS_OK;
}

hal_aes_status_t hal_aes_decrypt(hal_aes_buffer_t *plain_text,
                                     hal_aes_buffer_t *encrypted_text,
                                     hal_aes_buffer_t *key,
                                     uint8_t init_vector[HAL_AES_CBC_IV_LENGTH],
                                     uint8_t mode)
{
    if ((NULL == plain_text)
            || (NULL == encrypted_text)
            || (NULL == key)
            || (NULL == init_vector)) {
        log_hal_error("NULL input.");
        return HAL_AES_STATUS_ERROR;
    }

    if ((encrypted_text->length % HAL_AES_BLOCK_SIZES) != 0) {
        log_hal_error("Invalid encrypted text length: %lu.", encrypted_text->length);
        return HAL_AES_STATUS_ERROR;
    }
    if (plain_text->length < (encrypted_text->length - HAL_AES_BLOCK_SIZES)) {
        log_hal_error("Plain text buffer lengthL %lu is too small, encrypted length is: %lu",
                      encrypted_text->length, encrypted_text->length);
        return HAL_AES_STATUS_ERROR;
    }
    if ((key->length != HAL_AES_KEY_LENGTH_128)
            && (key->length != HAL_AES_KEY_LENGTH_192)
            && (key->length != HAL_AES_KEY_LENGTH_256)) {
        log_hal_error("key length is %lu, invalid. It has to be 16, 24 or 32.", key->length);
        return HAL_AES_STATUS_ERROR;
    }

    if (hal_crypt_lock_take() == -100)
        return HAL_AES_STATUS_BUSY;

    hal_nvic_register_isr_handler((IRQn_Type)CRYPTO_IRQn, aes_operation_done);
//    hal_nvic_set_priority((IRQn_Type)CRYPTO_IRQn, CM4_MTK_CRYPTO_PRI);
    hal_nvic_enable_irq((IRQn_Type)CRYPTO_IRQn);

    aes_set_key(key->length, key->buffer);
    if (AES_TYPE_CBC == mode)  /* AES_TYPE_CBC */
    {
        aes_configure(1, 1); /* 1:software 0:ECB and 1:CBC */
        //aes_configure(2, 1); /* 2:hw key 0:ECB and 1:CBC */
    } else { /* AES_TYPE_CBC */
        aes_configure(1, 0); /* 1:software 0:ECB and 1:CBC */
        //aes_configure(2, 0); /* 2:hw key 0:ECB and 1:CBC */
    }


    if (HAL_AES_STATUS_OK != do_aes_decrypt(plain_text, encrypted_text, init_vector)) {
        log_hal_error("do_aes_decrypt fail");
        hal_crypt_lock_give();
        return HAL_AES_STATUS_ERROR;
    }

    uint8_t padding_size = plain_text->buffer[encrypted_text->length - 1];
    plain_text->length = encrypted_text->length - padding_size;

    hal_crypt_lock_give();
    return HAL_AES_STATUS_OK;
}

hal_aes_status_t hal_aes_cbc_encrypt(hal_aes_buffer_t *encrypted_text,
                                     hal_aes_buffer_t *plain_text,
                                     hal_aes_buffer_t *key,
                                     uint8_t init_vector[HAL_AES_CBC_IV_LENGTH])
{
    hal_aes_status_t status; 

    hal_clock_enable(HAL_CLOCK_CG_CRYPTO);
    status = hal_aes_encrypt_with_padding(encrypted_text, plain_text, key, init_vector, AES_TYPE_CBC);    
    hal_clock_disable(HAL_CLOCK_CG_CRYPTO);

    return status;
}

hal_aes_status_t hal_aes_cbc_decrypt(hal_aes_buffer_t *plain_text,
                                     hal_aes_buffer_t *encrypted_text,
                                     hal_aes_buffer_t *key,
                                     uint8_t init_vector[HAL_AES_CBC_IV_LENGTH])
{
    hal_aes_status_t status; 

    hal_clock_enable(HAL_CLOCK_CG_CRYPTO);
    status = hal_aes_decrypt(plain_text, encrypted_text, key, init_vector, AES_TYPE_CBC);
    hal_clock_disable(HAL_CLOCK_CG_CRYPTO);
    return status;
}

hal_aes_status_t hal_aes_ecb_encrypt(hal_aes_buffer_t *encrypted_text,
                                     hal_aes_buffer_t *plain_text,
                                     hal_aes_buffer_t *key)
{
    uint8_t init_vector[16] = {0};
    hal_aes_status_t status; 

    hal_clock_enable(HAL_CLOCK_CG_CRYPTO);
    status = hal_aes_encrypt_with_padding(encrypted_text, plain_text, key, init_vector, AES_TYPE_ECB);    
    hal_clock_disable(HAL_CLOCK_CG_CRYPTO);

    return status;
}

hal_aes_status_t hal_aes_ecb_decrypt(hal_aes_buffer_t *plain_text,
                                     hal_aes_buffer_t *encrypted_text,
                                     hal_aes_buffer_t *key)
{
    uint8_t init_vector[HAL_AES_CBC_IV_LENGTH] = {0};
    hal_aes_status_t status; 

    hal_clock_enable(HAL_CLOCK_CG_CRYPTO);
    status = hal_aes_decrypt(plain_text, encrypted_text, key, init_vector, AES_TYPE_ECB);
    hal_clock_disable(HAL_CLOCK_CG_CRYPTO);

    return status;
}

/* internal common function */
hal_aes_status_t hal_aes_encrypt_with_padding_ex(hal_aes_buffer_t *encrypted_text,
                                                 hal_aes_buffer_t *plain_text,
                                                 hal_aes_buffer_t *key,
                                                 uint8_t init_vector[HAL_AES_CBC_IV_LENGTH],
                                                 uint8_t mode,
                                                 uint8_t key_src,
                                                 uint8_t key_slot)
{
    if ( (NULL == plain_text) ||
         (NULL == encrypted_text) ||
         (NULL == key) ||
         (NULL == init_vector) )
    {
        log_hal_error("NULL input.");
        return HAL_AES_STATUS_ERROR;
    }

    if(key_src >= HAL_AES_KEY_SOURCE_NUM)
    {
        log_hal_error("Invalid key source.");
        return HAL_AES_STATUS_ERROR;
    }

    uint32_t last_block_size = plain_text->length  % HAL_AES_BLOCK_SIZES;
    uint32_t block_num       = plain_text->length  / HAL_AES_BLOCK_SIZES;
    uint8_t  padding_size    = HAL_AES_BLOCK_SIZES - last_block_size;
    uint8_t  *iv;

    if (encrypted_text->length < (plain_text->length + padding_size)) {
        log_hal_error("Inadequate encrypted buffer.");
        return HAL_AES_STATUS_ERROR;
    }

    if (hal_crypt_lock_take() == -100)
        return HAL_AES_STATUS_BUSY;

    hal_nvic_register_isr_handler((IRQn_Type)CRYPTO_IRQn, aes_operation_done);
    // hal_nvic_set_priority((IRQn_Type)CRYPTO_IRQn, CM4_MTK_CRYPTO_PRI);
    hal_nvic_enable_irq((IRQn_Type)CRYPTO_IRQn);

    // aes_configure ( 0:FUSE0 1:FUSE1 , 0:ECB 1:CBC )
    aes_set_key((uint8_t)key->length, key->buffer);
    if (AES_TYPE_CBC == mode) {
        if (HAL_AES_KEY_SOURCE_EFUSE1 == key_src || HAL_AES_KEY_SOURCE_EFUSE2 == key_src) {
            aes_configure(0, 1);
            aes_set_key_slot(key_slot);
        } else {
            aes_configure(0, 1);
        }
    } else {
        if (HAL_AES_KEY_SOURCE_EFUSE1 == key_src || HAL_AES_KEY_SOURCE_EFUSE2 == key_src) {
            aes_configure(0, 0);
            aes_set_key_slot(key_slot);
        } else {
            aes_configure(0, 0);
        }
    }

    if (block_num > 0) {
        uint32_t first_encypt_size = block_num * HAL_AES_BLOCK_SIZES;

        if (HAL_AES_STATUS_OK != do_aes_encrypt(encrypted_text->buffer,
                                                encrypted_text->length,
                                                plain_text->buffer,
                                                first_encypt_size,
                                                init_vector)) {
            log_hal_error("do_aes_encrypt fail.");
            hal_crypt_lock_give();
            hal_nvic_disable_irq((IRQn_Type)CRYPTO_IRQn);
            return HAL_AES_STATUS_ERROR;
        }

        memset(data_block, 0x00, HAL_AES_BLOCK_SIZES);
        memcpy(data_block, plain_text->buffer + first_encypt_size, last_block_size);
        memset(data_block + last_block_size, padding_size, padding_size);

        if (AES_TYPE_CBC == mode){  // do 2nd aes cbc operation need to input newer iv
            init_vector2 = encrypted_text->buffer + first_encypt_size - HAL_AES_BLOCK_SIZES;
            iv = init_vector2;
        } else {
            iv = init_vector;
        }

        if (HAL_AES_STATUS_OK != do_aes_encrypt(encrypted_text->buffer + first_encypt_size,
                                                encrypted_text->length,
                                                data_block,
                                                HAL_AES_BLOCK_SIZES,
                                                iv)) {
            log_hal_error("do_aes_encrypt fail.");
            hal_crypt_lock_give();
            hal_nvic_disable_irq((IRQn_Type)CRYPTO_IRQn);
            return HAL_AES_STATUS_ERROR;
        }
    } else {
        memset(data_block, 0x00, HAL_AES_BLOCK_SIZES);
        memcpy(data_block, plain_text->buffer, plain_text->length);
        memset(data_block + last_block_size, padding_size, padding_size);
        if (HAL_AES_STATUS_OK != do_aes_encrypt(encrypted_text->buffer,
                                                encrypted_text->length,
                                                data_block,
                                                HAL_AES_BLOCK_SIZES,
                                                init_vector)) {
            log_hal_error("do_aes_encrypt fail.");
            hal_crypt_lock_give();
            hal_nvic_disable_irq((IRQn_Type)CRYPTO_IRQn);
            return HAL_AES_STATUS_ERROR;
        }
    }

    encrypted_text->length = (block_num + 1) * HAL_AES_BLOCK_SIZES;

    hal_crypt_lock_give();
    hal_nvic_disable_irq((IRQn_Type)CRYPTO_IRQn);
    return HAL_AES_STATUS_OK;
}

hal_aes_status_t hal_aes_decrypt_ex(hal_aes_buffer_t *plain_text,
                                    hal_aes_buffer_t *encrypted_text,
                                    hal_aes_buffer_t *key,
                                    uint8_t init_vector[HAL_AES_CBC_IV_LENGTH],
                                    uint8_t mode,
                                    uint8_t key_src,
                                    uint8_t key_slot)
{
    if((NULL == plain_text) ||
       (NULL == encrypted_text) ||
       (NULL == key) ||
       (NULL == init_vector))
    {
        log_hal_error("NULL input.");
        return HAL_AES_STATUS_ERROR;
    }
	
    if(key_src >= HAL_AES_KEY_SOURCE_NUM)
    {
        log_hal_error("Invalid key source.");
        return HAL_AES_STATUS_ERROR;
    }

    if ((encrypted_text->length % HAL_AES_BLOCK_SIZES) != 0) {
        log_hal_error("Invalid encrypted text length: %lu.", encrypted_text->length);
        return HAL_AES_STATUS_ERROR;
    }
    if (plain_text->length < (encrypted_text->length - HAL_AES_BLOCK_SIZES)) {
        log_hal_error("Plain text buffer lengthL %lu is too small, encrypted length is: %lu",
                      encrypted_text->length, encrypted_text->length);
        return HAL_AES_STATUS_ERROR;
    }
    if ((key->length != HAL_AES_KEY_LENGTH_128) &&
        (key->length != HAL_AES_KEY_LENGTH_192) &&
        (key->length != HAL_AES_KEY_LENGTH_256))
    {
        log_hal_error("key length is %lu, invalid. It has to be 16, 24 or 32.", key->length);
        return HAL_AES_STATUS_ERROR;
    }

    if (hal_crypt_lock_take() == -100)
        return HAL_AES_STATUS_BUSY;

    hal_nvic_register_isr_handler((IRQn_Type)CRYPTO_IRQn, aes_operation_done);
    // hal_nvic_set_priority((IRQn_Type)CRYPTO_IRQn, CM4_MTK_CRYPTO_PRI);
    hal_nvic_enable_irq((IRQn_Type)CRYPTO_IRQn);

    // aes_configure ( 0:FUSE0 1:FUSE1 , 0:ECB 1:CBC )
    aes_set_key((uint8_t)key->length, key->buffer);
    if (AES_TYPE_CBC == mode) {
        if (HAL_AES_KEY_SOURCE_EFUSE1 == key_src || HAL_AES_KEY_SOURCE_EFUSE2 == key_src){
            aes_configure(0, 1);
            aes_set_key_slot(key_slot);
        } else  {
            aes_configure(0, 1);
        }
    } else {
        if (HAL_AES_KEY_SOURCE_EFUSE1 == key_src || HAL_AES_KEY_SOURCE_EFUSE2 == key_src){
            aes_configure(0, 0);
            aes_set_key_slot(key_slot);
        } else {
            aes_configure(0, 1);
        }
    }

    if (HAL_AES_STATUS_OK != do_aes_decrypt(plain_text, encrypted_text, init_vector)) {
        log_hal_error("do_aes_decrypt fail");
        hal_crypt_lock_give();
        hal_nvic_disable_irq((IRQn_Type)CRYPTO_IRQn);
        return HAL_AES_STATUS_ERROR;
    }

    uint8_t padding_size = plain_text->buffer[encrypted_text->length - 1];
    plain_text->length = encrypted_text->length - padding_size;

    hal_crypt_lock_give();
    hal_nvic_disable_irq((IRQn_Type)CRYPTO_IRQn);
    return HAL_AES_STATUS_OK;
}

hal_aes_status_t hal_aes_cbc_encrypt_ex(hal_aes_buffer_t *encrypted_text,
                                        hal_aes_buffer_t *plain_text,
                                        hal_aes_buffer_t *key,
                                        uint8_t init_vector[HAL_AES_CBC_IV_LENGTH],
                                        hal_aes_key_src  key_src)
{
    hal_aes_status_t status;

    // key_slot: 0 or 1
    uint8_t key_slot = (key_src % 2) + 1;

    hal_clock_enable(HAL_CLOCK_CG_CRYPTO);
    status = hal_aes_encrypt_with_padding_ex(encrypted_text, plain_text, key, init_vector, AES_TYPE_CBC, key_src, key_slot);
    hal_clock_disable(HAL_CLOCK_CG_CRYPTO);

    return status;
}

hal_aes_status_t hal_aes_cbc_decrypt_ex(hal_aes_buffer_t *plain_text,
                                        hal_aes_buffer_t *encrypted_text,
                                        hal_aes_buffer_t *key,
                                        uint8_t init_vector[HAL_AES_CBC_IV_LENGTH],
                                        hal_aes_key_src  key_src)
{
    hal_aes_status_t status;

    // key_slot: 0 or 1
    uint8_t key_slot = (key_src % 2) + 1;

    hal_clock_enable(HAL_CLOCK_CG_CRYPTO);
    status = hal_aes_decrypt_ex(plain_text, encrypted_text, key, init_vector, AES_TYPE_CBC, key_src, key_slot);
    hal_clock_disable(HAL_CLOCK_CG_CRYPTO);

    return status;
}

hal_aes_status_t hal_aes_ecb_encrypt_ex(hal_aes_buffer_t *encrypted_text,
                                        hal_aes_buffer_t *plain_text,
                                        hal_aes_buffer_t *key,
                                        hal_aes_key_src  key_src)
{
    uint8_t init_vector[HAL_AES_CBC_IV_LENGTH] = {0};
    hal_aes_status_t status;

    // key_slot: 0 or 1
    uint8_t key_slot = (key_src % 2) + 1;

    hal_clock_enable(HAL_CLOCK_CG_CRYPTO);
    status = hal_aes_encrypt_with_padding_ex(encrypted_text, plain_text, key, init_vector, AES_TYPE_ECB, key_src, key_slot);
    hal_clock_disable(HAL_CLOCK_CG_CRYPTO);

    return status;
}

hal_aes_status_t hal_aes_ecb_decrypt_ex(hal_aes_buffer_t *plain_text,
                                        hal_aes_buffer_t *encrypted_text,
                                        hal_aes_buffer_t *key,
                                        hal_aes_key_src  key_src)
{
    uint8_t init_vector[HAL_AES_CBC_IV_LENGTH] = {0};
    hal_aes_status_t status;

    // key_slot: 0 or 1
    uint8_t key_slot = (key_src % 2) + 1;

    hal_clock_enable(HAL_CLOCK_CG_CRYPTO);
    status = hal_aes_decrypt_ex(plain_text, encrypted_text, key, init_vector, AES_TYPE_ECB, key_src, key_slot);
    hal_clock_disable(HAL_CLOCK_CG_CRYPTO);

    return status;
}

#endif /* HAL_AES_MODULE_ENABLED */

