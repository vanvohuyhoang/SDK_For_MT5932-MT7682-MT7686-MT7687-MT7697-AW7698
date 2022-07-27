/* Copyright Statement:
 *
 * (C) 2017  Airoha Technology Corp. All rights reserved.
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
 
#include "fota_util.h"
#include "fota_multi_info.h"
#ifdef FOTA_EXTERNAL_FLASH_SUPPORT
#include "bsp_flash.h"
#include <assert.h>
#endif
#include "fota_flash.h"
#include "fota_multi_info_util.h"
#include "fota_platform.h"


fota_flash_partition_info *g_flash_partition_info = NULL;
FotaStorageType g_flash_storage_type = InternalFlash;


void fota_flash_set_storage_type(FotaStorageType storage_type)
{
    if (InternalFlash == storage_type ||
        ExternalFlash == storage_type)
    {
        g_flash_storage_type = storage_type;
    }
}


FotaStorageType fota_flash_get_storage_type(void)
{
    return g_flash_storage_type;
}


void fota_flash_bootup(void)
{
    /* Init internal Flash */
    fota_flash_init(TRUE);
    /* Init external Flash */
    fota_flash_init(FALSE);
    
    /* Set flash partition information. */
    fota_flash_config_init(&g_flash_partition_info);

#if FOTA_STORE_IN_EXTERNAL_FLASH
    fota_flash_set_storage_type(ExternalFlash);
#else
    fota_flash_set_storage_type(InternalFlash);
#endif

    fota_multi_info_sector_init();
}


#ifdef BL_FOTA_ENABLE
/* The address is from the mover info of the FOTA package header which contains SPI_SERIAL_FLASH_ADDRESS
  * if its the external flash address. */
bool fota_flash_is_addr_range_valid(uint32_t addr, uint32_t length)
{
    uint32_t i = 0;

#ifdef FOTA_EXTERNAL_FLASH_SUPPORT
    if (SPI_SERIAL_FLASH_ADDRESS & addr)
    {
        /* External flash address */
        /* Weak check. */
        if (length)
        {
            return TRUE;
        }

        return FALSE;
    }
#endif

    while (g_flash_partition_info && 
           FLASH_PARTITION_TYPE_MAX != g_flash_partition_info[i].partition_type)
    {
        if (addr >= g_flash_partition_info[i].LoadAddressLow && 
            (addr + length <= g_flash_partition_info[i].LoadAddressLow + g_flash_partition_info[i].BinaryLengthLow))
        {
            return TRUE;
        }
        i++;
    }

    FOTA_LOG_MSGID_W("Invalid address range. addr:%x len:%x",2, addr, length);
    return FALSE;
}
#endif


fota_flash_partition_info *fota_flash_get_partition_info(flash_partition_type_enum partition_type)
{
    uint32_t i = 0;

    while (g_flash_partition_info && 
           FLASH_PARTITION_TYPE_MAX != g_flash_partition_info[i].partition_type &&
           partition_type != g_flash_partition_info[i].partition_type)
    {
        i++;
    }

    if (g_flash_partition_info && partition_type == g_flash_partition_info[i].partition_type)
    {
        FOTA_LOG_MSGID_I("Found, partition_type:%d",1, partition_type);
        return g_flash_partition_info + i;
    }

    FOTA_LOG_MSGID_W("Not Found, partition_type:%d",1, partition_type);

    return NULL;
}


FOTA_ERRCODE fota_flash_get_fota_partition_info(FotaStorageType * storage_type, uint32_t * fota_address, uint32_t * length)
{
    fota_flash_partition_info *partition_info = NULL;
    flash_partition_type_enum partition_type = FLASH_PARTITION_TYPE_MAX;

    if (!storage_type || !fota_address || !length)
    {
        return FOTA_ERRCODE_INVALID_PARAMETER;
    }

    *storage_type = fota_flash_get_storage_type();

    if (InternalFlash == *storage_type)
    {
        partition_type = FLASH_PARTITION_TYPE_FOTA;
    }
#ifdef FOTA_EXTERNAL_FLASH_SUPPORT
    else if (ExternalFlash == *storage_type)
        {
        partition_type = FLASH_PARTITION_TYPE_FOTA_EXT;
        }
#endif
    else
    {
        return FOTA_ERRCODE_UNSUPPORTED;
    }
    
    partition_info = fota_flash_get_partition_info(partition_type);
    if (!partition_info)
    {
        return FOTA_ERRCODE_FAIL;
    }

    *fota_address = partition_info->LoadAddressLow;
    *length = partition_info->BinaryLengthLow;

    return FOTA_ERRCODE_SUCCESS;
}


FOTA_ERRCODE fota_flash_init(bool is_int)
{
    FOTA_ERRCODE ret = FOTA_ERRCODE_FAIL;
    static bool is_hal_inited = FALSE;

    if (is_int)
    {
        if (!is_hal_inited)
        {
            ret = HAL_FLASH_STATUS_OK == hal_flash_init() ? FOTA_ERRCODE_SUCCESS : FOTA_ERRCODE_FAIL;
            is_hal_inited = TRUE;
        }
        else
        {   
            ret = FOTA_ERRCODE_SUCCESS;
        }
    }
    else
    {
#ifdef FOTA_EXTERNAL_FLASH_SUPPORT
        /* Do not care about the return value for other task may have initialzed the SPI external Flash and SPI external Flash does
                * not support call init API more than one time without calling enough number of deinit API in the midway.
                * Only support 'init-deinit-init' flow. BSP_FLASH_STATUS_ERROR will be returned if init without deinit.
                */
        ret = bsp_flash_init();
        FOTA_LOG_MSGID_I("bsp_flash_init() ret:%d",1, ret);
        ret = FOTA_ERRCODE_SUCCESS;
#else
        ret = FOTA_ERRCODE_UNSUPPORTED;
#endif
    }

    return ret;
}


#ifdef FOTA_EXTERNAL_FLASH_SUPPORT
static bsp_block_size_type_t fota_flash_block_type_cnvt(hal_flash_block_t block_type)
{
    switch (block_type)
    {
        case HAL_FLASH_BLOCK_4K:
            return BSP_FLASH_BLOCK_4K;

        case HAL_FLASH_BLOCK_32K:
            return BSP_FLASH_BLOCK_32K;

        case HAL_FLASH_BLOCK_64K:
            return BSP_FLASH_BLOCK_64K;

        default:
            assert(0);
            break;
    }

    return BSP_FLASH_BLOCK_4K;
}
#endif


FOTA_ERRCODE fota_flash_erase(uint32_t start_address, hal_flash_block_t block_type, bool is_int)
{
    int32_t ret = FOTA_ERRCODE_FAIL;
    
    if (is_int)
    {
        ret = hal_flash_erase(start_address, block_type);
        FOTA_LOG_MSGID_I("hal_flash_erase ret:%d",1, ret);
        return HAL_FLASH_STATUS_OK == ret ? FOTA_ERRCODE_SUCCESS : FOTA_ERRCODE_FAIL;
    }
    else
    {
#ifdef FOTA_EXTERNAL_FLASH_SUPPORT
        /* Support the address with and without SPI_SERIAL_FLASH_ADDRESS. */
        start_address |= SPI_SERIAL_FLASH_ADDRESS;
        ret = bsp_flash_erase(start_address,
                                       fota_flash_block_type_cnvt(block_type));
        FOTA_LOG_MSGID_I("bsp_flash_erase ret:%d start_address:%x",2, ret, start_address);
        if (BSP_FLASH_STATUS_NOT_INIT == ret)
        {
            /* Other task may deinit external Flash. */
            bsp_flash_init();
            ret = bsp_flash_erase(start_address,
                                           fota_flash_block_type_cnvt(block_type));
        }
        
        FOTA_LOG_MSGID_I("bsp_flash_erase ret:%d",1, ret);
        return BSP_FLASH_STATUS_OK == ret ? FOTA_ERRCODE_SUCCESS : FOTA_ERRCODE_FAIL;
#else
        return FOTA_ERRCODE_UNSUPPORTED;
#endif
    }
}


// TODO: fota_port_isr_disable() and fota_set_scramble_flag()
FOTA_ERRCODE fota_flash_write(uint32_t start_address, const uint8_t *data, uint32_t length, bool is_int)
{
    int32_t ret = FOTA_ERRCODE_FAIL;

    if (is_int)
    {
        ret = hal_flash_write(start_address, data, length);
        FOTA_LOG_MSGID_I("hal_flash_write ret:%d",1, ret);
        return HAL_FLASH_STATUS_OK == ret ? FOTA_ERRCODE_SUCCESS : FOTA_ERRCODE_FAIL;
    }
    else
    {
#ifdef FOTA_EXTERNAL_FLASH_SUPPORT
        /* Support the address with and without SPI_SERIAL_FLASH_ADDRESS. */
        start_address |= SPI_SERIAL_FLASH_ADDRESS;
        ret = bsp_flash_write(start_address, data, length);
        FOTA_LOG_MSGID_I("bsp_flash_write ret:%d start_address:%x length:%d",3, ret, start_address, length);
        if (BSP_FLASH_STATUS_NOT_INIT == ret)
        {
            /* Other task may deinit external Flash. */
            bsp_flash_init();
            ret = bsp_flash_write(start_address, data, length);
        }
        FOTA_LOG_MSGID_I("bsp_flash_write ret:%d",1, ret);
        return BSP_FLASH_STATUS_OK == ret ? FOTA_ERRCODE_SUCCESS : FOTA_ERRCODE_FAIL;
#else
        return FOTA_ERRCODE_UNSUPPORTED;
#endif
    }
}


FOTA_ERRCODE fota_flash_read(uint32_t start_address, uint8_t *buffer, uint32_t length, bool is_int)
{
    int32_t ret = FOTA_ERRCODE_FAIL;
    
    //FOTA_LOG_I("start_address:%x length:%d is_int:%d", start_address, length, is_int);
    
    if (is_int)
    {
        ret = hal_flash_read(start_address, buffer, length);
        //FOTA_LOG_I("hal_flash_read ret:%d", ret);
        return HAL_FLASH_STATUS_OK == ret ? FOTA_ERRCODE_SUCCESS : FOTA_ERRCODE_FAIL;
    }
    else
    {
#ifdef FOTA_EXTERNAL_FLASH_SUPPORT
        /* Support the address with and without SPI_SERIAL_FLASH_ADDRESS. */
        start_address |= SPI_SERIAL_FLASH_ADDRESS;
        ret = bsp_flash_read(start_address, buffer, length);
        //FOTA_LOG_I("bsp_flash_read ret:%d start_address:%x length:%d", ret, start_address, length);
        if (BSP_FLASH_STATUS_NOT_INIT == ret)
        {
            /* Other task may deinit external Flash. */
            bsp_flash_init();
            ret = bsp_flash_read(start_address, buffer, length);
        }
        //FOTA_LOG_I("bsp_flash_read ret:%d", ret);
        return BSP_FLASH_STATUS_OK == ret ? FOTA_ERRCODE_SUCCESS : FOTA_ERRCODE_FAIL;
#else
        return FOTA_ERRCODE_UNSUPPORTED;
#endif
    }
}


#ifdef BL_FOTA_ENABLE
/* Currently, this API only can be used by bootloader. If it need be used in main project,
 * buffer need be allocated dynamically or reduce FOTA_FLASH_4K_READ_EACH_LEN.
 */
static FOTA_ERRCODE fota_flash_is_4K_block_erased(bool *is_erased, uint32_t start_address, bool is_int)
{
#define FOTA_FLASH_4K_READ_EACH_LEN    (1024)

    uint8_t buffer[FOTA_FLASH_4K_READ_EACH_LEN] = {0};
    FOTA_ERRCODE ret = FOTA_ERRCODE_FAIL;
    uint32_t count = 0, i = 0, j = 0;

    if (start_address & 0xFFF || !is_erased)
    {
        return FOTA_ERRCODE_INVALID_PARAMETER;
    }

    *is_erased = FALSE;

    count = 4096 / FOTA_FLASH_4K_READ_EACH_LEN;
    for (i = 0; i < count; i++)
    {
        ret = fota_flash_read(start_address, buffer, FOTA_FLASH_4K_READ_EACH_LEN, is_int);
        if (FOTA_ERRCODE_SUCCESS != ret)
        {
            return ret;
        }

        for (j = 0; j < FOTA_FLASH_4K_READ_EACH_LEN; j++)
        {
            if (0xFF != buffer[j])
            {
                return FOTA_ERRCODE_SUCCESS;
            }
        }
    }

    *is_erased = TRUE;
    return FOTA_ERRCODE_SUCCESS;
}
#endif


/* This API only works when it's being called by the bootloader. External flash's read speed
  * is very low. For example, in 2811A the speed is 49K/s when reading 512bytes each time
  * while the speed is 114K/s when reading 1K each time. If it's used in the main project, it
  * would block the task calling it when the length is large enough.
  * If erase_status is NULL and erase_status_size is not, return the size only. 
  * If erase_status is not NULL, originally erase_status_size is the size of erase_status buffer.
  * And it will be updated to be the actual size needed.
  */
FOTA_ERRCODE fota_flash_get_partition_erase_status(uint8_t *erase_status,
                                                           uint16_t *erase_status_size,
                                                           uint32_t start_address,
                                                           uint32_t length,
                                                           bool is_int)
{
#ifdef BL_FOTA_ENABLE
    uint32_t bit_size = 0, curr_bit_pos = 0;
    bool is_erased = FALSE;
    FOTA_ERRCODE ret = FOTA_ERRCODE_FAIL;
    uint16_t erase_status_size_old = 0;

    FOTA_LOG_MSGID_I("erase_status:%x,erase_status_size:%x,start_address:%x,length:%x,is_int:%d",5,
               erase_status,
               erase_status_size,
               start_address,
               length,
               is_int);

    if (!erase_status_size ||
#ifndef FOTA_ERASE_VERIFY_LEN
        start_address & 0xFFF)
#else
        start_address & 0xFFF || length & 0xFFF)
#endif
    {
        return FOTA_ERRCODE_INVALID_PARAMETER;
    }

    erase_status_size_old = *erase_status_size;

#ifndef FOTA_ERASE_VERIFY_LEN
    bit_size = ((length >> 12) + ((length & 0xFFF) > 0 ? 1 : 0));
#else
    bit_size = length >> 12;
#endif
    *erase_status_size = ((bit_size >> 3) + (bit_size % 8 ? 1 : 0));

    FOTA_LOG_MSGID_I("bit_size:%d, *erase_status_size:%d",2, bit_size, *erase_status_size);
    if (!erase_status)
    {
        /* Return erase_status_size only. */
        return FOTA_ERRCODE_SUCCESS;
    }

    FOTA_LOG_MSGID_I("erase_status_size_old:%d, *erase_status_size:%d",2,
               erase_status_size_old,
               *erase_status_size);
    if (erase_status_size_old < *erase_status_size)
    {
        return FOTA_ERRCODE_FAIL;
    }

    memset(erase_status, 0, erase_status_size_old);

    curr_bit_pos = 0;

    while (curr_bit_pos < bit_size)
    {
        ret = fota_flash_is_4K_block_erased(&is_erased, start_address, is_int);
        if (FOTA_ERRCODE_SUCCESS != ret)
        {
            return ret;
        }

        if (is_erased)
        {
            erase_status[curr_bit_pos / 8] |= (0x80 >> (curr_bit_pos % 8));
        }

        curr_bit_pos++;
        start_address += 0x1000;
    }

    for (int k = 0; k < *erase_status_size; k++)
    {
        FOTA_LOG_MSGID_I("erase_status[%d]:%x",2, k, erase_status[k]);
    }

    return FOTA_ERRCODE_SUCCESS;
#else
    return FOTA_ERRCODE_UNSUPPORTED;
#endif
}

