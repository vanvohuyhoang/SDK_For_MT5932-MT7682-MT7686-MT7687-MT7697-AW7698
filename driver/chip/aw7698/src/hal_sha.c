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
 
#include "hal_sha.h"

#ifdef HAL_SHA_MODULE_ENABLED

#include "stddef.h"
#include "crypt_sha.h"
#include "hal_log.h"
#include "hal_crypt_internal.h"

void hal_sha_enable_polling_mode(){
    crypt_sha_enable_polling_mode();
}

void hal_sha_disable_polling_mode(){
    crypt_sha_disable_polling_mode();
}

hal_sha_status_t hal_sha1_init(hal_sha1_context_t *context)
{
    hal_sha_status_t status;

    if (hal_crypt_lock_take() == -100)
        return HAL_SHA_STATUS_BUSY;
    status = crypt_sha_init((SHA_CTX_STRUC *)context, HAL_SHA_INTERNAL_SHA1);
    hal_crypt_lock_give();

    return status;
}

hal_sha_status_t hal_sha1_append(hal_sha1_context_t *context, uint8_t *message, uint32_t length)
{
    if (NULL == context) {
        log_hal_error("context is NULL.");
        return HAL_SHA_STATUS_ERROR;
    }
    if (NULL == message) {
        log_hal_error("message is NULL.");
        return HAL_SHA_STATUS_ERROR;
    }

    if (hal_crypt_lock_take() == -100)
        return HAL_SHA_STATUS_BUSY;

    while (1) {
        if (length >= MAX_APPEND_LENGTH) {
            crypt_sha_append((SHA1_CTX_STRUC *)context, message, MAX_APPEND_LENGTH, HAL_SHA_INTERNAL_SHA1);
            message += MAX_APPEND_LENGTH;
            length -= MAX_APPEND_LENGTH;
        } else {
            crypt_sha_append((SHA1_CTX_STRUC *)context, message, length, HAL_SHA_INTERNAL_SHA1);
            break;
        }
    }

    hal_crypt_lock_give();
    return HAL_SHA_STATUS_OK;
}

hal_sha_status_t hal_sha1_end(hal_sha1_context_t *context, uint8_t digest_message[HAL_SHA1_DIGEST_SIZE])
{
    hal_sha_status_t status;

    if (hal_crypt_lock_take() == -100)
        return HAL_SHA_STATUS_BUSY;
    status = crypt_sha_end((SHA1_CTX_STRUC *)context, digest_message, HAL_SHA_INTERNAL_SHA1);
    hal_crypt_lock_give();
    return status;
}

hal_sha_status_t hal_sha224_init(hal_sha224_context_t *context)
{
    hal_sha_status_t status;

    if (hal_crypt_lock_take() == -100)
        return HAL_SHA_STATUS_BUSY;
    status = crypt_sha_init((SHA_CTX_STRUC *)context, HAL_SHA_INTERNAL_SHA224);
    hal_crypt_lock_give();

    return status;

}

hal_sha_status_t hal_sha224_append(hal_sha224_context_t *context, uint8_t *message, uint32_t length)
{
    if (NULL == context) {
        log_hal_error("context is NULL.");
        return HAL_SHA_STATUS_ERROR;
    }
    if (NULL == message) {
        log_hal_error("message is NULL.");
        return HAL_SHA_STATUS_ERROR;
    }

    if (hal_crypt_lock_take() == -100)
        return HAL_SHA_STATUS_BUSY;

    while (1) {
        if (length >= MAX_APPEND_LENGTH) {
            crypt_sha_append((SHA224_CTX_STRUC *)context, message, MAX_APPEND_LENGTH, HAL_SHA_INTERNAL_SHA224);
            message += MAX_APPEND_LENGTH;
            length -= MAX_APPEND_LENGTH;
        } else {
            crypt_sha_append((SHA224_CTX_STRUC *)context, message, length, HAL_SHA_INTERNAL_SHA224);
            break;
        }
    }

    hal_crypt_lock_give();
    return HAL_SHA_STATUS_OK;
}

hal_sha_status_t hal_sha224_end(hal_sha224_context_t *context, uint8_t digest_message[HAL_SHA224_DIGEST_SIZE])
{
    hal_sha_status_t status;
    if (hal_crypt_lock_take() == -100)
        return HAL_SHA_STATUS_BUSY;

    status = crypt_sha_end((SHA224_CTX_STRUC *)context, digest_message, HAL_SHA_INTERNAL_SHA224);

    hal_crypt_lock_give();
    return status;
}

hal_sha_status_t hal_sha256_init(hal_sha256_context_t *context)
{
    hal_sha_status_t status;

    if (hal_crypt_lock_take() == -100)
        return HAL_SHA_STATUS_BUSY;
    status = crypt_sha_init((SHA_CTX_STRUC *)context, HAL_SHA_INTERNAL_SHA256);
    hal_crypt_lock_give();

    return status;
}

hal_sha_status_t hal_sha256_append(hal_sha256_context_t *context, uint8_t *message, uint32_t length)
{
    if (NULL == context) {
        log_hal_error("context is NULL.");
        return HAL_SHA_STATUS_ERROR;
    }
    if (NULL == message) {
        log_hal_error("message is NULL.");
        return HAL_SHA_STATUS_ERROR;
    }

    if (hal_crypt_lock_take() == -100)
        return HAL_SHA_STATUS_BUSY;

    while (1) {
        if (length >= MAX_APPEND_LENGTH) {
            crypt_sha_append((SHA256_CTX_STRUC *)context, message, MAX_APPEND_LENGTH, HAL_SHA_INTERNAL_SHA256);
            message += MAX_APPEND_LENGTH;
            length -= MAX_APPEND_LENGTH;
        } else {
            crypt_sha_append((SHA256_CTX_STRUC *)context, message, length, HAL_SHA_INTERNAL_SHA256);
            break;
        }
    }

    hal_crypt_lock_give();
    return HAL_SHA_STATUS_OK;
}

hal_sha_status_t hal_sha256_end(hal_sha256_context_t *context, uint8_t digest_message[HAL_SHA256_DIGEST_SIZE])
{
    hal_sha_status_t status;

    if (hal_crypt_lock_take() == -100)
        return HAL_SHA_STATUS_BUSY;
    status = crypt_sha_end((SHA256_CTX_STRUC *)context, digest_message, HAL_SHA_INTERNAL_SHA256);
    hal_crypt_lock_give();

    return status;
}

hal_sha_status_t hal_sha384_init(hal_sha384_context_t *context)
{
    hal_sha_status_t status;

    if (hal_crypt_lock_take() == -100)
        return HAL_SHA_STATUS_BUSY;
    status = crypt_sha_init((SHA_CTX_STRUC *)context, HAL_SHA_INTERNAL_SHA384);
    hal_crypt_lock_give();

    return status;
}

hal_sha_status_t hal_sha384_append(hal_sha384_context_t *context, uint8_t *message, uint32_t length)
{
    if (NULL == context) {
        log_hal_error("context is NULL.");
        return HAL_SHA_STATUS_ERROR;
    }
    if (NULL == message) {
        log_hal_error("message is NULL.");
        return HAL_SHA_STATUS_ERROR;
    }

    if (hal_crypt_lock_take() == -100)
        return HAL_SHA_STATUS_BUSY;

    while (1) {
        if (length >= MAX_APPEND_LENGTH) {
            crypt_sha_append((SHA384_CTX_STRUC *)context, message, MAX_APPEND_LENGTH, HAL_SHA_INTERNAL_SHA384);
            message += MAX_APPEND_LENGTH;
            length -= MAX_APPEND_LENGTH;
        } else {
            crypt_sha_append((SHA384_CTX_STRUC *)context, message, length, HAL_SHA_INTERNAL_SHA384);
            break;
        }
    }

    hal_crypt_lock_give();
    return HAL_SHA_STATUS_OK;
}

hal_sha_status_t hal_sha384_end(hal_sha384_context_t *context, uint8_t digest_message[HAL_SHA384_DIGEST_SIZE])
{
    hal_sha_status_t status;

    if (hal_crypt_lock_take() == -100)
        return HAL_SHA_STATUS_BUSY;
    status = crypt_sha_end((SHA384_CTX_STRUC *)context, digest_message, HAL_SHA_INTERNAL_SHA384);
    hal_crypt_lock_give();
    
    return status;
}

hal_sha_status_t hal_sha512_init(hal_sha512_context_t *context)
{
    hal_sha_status_t status;

    if (hal_crypt_lock_take() == -100)
        return HAL_SHA_STATUS_BUSY;
    status = crypt_sha_init((SHA_CTX_STRUC *)context, HAL_SHA_INTERNAL_SHA512);
    hal_crypt_lock_give();

    return status;
}

hal_sha_status_t hal_sha512_append(hal_sha512_context_t *context, uint8_t *message, uint32_t length)
{

    if (NULL == context) {
        log_hal_error("context is NULL.");
        return HAL_SHA_STATUS_ERROR;
    }
    if (NULL == message) {
        log_hal_error("message is NULL.");
        return HAL_SHA_STATUS_ERROR;
    }

    if (hal_crypt_lock_take() == -100)
        return HAL_SHA_STATUS_BUSY;

    while (1) {
        if (length >= MAX_APPEND_LENGTH) {
            crypt_sha_append((SHA512_CTX_STRUC *)context, message, MAX_APPEND_LENGTH, HAL_SHA_INTERNAL_SHA512);
            message += MAX_APPEND_LENGTH;
            length -= MAX_APPEND_LENGTH;
        } else {
            crypt_sha_append((SHA512_CTX_STRUC *)context, message, length, HAL_SHA_INTERNAL_SHA512);
            break;
        }
    }

    hal_crypt_lock_give();
    return HAL_SHA_STATUS_OK;
}

hal_sha_status_t hal_sha512_end(hal_sha512_context_t *context, uint8_t digest_message[HAL_SHA512_DIGEST_SIZE])
{
    hal_sha_status_t status;

    if (hal_crypt_lock_take() == -100)
        return HAL_SHA_STATUS_BUSY;
    status = crypt_sha_end((SHA512_CTX_STRUC *)context, digest_message, HAL_SHA_INTERNAL_SHA512);
    hal_crypt_lock_give();

    return status;
}

#endif /* HAL_SHA_MODULE_ENABLED */

