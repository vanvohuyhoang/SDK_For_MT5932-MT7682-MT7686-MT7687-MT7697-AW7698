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

#include "fota_flash.h"
#include "fota_multi_info.h"
#include "fota_multi_info_util.h"
#include "fota_platform.h"


/* Last 4K is used to store multiple information. */
#define FOTA_MULTI_INFO_SECTOR_ADDR_RETRO_OFFSET  0x1000
/* Last 4K layout discribed by the offset of the beginning of the Lask 4K */
/* State records: 0th-255th bytes. */
#define FOTA_MULTI_INFO_STATE_ADDR_OFFSET  0
/* Upgrade flag: 256th-383th bytes. */
#define FOTA_MULTI_INFO_UPGRADE_FLAG_ADDR_OFFSET  0x100
/* Integrity check result: 384th-511th bytes. */
#define FOTA_MULTI_INFO_DL_INTEGRITY_RES_ADDR_OFFSET  0x180
/* FOTA package release version: 512th- bytes*/
#define FOTA_MULTI_INFO_VERSION_ADDR_OFFSET  0x200

/* Add new info type offset before this line and update the offset below. */
#define FOTA_MULTI_INFO_NULL_ADDR_OFFSET  0x280

typedef struct
{
    uint32_t last_record_addr;
    uint16_t history_record_count;
    FotaStorageType storage_type;
    bool is_init;
}fota_multi_info_info_struct;


typedef struct
{
    fota_multi_info_info_struct state;
    fota_multi_info_info_struct upgrade_flag;
    fota_multi_info_info_struct dl_integrity_res;
    fota_multi_info_info_struct version;
} fota_multi_info_context_struct;


fota_multi_info_context_struct g_fota_multi_info_ctx;
fota_multi_info_context_struct *g_fota_multi_info_ctx_ptr = NULL;


void fota_multi_info_sector_init(void)
{
    if (!g_fota_multi_info_ctx_ptr)
    {
        g_fota_multi_info_ctx_ptr = &g_fota_multi_info_ctx;
    }
}


void fota_multi_info_sector_deinit(void)
{
    if (g_fota_multi_info_ctx_ptr)
    {
        memset(g_fota_multi_info_ctx_ptr, 0, sizeof(fota_multi_info_context_struct));
    }
}



/* offset is FOTA_MULTI_INFO_X such as FOTA_MULTI_INFO_STATE_ADDR_OFFSET. */
FOTA_ERRCODE fota_multi_info_sector_start_addr_get(uint32_t *addr,
                                                   FotaStorageType *storage_type)
{
    FOTA_ERRCODE ret = FOTA_ERRCODE_FAIL;
    uint32_t fota_address, length;

    if (!addr || !storage_type)
    {
        return FOTA_ERRCODE_FAIL;
    }

    ret = fota_flash_get_fota_partition_info(storage_type, &fota_address, &length);
    if (FOTA_ERRCODE_SUCCESS != ret)
    {
        return ret;
    }

    *addr = fota_address + length - FOTA_MULTI_INFO_SECTOR_ADDR_RETRO_OFFSET;

    return FOTA_ERRCODE_SUCCESS;
}


static FOTA_ERRCODE fota_multi_info_sector_erase(void)
{
    FOTA_ERRCODE ret = FOTA_ERRCODE_FAIL;
    uint32_t start_addr = 0;
    FotaStorageType storage_type = 0;

    ret = fota_multi_info_sector_start_addr_get(&start_addr, &storage_type);
    if (FOTA_ERRCODE_SUCCESS != ret)
    {
        return ret;
    }

    /* Be aware that erase size shuld be changed, if the size of multi info sector changes. */
    FOTA_LOG_MSGID_I("Before erase start_addr:%x",1, start_addr);
    return fota_flash_erase(start_addr,
                           HAL_FLASH_BLOCK_4K,
                           InternalFlash == storage_type);
    FOTA_LOG_MSGID_I("End erase start_addr:%x",1, start_addr);
}


/* Erase last 4k and write back the last valid records for the info types required. 
  * Even if the return value is not SUCCESS, erase_done is valid.
  */
FOTA_ERRCODE fota_multi_info_sector_clean(uint16_t not_wb_info_types, bool *erase_done)
{
    FOTA_ERRCODE ret = FOTA_ERRCODE_SUCCESS, ret_val = FOTA_ERRCODE_SUCCESS;
    uint16_t state = 0;
    bool upgrade_flag_set = FALSE;
#ifndef BL_FOTA_ENABLE
    uint8_t dl_integrity_res = 0;
    bool write_back_dl_integrity_res = FALSE;
#endif
    uint8_t version[FOTA_VERSION_MAX_SIZE] = {0};
    bool write_back_state = FALSE, write_back_upgrade_flag = FALSE, write_back_version = FALSE;

    if (erase_done)
    {
        *erase_done = FALSE;
    }

    /* FOTA state */
    if (!(FOTA_MULTI_INFO_TYPE_STATE & not_wb_info_types))
    {
        ret = fota_state_read(&state);
        if (FOTA_ERRCODE_SUCCESS == ret)
        {
            if (0xFFFF != state)
            {
                write_back_state = TRUE;
            }
        }
        else
        {
            ret_val = ret;
        }
    }

    /* upgrade_flag */
    if (!(FOTA_MULTI_INFO_TYPE_UPGRADE_FLAG & not_wb_info_types))
    {
        upgrade_flag_set = fota_upgrade_flag_is_set();
        write_back_upgrade_flag = TRUE;
    }

#ifndef BL_FOTA_ENABLE
    /* dl_integrity_res */
    if (!(FOTA_MULTI_INFO_TYPE_DL_INTEGRITY_RES & not_wb_info_types))
    {
        ret = fota_dl_integrity_res_read(&dl_integrity_res);
        if (FOTA_ERRCODE_SUCCESS == ret)
        {
            if (FOTA_DL_INTEGRITY_RES_VAL_EMPTY != dl_integrity_res)
            {
                write_back_dl_integrity_res = TRUE;
            }
        }
        else
        {
            ret_val = ret;
        }
    }
#endif

    /* version */
    if (!(FOTA_MULTI_INFO_TYPE_VERSION & not_wb_info_types))
    {
        ret = fota_version_read(version, FOTA_VERSION_MAX_SIZE);
        if (FOTA_ERRCODE_SUCCESS == ret)
        {
            write_back_version = TRUE;
        }
        /* ret_val does not update to ret here because even if missing writting back the version, reading
                 * the version may still work(NVDM/Default). */
    }

    fota_multi_info_sector_deinit();

    /* Erase last 4K */    
    ret = fota_multi_info_sector_erase();
    if (FOTA_ERRCODE_SUCCESS != ret)
    {
        FOTA_LOG_MSGID_E("Failed to erase FOTA last 4K. ret:%d",1, ret);
        return ret;
    }

    if (erase_done)
    {
        *erase_done = TRUE;
    }    
    
    /* Write back FOTA state */
    if (write_back_state)
    {
        ret = fota_state_write(state);
        if (FOTA_ERRCODE_SUCCESS != ret)
        {
            ret_val = ret;
        }
    }
    
    /* Write back upgrade_flag */
    if (write_back_upgrade_flag)
    {
        if (upgrade_flag_set)
        {
            ret = fota_upgrade_flag_set();
            if (FOTA_ERRCODE_SUCCESS != ret)
            {
                ret_val = ret;
            }
        }
    }

#ifndef BL_FOTA_ENABLE
    /* Write back dl_integrity_res */
    if (write_back_dl_integrity_res)
    {
        ret = fota_dl_integrity_res_write(dl_integrity_res);
        if (FOTA_ERRCODE_SUCCESS != ret)
        {
            ret_val = ret;
        }
    }
#endif

    /* Write back version */
    if (write_back_version)
    {
        ret = fota_version_write(version, (uint8_t)strlen((const char *)version));
        if (FOTA_ERRCODE_SUCCESS != ret)
        {
            ret_val = ret;
        }
    }

    if (FOTA_ERRCODE_SUCCESS != ret_val)
    {
        FOTA_LOG_MSGID_W("Erase is done. However, something wrong happened. ret_val:%d", ret_val);
    }

#ifndef BL_FOTA_ENABLE
    FOTA_LOG_MSGID_I("state:%d upgrade_flag:%d dl_integrity:%d version:%d ret_val:%d",5,
                     write_back_state,
                     write_back_upgrade_flag,
                     write_back_dl_integrity_res,
                     write_back_version,
                     ret_val);
#else
    FOTA_LOG_MSGID_I("state:%d upgrade_flag:%d version:%d ret_val:%d",4,
                     write_back_state,
                     write_back_upgrade_flag,
                     write_back_version,
                     ret_val);
#endif

    return FOTA_ERRCODE_SUCCESS;
}


FOTA_ERRCODE fota_multi_info_sector_reset(void)
{
    bool erase_done = FALSE;
    FOTA_ERRCODE ret = FOTA_ERRCODE_SUCCESS;

    ret = fota_multi_info_sector_clean(~FOTA_MULTI_INFO_TYPE_VERSION, &erase_done);
    if (FOTA_ERRCODE_SUCCESS != ret && erase_done)
    {
        FOTA_LOG_MSGID_W("Something wrong happened ret:%d. But continue for erasing is done.",1, ret);
        return FOTA_ERRCODE_SUCCESS;
    }

    return ret;
}


fota_multi_info_info_struct *fota_multi_info_get_info(fota_multi_info_type_enum info_type)
{
    if (!g_fota_multi_info_ctx_ptr)
    {
        return NULL;
    }

    switch (info_type)
    {
        case FOTA_MULTI_INFO_TYPE_STATE:
        {
            return &(g_fota_multi_info_ctx_ptr->state);
        }

        case FOTA_MULTI_INFO_TYPE_UPGRADE_FLAG:
        {
            return &(g_fota_multi_info_ctx_ptr->upgrade_flag);
        }

        case FOTA_MULTI_INFO_TYPE_DL_INTEGRITY_RES:
        {
            return &(g_fota_multi_info_ctx_ptr->dl_integrity_res);
        }

        case FOTA_MULTI_INFO_TYPE_VERSION:
        {
            return &(g_fota_multi_info_ctx_ptr->version);
        }

        default:
            break;
    }

    return NULL;
}


FOTA_ERRCODE fota_multi_info_get_addr_offset(fota_multi_info_type_enum info_type,
                                                    uint16_t *offset)
{
    if (!offset)
    {
        return FOTA_ERRCODE_INVALID_PARAMETER;
    }

    switch (info_type)
    {        
        case FOTA_MULTI_INFO_TYPE_STATE:
        {
            *offset = FOTA_MULTI_INFO_STATE_ADDR_OFFSET;
            break;
        }

        case FOTA_MULTI_INFO_TYPE_UPGRADE_FLAG:
        {
            *offset = FOTA_MULTI_INFO_UPGRADE_FLAG_ADDR_OFFSET;
            break;
        }

        case FOTA_MULTI_INFO_TYPE_DL_INTEGRITY_RES:
        {
            *offset = FOTA_MULTI_INFO_DL_INTEGRITY_RES_ADDR_OFFSET;
            break;
        }

        case FOTA_MULTI_INFO_TYPE_VERSION:
        {
            *offset = FOTA_MULTI_INFO_VERSION_ADDR_OFFSET;
            break;
        }

        /* Do not allow to access FOTA_MULTI_INFO_TYPE_NULL for it's invalid. */
        default:
            return FOTA_ERRCODE_INVALID_PARAMETER;
    }

    return FOTA_ERRCODE_SUCCESS;
}


/* offset is FOTA_MULTI_INFO_X such as FOTA_MULTI_INFO_STATE_ADDR_OFFSET. */
FOTA_ERRCODE fota_multi_info_start_addr_get(uint32_t *addr,
                                                   FotaStorageType *storage_type,
                                                   fota_multi_info_type_enum info_type)
{
    FOTA_ERRCODE ret = FOTA_ERRCODE_FAIL;
    uint16_t offset;

    if (!addr || !storage_type)
    {
        return FOTA_ERRCODE_FAIL;
    }
    
    ret = fota_multi_info_get_addr_offset(info_type, &offset);
    if (FOTA_ERRCODE_SUCCESS != ret)
    {
        return ret;
    }

    ret = fota_multi_info_sector_start_addr_get(addr, storage_type);
    if (FOTA_ERRCODE_SUCCESS != ret)
    {
        return ret;
    }

    *addr += offset;

    FOTA_LOG_MSGID_I("fota_multi_info_start_addr_get() info_type:%d addr:%x storage_type:%d",3, info_type, *addr, *storage_type);
    return FOTA_ERRCODE_SUCCESS;
}


fota_multi_info_type_enum fota_multi_info_next_info_type_get(fota_multi_info_type_enum info_type)
{
    switch (info_type)
    {
        case FOTA_MULTI_INFO_TYPE_STATE:
        {
            return FOTA_MULTI_INFO_TYPE_UPGRADE_FLAG;
        }

        case FOTA_MULTI_INFO_TYPE_UPGRADE_FLAG:
        {
            return FOTA_MULTI_INFO_TYPE_DL_INTEGRITY_RES;
        }

        case FOTA_MULTI_INFO_TYPE_DL_INTEGRITY_RES:
        {
            return FOTA_MULTI_INFO_TYPE_VERSION;
        }

        case FOTA_MULTI_INFO_TYPE_VERSION:
        {
            return FOTA_MULTI_INFO_TYPE_NULL;
        }

        default:
            break;        
    }

    return FOTA_MULTI_INFO_TYPE_NONE;
}


/* offset is FOTA_MULTI_INFO_X such as FOTA_MULTI_INFO_STATE_ADDR_OFFSET. */
FOTA_ERRCODE fota_multi_info_end_addr_get(uint32_t *addr,
                                                 FotaStorageType *storage_type,
                                                 fota_multi_info_type_enum info_type)
{
    FOTA_ERRCODE ret = FOTA_ERRCODE_FAIL;
    uint32_t multi_info_start_addr;
    fota_multi_info_type_enum next_info_type = FOTA_MULTI_INFO_TYPE_NONE;

    if (!addr || !storage_type)
    {
        return FOTA_ERRCODE_FAIL;
    }

    next_info_type = fota_multi_info_next_info_type_get(info_type);
    if (FOTA_MULTI_INFO_TYPE_NONE == next_info_type ||
        FOTA_MULTI_INFO_TYPE_ALL == next_info_type)
    {
        return FOTA_ERRCODE_FAIL;
    }

    if (FOTA_MULTI_INFO_TYPE_NULL == next_info_type)
    {
        /* If it's the valid last type of info, allow it to reach the end of the FOTA secotr. */
        ret = fota_multi_info_sector_start_addr_get(&multi_info_start_addr,
                                                    storage_type);
        *addr = multi_info_start_addr + FOTA_MULTI_INFO_SECTOR_ADDR_RETRO_OFFSET;
    }
    else
    {
        /* Get next info's start address */
        ret = fota_multi_info_start_addr_get(addr,
                                             storage_type,
                                             next_info_type);
        if (FOTA_ERRCODE_SUCCESS != ret)
        {
            return ret;
        }
    }

    *addr = *addr - 1;

    return FOTA_ERRCODE_SUCCESS;
}


FOTA_ERRCODE fota_multi_info_get_record_len(fota_multi_info_type_enum info_type,
                                                    uint8_t *record_len)
{
    if (!record_len)
    {
        return FOTA_ERRCODE_INVALID_PARAMETER;
    }

    switch (info_type)
    {        
        case FOTA_MULTI_INFO_TYPE_STATE:
        {
            *record_len = FOTA_MULTI_INFO_STATE_RECORD_LEN;
            break;
        }

        case FOTA_MULTI_INFO_TYPE_UPGRADE_FLAG:
        {
            *record_len = FOTA_MULTI_INFO_UPGRADE_FLAG_RECORD_LEN;
            break;
        }

        case FOTA_MULTI_INFO_TYPE_DL_INTEGRITY_RES:
        {
            *record_len = FOTA_MULTI_INFO_DL_INTEGRITY_RES_RECORD_LEN;
            break;
        }

        case FOTA_MULTI_INFO_TYPE_VERSION:
        {
            *record_len = FOTA_MULTI_INFO_VERSION_RECORD_LEN;
            break;
        }

        default:
            return FOTA_ERRCODE_INVALID_PARAMETER;
    }

    return FOTA_ERRCODE_SUCCESS;
}


FOTA_ERRCODE fota_multi_info_init(fota_multi_info_type_enum info_type)
{
    uint32_t start_addr, end_addr, read_addr;
    FotaStorageType storage_type;
    FOTA_ERRCODE ret = FOTA_ERRCODE_FAIL;
    uint8_t buffer[FOTA_MULTI_INFO_RECORD_LEN_LCM] = {0}, record_len = 0;
    int16_t unread_len = 0, valid_data_len = FOTA_MULTI_INFO_RECORD_LEN_LCM, data_num = 0;
    uint16_t i = 0, j = 0;
    fota_multi_info_info_struct *info = NULL;

    info = fota_multi_info_get_info(info_type);

    if (!info)
    {
        return FOTA_ERRCODE_FOTA_RESULT_NOT_FOUND;
    }
    else if (info->is_init)
    {
        return FOTA_ERRCODE_SUCCESS;
    }

    FOTA_LOG_MSGID_I("info_type:%x, info->is_init:%d",2,
               info_type,
               info->is_init);

    ret = fota_multi_info_start_addr_get(&start_addr, &storage_type, info_type);
    if (FOTA_ERRCODE_SUCCESS != ret)
    {
        return ret;
    }

    ret = fota_multi_info_end_addr_get(&end_addr, &storage_type, info_type);
    if (FOTA_ERRCODE_SUCCESS != ret)
    {
        return ret;
    }

    ret = fota_multi_info_get_record_len(info_type, &record_len);
    if (FOTA_ERRCODE_SUCCESS != ret || FOTA_MULTI_INFO_RECORD_LEN_LCM % record_len)
    {
        return ret;
    }

    unread_len = end_addr - start_addr + 1;

    if (unread_len <= 0)
    {
        return FOTA_ERRCODE_FAIL;
    }

    /* Read the data from the start to the end. Do not read from the end to the start because
         * the number of the records is usually small. If read from the end, it would read lots of data
         * before find the last record. And if it's external flash, it would take a very long time during bootup.
         */
    read_addr = start_addr;
    while (unread_len)
    {
        memset(buffer, 0, FOTA_MULTI_INFO_RECORD_LEN_LCM);
        ret = fota_flash_read(read_addr, buffer, FOTA_MULTI_INFO_RECORD_LEN_LCM, storage_type == InternalFlash);
        if (FOTA_ERRCODE_SUCCESS != ret)
        {
            return ret;
        }
        read_addr += FOTA_MULTI_INFO_RECORD_LEN_LCM;
        
        if (unread_len < FOTA_MULTI_INFO_RECORD_LEN_LCM)
        {
            valid_data_len = unread_len;
        }
        unread_len -= valid_data_len;

        /* The last part which is not enough for one record is ignored. */
        for (i = 0; i < valid_data_len / record_len; i++)
        {
            for (j = 0; j < record_len; j++)
            {
                if (0xFF != buffer[i * record_len + j])
                {
                    break;
                }
            }

            if (j >= record_len)
            {
                /* A block of 0xFF with record_len long is found. */
                break;
            }
            
            /* Record found */
            data_num += record_len;
        }

        if (i < valid_data_len / record_len)
        {
            /* A block of 0xFF with record_len long is found. */
            break;
        }
    }

    /* This check can also make sure info->last_record_addr calculation below be valid. */
    if (data_num % record_len)
    {
        return FOTA_ERRCODE_FAIL;
    }

    if (0 == data_num)
    {
        info->last_record_addr = start_addr;
    }
    else 
    {
        info->last_record_addr = start_addr + data_num - record_len;
    }

    info->history_record_count = data_num / record_len;
    info->storage_type = storage_type;
    info->is_init = TRUE;

    FOTA_LOG_MSGID_I("info_type:%x start_addr:%x last_record_addr:%x data_num:%d history_record_count:%d",5,
               info_type,
               start_addr,
               info->last_record_addr,                   
               data_num,
               info->history_record_count);
    return FOTA_ERRCODE_SUCCESS;
}


FOTA_ERRCODE fota_multi_info_is_addr_valid(fota_multi_info_type_enum info_type,
                                                 uint32_t addr,
                                                 uint8_t len)
{
    uint32_t info_start_addr, info_end_addr;
    FotaStorageType storage_type;
    FOTA_ERRCODE ret = FOTA_ERRCODE_FAIL;

    ret = fota_multi_info_start_addr_get(&info_start_addr,
                                         &storage_type,
                                         info_type);
    if (FOTA_ERRCODE_SUCCESS != ret)
    {
        return ret;
    }

    ret = fota_multi_info_end_addr_get(&info_end_addr,
                                       &storage_type,
                                       info_type);
    if (FOTA_ERRCODE_SUCCESS != ret)
    {
        return ret;
    }

    if (addr >= info_start_addr && ((addr + len - 1) <= info_end_addr))
    {
        return FOTA_ERRCODE_SUCCESS;
    }
    else if (addr >= info_start_addr && ((addr + len - 1) > info_end_addr))
    {
        return FOTA_ERRCODE_OUT_OF_RANGE;
    }

    return FOTA_ERRCODE_FAIL;
}


FOTA_ERRCODE fota_multi_info_read(fota_multi_info_type_enum info_type,
                                      uint8_t *record_data,
                                      uint8_t record_len)
{
    FOTA_ERRCODE ret = FOTA_ERRCODE_FAIL;
    fota_multi_info_info_struct *info = NULL;

    FOTA_LOG_MSGID_I("info_type:%x, record_data:%x, record_len:%d",3,
               info_type, record_data, record_len);

    if (!record_data || !record_len)
    {
        return FOTA_ERRCODE_INVALID_PARAMETER;
    }

    ret = fota_multi_info_init(info_type);
    if (FOTA_ERRCODE_SUCCESS != ret)
    {
        return ret;
    }

    info = fota_multi_info_get_info(info_type);
    if (!info)
    {
        return FOTA_ERRCODE_FOTA_RESULT_NOT_FOUND;
    }

    ret = fota_multi_info_is_addr_valid(info_type, info->last_record_addr, record_len);
    if (FOTA_ERRCODE_SUCCESS != ret)
    {
        return ret;
    }

    ret = fota_flash_read(info->last_record_addr,
                          record_data,
                          record_len,
                          InternalFlash == info->storage_type);

    FOTA_LOG_MSGID_I("ret:%d, record_data:%x %x",3,
               ret,
               record_data[0],
               record_data[1]);
    return ret;
}


FOTA_ERRCODE fota_multi_info_write(fota_multi_info_type_enum info_type,
                                        uint8_t *record_data,
                                        uint8_t record_len)
{
    FOTA_ERRCODE ret = FOTA_ERRCODE_FAIL, ret_val = FOTA_ERRCODE_FAIL;
    fota_multi_info_info_struct *info = NULL;
    uint8_t buffer[FOTA_MULTI_INFO_RECORD_MAX_LEN] = {0};
    uint32_t address = 0;

    FOTA_LOG_MSGID_I("info_type:%x, record_data:%x, record_len:%d",3,
               info_type, record_data, record_len);

    if (!record_data || !record_len || FOTA_MULTI_INFO_RECORD_MAX_LEN < record_len)
    {
        return FOTA_ERRCODE_INVALID_PARAMETER;
    }

    ret = fota_multi_info_read(info_type, buffer, record_len);
    if (FOTA_ERRCODE_SUCCESS != ret)
    {
        return ret;
    }

    /* Do not write, if the last record equals to the record_data. */
    FOTA_LOG_MSGID_I("buffer:%x, record_data:%x, record_len:%d",3,
               buffer, record_data, record_len);
    if (0 == memcmp((const void *)buffer, (const void *)record_data, (unsigned int)record_len))
    {
        return FOTA_ERRCODE_SUCCESS;
    }

    info = fota_multi_info_get_info(info_type);
    if (!info)
    {
        return FOTA_ERRCODE_FOTA_RESULT_NOT_FOUND;
    }

    if (0 == info->history_record_count)
    {
        address = info->last_record_addr;
    }
    else
    {
        address = info->last_record_addr + record_len;
    }

    ret_val = fota_multi_info_is_addr_valid(info_type, address, record_len);
    if (FOTA_ERRCODE_SUCCESS == ret_val)
    {
        ret = fota_flash_write(address,
                               record_data,
                               record_len,
                               InternalFlash == info->storage_type);
        /* Check if [address, address + record_len) is broken. */
        if (FOTA_ERRCODE_SUCCESS != ret)
        {
            uint8_t record_data_read[FOTA_MULTI_INFO_RECORD_MAX_LEN] = {0};
            int i = 0;

            ret = fota_flash_read(address,
                                  record_data_read,
                                  record_len,
                                  InternalFlash == info->storage_type);            
            for (i = 0; i < record_len; i++)
            {
                if (0xFF != record_data_read[i])
                {
                    break;
                }
            }

            if (i < record_len)
            {
                /* [address, address + record_len) is broken and update last_record_addr.
                                */
                info->last_record_addr = address;
                info->history_record_count++;
            }
            ret = FOTA_ERRCODE_FAIL;
        }
        else
        {
            /* update last_record_addr */
            info->last_record_addr = address;
            info->history_record_count++;
        }
    }

    return ret_val;
}

