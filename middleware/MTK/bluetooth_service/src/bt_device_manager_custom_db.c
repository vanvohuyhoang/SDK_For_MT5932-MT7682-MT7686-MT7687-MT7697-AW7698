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

#ifdef MTK_NVDM_ENABLE
#include "nvdm.h"
#include "nvkey.h"
#include "nvkey_id_list.h"
#endif /* MTK_NVDM_ENABLE */
#include "bt_device_manager_internal.h"
#include "bt_device_manager_custom_db.h"

#define BT_DEVICE_MANAGER_LOCAL_INFO_NVDM_GROUP_INFO         "BT"
#define BT_DEVICE_MANAGER_LOCAL_INFO_NVDM_ITEM_LOCAL_ADDR    "local_addr"

#ifdef MTK_NVDM_ENABLE
static bool bt_device_manager_custom_db_rw_local_info(uint8_t *buffer, uint32_t *size, bool read)
{
    if (true == read) {
        uint32_t temp_size = *size;
        nvdm_status_t result = nvdm_read_data_item(BT_DEVICE_MANAGER_LOCAL_INFO_NVDM_GROUP_INFO,
                                    BT_DEVICE_MANAGER_LOCAL_INFO_NVDM_ITEM_LOCAL_ADDR, buffer, &temp_size);
        bt_dmgr_report_id("[BT_DM][CUSTOM_DB][I] Read local info size %d", 1, *size);
        if (NVDM_STATUS_ITEM_NOT_FOUND == result) {
            memset(buffer, 0, *size);
            result = nvdm_write_data_item(BT_DEVICE_MANAGER_LOCAL_INFO_NVDM_GROUP_INFO,
                                    BT_DEVICE_MANAGER_LOCAL_INFO_NVDM_ITEM_LOCAL_ADDR,
                                    NVDM_DATA_ITEM_TYPE_RAW_DATA, buffer, *size);
        }

        if (NVDM_STATUS_OK != result) {
            bt_dmgr_report_id("[BT_DM][CUSTOM_DB][E] Read local info fail status : %d", 1, result);
            return false;
        }
    } else {
        nvdm_status_t result = nvdm_write_data_item(BT_DEVICE_MANAGER_LOCAL_INFO_NVDM_GROUP_INFO,
                                    BT_DEVICE_MANAGER_LOCAL_INFO_NVDM_ITEM_LOCAL_ADDR,
                                    NVDM_DATA_ITEM_TYPE_RAW_DATA, buffer, *size);
        bt_dmgr_report_id("[BT_DM][CUSTOM_DB][I] Write local info size %d", 1, *size);
        if (NVDM_STATUS_OK != result) {
            bt_dmgr_report_id("[BT_DM][CUSTOM_DB][E] Write local info fail status : %d", 1, result);
            return false;
        }
    }
    return true;
}

static bool bt_device_manager_custom_db_rw_aws_local_info(uint8_t *buffer, uint32_t *size, bool read)
{
    if (true == read) {
        nvkey_status_t result = nvkey_read_data(
                                    NVKEYID_PROFILE_AWS_MCE_CONFIG, buffer, size);
        bt_dmgr_report_id("[BT_DM][CUSTOM_DB][I] Read aws local info size %d", 1, *size);

        if (NVKEY_STATUS_OK != result) {
            bt_dmgr_report_id("[BT_DM][CUSTOM_DB][E] Read aws local info status : %d", 1, result);
            return false;
        }
    } else {
        nvkey_status_t result = nvkey_write_data(NVKEYID_PROFILE_AWS_MCE_CONFIG, buffer, *size);
        bt_dmgr_report_id("[BT_DM][CUSTOM_DB][I] Write aws local info size %d", 1, *size);
        if (NVKEY_STATUS_OK != result) {
            bt_dmgr_report_id("[BT_DM][CUSTOM_DB][E] write aws local info status : %d", 1, result);
            return false;
        }
    }
    return true;
}

bool bt_device_manager_custom_db_read(bt_device_manager_custom_db_item_t db_type, uint8_t *buffer, uint32_t *size)
{
    if (NULL == buffer) {
        bt_dmgr_report_id("[BT_DM][CUSTOM_DB][E] Read local info buffer is NULL", 0);
        return false;
    }
    if (BT_DEVICE_MANAGER_CUSTOM_DB_ITEM_LOCAL_ALL == db_type) {
        return bt_device_manager_custom_db_rw_local_info(buffer, size, true);
    } else if (BT_DEVICE_MANAGER_CUSTOM_DB_ITEM_AWS_LOCAL == db_type) {
        return bt_device_manager_custom_db_rw_aws_local_info(buffer, size, true);
    } else {
        bt_dmgr_report_id("[BT_DM][CUSTOM_DB][E] read custom DB type %d not define", 1, db_type);
    }
    return false;
}

bool bt_device_manager_custom_db_write(bt_device_manager_custom_db_item_t db_type, uint8_t *buffer, uint32_t size)
{
    if (NULL == buffer) {
        bt_dmgr_report_id("[BT_DM][CUSTOM_DB][E] Write local info buffer is NULL", 0);
        return false;
    }
    if (BT_DEVICE_MANAGER_CUSTOM_DB_ITEM_LOCAL_ALL == db_type) {
        return bt_device_manager_custom_db_rw_local_info(buffer, &size, false);
    } else if (BT_DEVICE_MANAGER_CUSTOM_DB_ITEM_AWS_LOCAL == db_type) {
        return bt_device_manager_custom_db_rw_aws_local_info(buffer, &size, false);
    } else {
        bt_dmgr_report_id("[BT_DM][CUSTOM_DB][E] Write custom DB type %d not define", 1, db_type);
    }
    return false;
}

/*
bool bt_device_manager_coustom_db_write_non_block(bt_device_manager_db_type_t info_type,
                                                                uint8_t *buffer, uint32_t size,
                                                                const nvdm_user_callback_t callback,
                                                                const void *user_data)
{
    if (NULL == buffer) {
        bt_dmgr_report_id("[BT_DM][CUSTOM_DB][E] Write local info buffer is NULL", 0);
        return false;
    }
    if (BT_DEVICE_MANAGER_DB_TYPE_LOCAL_INFO == info_type || BT_DEVICE_MANAGER_DB_TYPE_REMOTE_INFO == info_type) {
        return bt_device_manager_custom_db_rw_local_info(buffer, &size, false);
    } else if (BT_DEVICE_MANAGER_DB_TYPE_AWS_LOCAL_INFO = info_type) {
        return bt_device_manager_custom_db_rw_local_info(buffer, &size, false);
    } else {
        bt_dmgr_report_id("[BT_DM][CUSTOM_DB][E] write custom DB type %d not define", 1, info_type);
    }
    return false;
}*/

#endif


