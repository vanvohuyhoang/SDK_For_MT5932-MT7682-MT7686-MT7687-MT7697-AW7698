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


#include "crc32.h"
#ifdef MTK_NVDM_ENABLE
#include "nvdm.h"
#endif
#include "fota_flash.h"
#include "fota_multi_info_util.h"
#include "fota_multi_info.h"
#include "fota_platform.h"


#define FOTA_UPGRADE_INFO_FLAG_MARK        0x2F
#define FOTA_UPGRADE_INFO_FLAG_SET         0x01
#define FOTA_UPGRADE_INFO_FLAG_CLEAR       0x00
#define FOTA_DL_INTEGRITY_RES_MARK         0x4F


FOTA_ERRCODE fota_state_read(uint16_t *state)
{
    FOTA_ERRCODE ret = FOTA_ERRCODE_FAIL;

    if (!state)
    {
        return FOTA_ERRCODE_INVALID_PARAMETER;
    }

    ret = fota_multi_info_read(FOTA_MULTI_INFO_TYPE_STATE,
                               (uint8_t *)state,
                               FOTA_MULTI_INFO_STATE_RECORD_LEN);
    return ret;
}


FOTA_ERRCODE fota_state_write(uint16_t state)
{
    FOTA_ERRCODE ret = FOTA_ERRCODE_SUCCESS;

    ret = fota_multi_info_write(FOTA_MULTI_INFO_TYPE_STATE,
                                 (uint8_t *)&state,
                                 FOTA_MULTI_INFO_STATE_RECORD_LEN);

    if (FOTA_ERRCODE_OUT_OF_RANGE == ret)
    {
        ret = fota_multi_info_sector_clean(FOTA_MULTI_INFO_TYPE_STATE, NULL);
        if (FOTA_ERRCODE_SUCCESS == ret)
        {
            ret = fota_multi_info_write(FOTA_MULTI_INFO_TYPE_STATE,
                                        (uint8_t *)&state,
                                        FOTA_MULTI_INFO_STATE_RECORD_LEN);
        }
    }

    return ret;
}


FOTA_ERRCODE fota_upgrade_flag_read(uint8_t *ugprade_flag)
{
    FOTA_ERRCODE ret = FOTA_ERRCODE_FAIL;
    uint8_t buffer[FOTA_MULTI_INFO_UPGRADE_FLAG_RECORD_LEN] = {0};

    if (!ugprade_flag)
    {
        return FOTA_ERRCODE_INVALID_PARAMETER;
    }

    *ugprade_flag = 0xFF;

    ret = fota_multi_info_read(FOTA_MULTI_INFO_TYPE_UPGRADE_FLAG,
                               buffer,
                               FOTA_MULTI_INFO_UPGRADE_FLAG_RECORD_LEN);
    if (FOTA_ERRCODE_SUCCESS != ret)
    {
        return ret;
    }

    if (FOTA_UPGRADE_INFO_FLAG_MARK != buffer[0] ||
        (FOTA_UPGRADE_INFO_FLAG_CLEAR != buffer[1] &&
         FOTA_UPGRADE_INFO_FLAG_SET != buffer[1]))
    {
        return FOTA_ERRCODE_FAIL;
    }

    *ugprade_flag = buffer[1];
    return FOTA_ERRCODE_SUCCESS;
}


bool fota_upgrade_flag_is_set(void)
{
    uint8_t ugprade_flag = 0;

#ifdef FOTA_UPGRADE_TEST
    return TRUE;
#endif

    if (FOTA_ERRCODE_SUCCESS == fota_upgrade_flag_read(&ugprade_flag))
    {
        if (FOTA_UPGRADE_INFO_FLAG_SET == ugprade_flag)
        {
            return TRUE;
        }
    }

    return FALSE;
}


FOTA_ERRCODE fota_upgrade_flag_write(uint8_t ugprade_flag)
{
    FOTA_ERRCODE ret = FOTA_ERRCODE_SUCCESS;
    uint8_t buffer[FOTA_MULTI_INFO_UPGRADE_FLAG_RECORD_LEN] = {FOTA_UPGRADE_INFO_FLAG_MARK, ugprade_flag};

    ret = fota_multi_info_write(FOTA_MULTI_INFO_TYPE_UPGRADE_FLAG,
                                 buffer,
                                 FOTA_MULTI_INFO_UPGRADE_FLAG_RECORD_LEN);
    if (FOTA_ERRCODE_OUT_OF_RANGE == ret)
    {
        ret = fota_multi_info_sector_clean(FOTA_MULTI_INFO_TYPE_UPGRADE_FLAG, NULL);
        if (FOTA_ERRCODE_SUCCESS == ret)
        {
            ret = fota_multi_info_write(FOTA_MULTI_INFO_TYPE_UPGRADE_FLAG,
                                        buffer,
                                        FOTA_MULTI_INFO_UPGRADE_FLAG_RECORD_LEN);
        }
    }

    return ret;
}


FOTA_ERRCODE fota_upgrade_flag_clear(void)
{
    FOTA_LOG_MSGID_I("fota_upgrade_flag_clear()", 0);
    return fota_upgrade_flag_write(FOTA_UPGRADE_INFO_FLAG_CLEAR);
}


FOTA_ERRCODE fota_upgrade_flag_set(void)
{
    FOTA_LOG_MSGID_I("fota_upgrade_flag_set()", 0);
    return fota_upgrade_flag_write(FOTA_UPGRADE_INFO_FLAG_SET);
}


#ifndef BL_FOTA_ENABLE
/* Integrity check result after FOTA package is downloaded. */
FOTA_ERRCODE fota_dl_integrity_res_read(uint8_t *integrity_res)
{
    FOTA_ERRCODE ret = FOTA_ERRCODE_FAIL;
    uint8_t buffer[FOTA_MULTI_INFO_DL_INTEGRITY_RES_RECORD_LEN] = {0};

    if (!integrity_res)
    {
        return FOTA_ERRCODE_INVALID_PARAMETER;
    }

    ret = fota_multi_info_read(FOTA_MULTI_INFO_TYPE_DL_INTEGRITY_RES,
                               buffer,
                               FOTA_MULTI_INFO_DL_INTEGRITY_RES_RECORD_LEN);
    if (FOTA_ERRCODE_SUCCESS != ret)
    {
        return ret;
    }

    if (buffer[0] == FOTA_DL_INTEGRITY_RES_MARK &&
        (FOTA_DL_INTEGRITY_RES_VAL_PASS == buffer[1] ||
         FOTA_DL_INTEGRITY_RES_VAL_FAIL == buffer[1]))
    {
        *integrity_res = buffer[1];
        return FOTA_ERRCODE_SUCCESS;
    }

    if (0xFF == buffer[0] && 0xFF == buffer[1])
    {
        *integrity_res = FOTA_DL_INTEGRITY_RES_VAL_EMPTY;
        return FOTA_ERRCODE_SUCCESS;
    }

    return FOTA_ERRCODE_FAIL;
}


FOTA_ERRCODE fota_dl_integrity_res_write(uint8_t result)
{
    FOTA_ERRCODE ret = FOTA_ERRCODE_SUCCESS;
    uint8_t buffer[FOTA_MULTI_INFO_DL_INTEGRITY_RES_RECORD_LEN] = {FOTA_DL_INTEGRITY_RES_MARK, result};

    ret = fota_multi_info_write(FOTA_MULTI_INFO_TYPE_DL_INTEGRITY_RES,
                                 buffer,
                                 FOTA_MULTI_INFO_DL_INTEGRITY_RES_RECORD_LEN);
    if (FOTA_ERRCODE_OUT_OF_RANGE == ret)
    {
        ret = fota_multi_info_sector_clean(FOTA_MULTI_INFO_TYPE_DL_INTEGRITY_RES, NULL);
        if (FOTA_ERRCODE_SUCCESS == ret)
        {
            ret = fota_multi_info_write(FOTA_MULTI_INFO_TYPE_DL_INTEGRITY_RES,
                                 buffer,
                                 FOTA_MULTI_INFO_DL_INTEGRITY_RES_RECORD_LEN);
        }
    }

    return ret;
}
#endif


static FOTA_ERRCODE fota_version_crc32_check(uint8_t *version_record, uint8_t version_len)
{
    uint32_t crc32_calc = 0xFFFFFF00, crc32_read = 0xFFFFFFFF;
    
    if (!version_record || !version_len)
    {
        return FOTA_ERRCODE_INVALID_PARAMETER;
    }

    /* Check crc32 only when version_len > 0 */
    crc32_calc = CRC32_Generate(version_record, version_len, crc32_calc);
    memcpy(&crc32_read, version_record + FOTA_VERSION_MAX_SIZE, FOTA_VERSION_INTEGRITY_CHECK_VALUE_SIZE);

    if (crc32_calc != crc32_read)
    {
        FOTA_LOG_MSGID_E("version checksum does not match.", 0);
        return FOTA_ERRCODE_CHECK_INTEGRITY_FAIL;
    }

    return FOTA_ERRCODE_SUCCESS;
}


static FOTA_ERRCODE fota_version_read_from_nvdm(uint8_t *buffer, uint8_t buf_size)
{
#ifdef MTK_NVDM_ENABLE
    int32_t ret = FOTA_ERRCODE_FAIL;
    uint8_t version_record[FOTA_MULTI_INFO_VERSION_RECORD_LEN] = {0};    
    uint32_t version_len = 0;

    if (!buffer || !buf_size)
    {
        return FOTA_ERRCODE_INVALID_PARAMETER;
    }

    ret = (int32_t)nvdm_read_data_item(FOTA_NVDM_GROUP_NAME,
                                       FOTA_NVDM_DATA_ITEM_VERSION,
                                       version_record,
                                       &version_len);
    if (NVDM_STATUS_OK != ret)
    {
        /* No NVDM initial value. If version has never been written into NVDM, NOT OK will be returned. */
        return FOTA_ERRCODE_UNINITIALIZED;
    }

    if (version_record[FOTA_VERSION_MAX_SIZE - 1])
    {
        return FOTA_ERRCODE_FAIL;
    }

    version_len = (uint32_t)strlen((const char *)version_record);

    if (buf_size < version_len + 1)
    {
        return FOTA_ERRCODE_INVALID_PARAMETER;
    }

    ret = fota_version_crc32_check(version_record, (uint8_t)version_len);
    if (FOTA_ERRCODE_SUCCESS != ret)
    {
        return ret;
    }

    memcpy(buffer, version_record, version_len);

    buffer[version_len] = '\0';

    return FOTA_ERRCODE_SUCCESS;
#else
    return FOTA_ERRCODE_UNSUPPORTED;
#endif
}    


FOTA_ERRCODE fota_version_read(uint8_t *buffer, uint8_t buf_size)
{
    uint8_t version_record[FOTA_MULTI_INFO_VERSION_RECORD_LEN] = {0};
    FOTA_ERRCODE ret = FOTA_ERRCODE_FAIL;
    uint8_t version_len = 0;

    if (!buffer || !buf_size)
    {
        return FOTA_ERRCODE_INVALID_PARAMETER;
    }

    ret = fota_multi_info_read(FOTA_MULTI_INFO_TYPE_VERSION,
                               version_record,
                               FOTA_MULTI_INFO_VERSION_RECORD_LEN);
    if (FOTA_ERRCODE_SUCCESS != ret)
    {
        return fota_version_read_from_nvdm(buffer, buf_size);
    }

    if (0xFF == version_record[0])
    {
        return fota_version_read_from_nvdm(buffer, buf_size);
    }

    if (version_record[FOTA_VERSION_MAX_SIZE - 1])
    {
        return fota_version_read_from_nvdm(buffer, buf_size);
    }

    version_len = strlen((const char *)version_record);

    if (buf_size < version_len + 1)
    {
        return FOTA_ERRCODE_INVALID_PARAMETER;
    }

    ret = fota_version_crc32_check(version_record, version_len);
    if (FOTA_ERRCODE_SUCCESS != ret)
    {
        return fota_version_read_from_nvdm(buffer, buf_size);
    }

    memcpy(buffer, version_record, version_len);

    buffer[version_len] = '\0';

    return FOTA_ERRCODE_SUCCESS;    
}


/* Version stored format: version + crc32 */
FOTA_ERRCODE fota_version_write(uint8_t *version, uint8_t version_len)
{
    FOTA_ERRCODE ret = FOTA_ERRCODE_FAIL;
    uint8_t version_record[FOTA_MULTI_INFO_VERSION_RECORD_LEN] = {0};
    uint32_t crc32 = 0xFFFFFF00, ver_len = 0;

    if (version_len)
    {
        if (!version)
        {
            return FOTA_ERRCODE_INVALID_PARAMETER;
        }

        ver_len = (uint32_t)strlen((const char *)version);
        if (version_len > ver_len)
        {
            version_len = ver_len;
        }
    }

    if (version_len >= FOTA_VERSION_MAX_SIZE)
    {
        return FOTA_ERRCODE_INVALID_PARAMETER;
    }

    if (version)
    {
        memcpy(version_record, version, version_len);
    }

    crc32 = CRC32_Generate(version_record, version_len, crc32);
    memcpy(version_record + FOTA_VERSION_MAX_SIZE, &crc32, FOTA_VERSION_INTEGRITY_CHECK_VALUE_SIZE);

    ret = fota_multi_info_write(FOTA_MULTI_INFO_TYPE_VERSION,
                                version_record,
                                FOTA_MULTI_INFO_VERSION_RECORD_LEN);
    if (FOTA_ERRCODE_OUT_OF_RANGE == ret)
    {
        ret = fota_multi_info_sector_clean(FOTA_MULTI_INFO_TYPE_VERSION, NULL);
        if (FOTA_ERRCODE_SUCCESS == ret)
        {
            ret = fota_multi_info_write(FOTA_MULTI_INFO_TYPE_VERSION,
                                        version_record,
                                        FOTA_MULTI_INFO_VERSION_RECORD_LEN);
        }
    }

    /* Backup version into NVDM */
    if (FOTA_ERRCODE_SUCCESS == ret)
    {
#ifdef MTK_NVDM_ENABLE
        nvdm_write_data_item(FOTA_NVDM_GROUP_NAME,
                             FOTA_NVDM_DATA_ITEM_VERSION,
                             NVDM_DATA_ITEM_TYPE_STRING,
                             (const uint8_t *)version_record,
                             FOTA_MULTI_INFO_VERSION_RECORD_LEN);
#endif
    }

    return ret;
}

