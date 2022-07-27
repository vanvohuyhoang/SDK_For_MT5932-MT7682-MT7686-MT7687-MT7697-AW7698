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

#include "bt_gap.h"
#include "bt_gap_le.h"
#include "bt_device_manager.h"
#include "bt_callback_manager.h"
#include "bt_device_manager_config.h"
#include "bt_device_manager_internal.h"
#include "bt_device_manager_db.h"

static bt_device_manager_db_remote_info_t *remote_list = NULL;

void bt_device_manager_remote_info_init(void)
{
    bt_dmgr_report_id("[BT_DM][REMOTE][I] Remote info init", 0);
    bt_device_manager_db_open(BT_DEVICE_MANAGER_DB_TYPE_REMOTE_INFO);
    remote_list = (bt_device_manager_db_remote_info_t *)bt_device_manager_db_read_data(BT_DEVICE_MANAGER_DB_TYPE_REMOTE_INFO);
    if (NULL == remote_list) {
        bt_device_manager_assert(0 && "read aws local info fail");
    }
}

bt_status_t bt_device_manager_remote_delete_info(bt_bd_addr_t *addr, bt_device_manager_remote_info_mask_t info_mask)
{
    uint32_t index = 0;
    bt_device_manager_db_remote_info_t *temp_remote = remote_list;
    bt_dmgr_report_id("[BT_DM][REMOTE][I] Delete info addr %p, info mask %d", 2, addr, info_mask);
    for(index = 0; (index < BT_DEVICE_MANAGER_MAX_PAIR_NUM) && temp_remote->sequence_num; temp_remote++, index++) {
        if (info_mask == 0 && addr == NULL) {
            temp_remote->sequence_num = 0;
            continue;
        } else if (addr == NULL) {
            temp_remote->info_valid_flag &= (~info_mask);
            continue;
        } else if (info_mask == 0 && !memcmp(&(temp_remote->address), addr, sizeof(bt_bd_addr_t))) {
            temp_remote->sequence_num = 0;
            break;
        } else if (!memcmp(&(temp_remote->address), addr, sizeof(bt_bd_addr_t))) {
            temp_remote->info_valid_flag &= (~info_mask);
            break;
        }
    }
    bt_device_manager_db_update_data(BT_DEVICE_MANAGER_DB_TYPE_REMOTE_INFO);
    return BT_STATUS_SUCCESS;
}

bt_status_t bt_device_manager_remote_top(bt_bd_addr_t addr)
{
    uint32_t index = 0;
    bt_device_manager_db_remote_info_t *temp_remote = remote_list;
    bt_device_manager_db_remote_info_t *find_remote = NULL;
    bt_dmgr_report_id("[BT_DM][REMOTE][I] Top device", 0);
    for (index = 0; index < BT_DEVICE_MANAGER_MAX_PAIR_NUM; index++, temp_remote++) {
        if (temp_remote->sequence_num && !memcmp(temp_remote->address, addr, sizeof(bt_bd_addr_t))) {
            find_remote = temp_remote;
            break;
        }
    }
    if (NULL == find_remote) {
        bt_dmgr_report_id("[BT_DM][REMOTE][W] Top device fail not find dev", 0);    
        return BT_STATUS_FAIL;
    }
    for (index = 0, temp_remote = remote_list; index < BT_DEVICE_MANAGER_MAX_PAIR_NUM; index++, temp_remote++) {
        if (temp_remote->sequence_num != 0 && temp_remote->sequence_num < find_remote->sequence_num) {
            temp_remote->sequence_num++;
        }
    }
    find_remote->sequence_num = 1;
    bt_device_manager_db_update_data(BT_DEVICE_MANAGER_DB_TYPE_REMOTE_INFO);
    return BT_STATUS_SUCCESS;    
}

bt_bd_addr_t *bt_device_manager_remote_get_dev_by_seq_num(uint8_t sequence)
{
    uint32_t index = 0;
    bt_device_manager_db_remote_info_t *temp_remote = remote_list;
    bt_dmgr_report_id("[BT_DM][REMOTE][I] Get dev by seq num %d", 1, sequence);
    if(0 != sequence) {
        for (index = 0; index < BT_DEVICE_MANAGER_MAX_PAIR_NUM; index++, temp_remote++) {
            if (sequence == temp_remote->sequence_num) {
                return &(temp_remote->address);
            }
        }
    }
    return NULL;    
}

uint32_t bt_device_manager_remote_get_paired_num(void)
{
    uint32_t ret = 0;
    uint32_t index = 0;
    bt_device_manager_db_remote_info_t *temp_remote = remote_list;
    bt_dmgr_report_id("[BT_DM][REMOTE][I] Get paiared num", 0);
    for (index = 0, temp_remote = remote_list; index < BT_DEVICE_MANAGER_MAX_PAIR_NUM; index++, temp_remote++) {
        if (temp_remote->sequence_num && (temp_remote->info_valid_flag & BT_DEVICE_MANAGER_REMOTE_INFO_MASK_PAIRED)) {
            ret++;
        }
    }
    return ret;
}

void bt_device_manager_remote_get_paired_list(bt_device_manager_paired_infomation_t* info, uint32_t* read_count)
{
    uint32_t count = 0;
    uint32_t index = 0;
    bt_device_manager_db_remote_info_t *temp_remote = remote_list;
    bt_dmgr_report_id("[BT_DM][REMOTE][I] Get Paired list", 0);
    if (NULL == info || *read_count == 0) {
        bt_dmgr_report_id("[BT_DM][REMOTE][E] Get Paired list error param info buffer : %p, read count : %d", 2, info, *read_count);
        return;
    }
    for (index = 0; index < BT_DEVICE_MANAGER_MAX_PAIR_NUM; index++, temp_remote++) {
        if ((count < *read_count) && temp_remote->sequence_num && (temp_remote->info_valid_flag & BT_DEVICE_MANAGER_REMOTE_INFO_MASK_PAIRED)) {
            memcpy(info->address, temp_remote->address, sizeof(bt_bd_addr_t));
            memcpy(info->name, temp_remote->paired_info.name, BT_GAP_MAX_DEVICE_NAME_LENGTH);
            count++;
        }
    }
    *read_count = count;
}

bt_status_t bt_device_manager_remote_find_paired_info(bt_bd_addr_t addr, bt_device_manager_db_remote_paired_info_t *info)
{
    uint32_t index = 0;
    bt_device_manager_db_remote_info_t *temp_remote = remote_list;
    bt_dmgr_report_id("[BT_DM][REMOTE][I] Find paired info", 0);
    if (NULL == info) {
        bt_dmgr_report_id("[BT_DM][REMOTE][E] Find paired info buffer is null", 0);
        return BT_STATUS_FAIL;
    }
    for (index = 0; index < BT_DEVICE_MANAGER_MAX_PAIR_NUM; index++, temp_remote++) {
        if (temp_remote->sequence_num && !memcmp(temp_remote->address, addr, sizeof(bt_bd_addr_t))) {
            if(temp_remote->info_valid_flag & BT_DEVICE_MANAGER_REMOTE_INFO_MASK_PAIRED) {
                memcpy(info, &(temp_remote->paired_info), sizeof(bt_device_manager_paired_infomation_t));
                bt_dmgr_report_id("[BT_DM][REMOTE][I] Find paired info success", 0);
                return BT_STATUS_SUCCESS;
            }
            break;
        }
    }
    return BT_STATUS_FAIL;
}

bt_status_t bt_device_manager_remote_find_paired_info_by_seq_num(uint8_t sequence, bt_device_manager_db_remote_paired_info_t *info)
{
    uint32_t index = 0;
    bt_device_manager_db_remote_info_t *temp_remote = remote_list;
    bt_dmgr_report_id("[BT_DM][REMOTE][I] Find paired info by sequence num %d", 1, sequence);
    if (NULL == info) {
        bt_dmgr_report_id("[BT_DM][REMOTE][E] Find paired info buffer is null", 0);
        return BT_STATUS_FAIL;
    }
    for (index = 0; index < BT_DEVICE_MANAGER_MAX_PAIR_NUM; index++, temp_remote++) {
        if (temp_remote->sequence_num && temp_remote->sequence_num == sequence) {
            if(temp_remote->info_valid_flag & BT_DEVICE_MANAGER_REMOTE_INFO_MASK_PAIRED) {
                memcpy(info, &(temp_remote->paired_info), sizeof(bt_device_manager_paired_infomation_t));
                bt_dmgr_report_id("[BT_DM][REMOTE][I] Find paired info success", 0);
                return BT_STATUS_SUCCESS;
            }
            break;
        }
    }
    return BT_STATUS_FAIL;
}

bt_status_t bt_device_manager_remote_update_paired_info(bt_bd_addr_t addr, bt_device_manager_db_remote_paired_info_t *info)
{
    uint8_t saved_flag = 0;
    uint32_t index = 0;
    bt_device_manager_db_remote_info_t *temp_remote = remote_list;
    bt_dmgr_report_id("[BT_DM][REMOTE][I] Update paired info", 0);
    if (NULL == info) {
        bt_dmgr_report_id("[BT_DM][REMOTE][E] Update paired info buffer is null", 0);
        return BT_STATUS_FAIL;
    }
    for (index = 0; index < BT_DEVICE_MANAGER_MAX_PAIR_NUM; index++, temp_remote++) {
        if (temp_remote->sequence_num && !memcmp(temp_remote->address, addr, sizeof(bt_bd_addr_t))) {
            memcpy(&(temp_remote->paired_info), info, sizeof(bt_device_manager_db_remote_paired_info_t));
            temp_remote->info_valid_flag |= BT_DEVICE_MANAGER_REMOTE_INFO_MASK_PAIRED;
            bt_device_manager_db_update_data(BT_DEVICE_MANAGER_DB_TYPE_REMOTE_INFO);
            return BT_STATUS_SUCCESS;
        }
    }
    bt_dmgr_report_id("[BT_DM][REMOTE][I] Create a item to save paired info ", 0);
    for (index = 0, temp_remote = remote_list; index < BT_DEVICE_MANAGER_MAX_PAIR_NUM; index++, temp_remote++) {
        if (temp_remote->sequence_num == 0 || temp_remote->sequence_num == BT_DEVICE_MANAGER_MAX_PAIR_NUM) {
            if (saved_flag == 0) {
                memcpy(temp_remote->address, addr, sizeof(bt_bd_addr_t));
                memcpy(&(temp_remote->paired_info), info, sizeof(bt_device_manager_db_remote_paired_info_t));
                temp_remote->info_valid_flag = BT_DEVICE_MANAGER_REMOTE_INFO_MASK_PAIRED;
                temp_remote->sequence_num = 1;
                saved_flag = 1;
            }
        } else {
            temp_remote->sequence_num++;
        }
    }
    bt_device_manager_db_update_data(BT_DEVICE_MANAGER_DB_TYPE_REMOTE_INFO);
    return BT_STATUS_SUCCESS;
}

bt_status_t bt_device_manager_remote_find_version_info(bt_bd_addr_t addr, bt_device_manager_db_remote_version_info_t *info)
{
    uint32_t index = 0;
    bt_device_manager_db_remote_info_t *temp_remote = remote_list;
    bt_dmgr_report_id("[BT_DM][REMOTE][I] Find version info", 0);
    if (NULL == info) {
        bt_dmgr_report_id("[BT_DM][REMOTE][E] Find version info buffer is null", 0);
        return BT_STATUS_FAIL;
    }
    for (index = 0; index < BT_DEVICE_MANAGER_MAX_PAIR_NUM; index++, temp_remote++) {
        if (temp_remote->sequence_num && !memcmp(temp_remote->address, addr, sizeof(bt_bd_addr_t))) {
            if(temp_remote->info_valid_flag & BT_DEVICE_MANAGER_REMOTE_INFO_MASK_VERSION) {
                memcpy(info, &(temp_remote->version_info.remote_version), sizeof(bt_device_manager_db_remote_version_info_t));
                bt_dmgr_report_id("[BT_DM][REMOTE][I] Find version info success", 0);
                return BT_STATUS_SUCCESS;
            }            
            break;
        }
    }
    return BT_STATUS_FAIL;
}

bt_status_t bt_device_manager_remote_update_version_info(bt_bd_addr_t addr, bt_device_manager_db_remote_version_info_t *info)
{
    uint8_t saved_flag = 0;
    uint32_t index = 0;
    bt_device_manager_db_remote_info_t *temp_remote = remote_list;
    bt_dmgr_report_id("[BT_DM][REMOTE][I] Update version info", 0);
    if (NULL == info) {
        bt_dmgr_report_id("[BT_DM][REMOTE][E] Update version info buffer is null", 0);
        return BT_STATUS_FAIL;
    }
    for (index = 0; index < BT_DEVICE_MANAGER_MAX_PAIR_NUM; index++, temp_remote++) {
        if (temp_remote->sequence_num && !memcmp(temp_remote->address, addr, sizeof(bt_bd_addr_t))) {
            memcpy(&(temp_remote->version_info.remote_version), info, sizeof(bt_device_manager_db_remote_version_info_t));
            temp_remote->info_valid_flag |= BT_DEVICE_MANAGER_REMOTE_INFO_MASK_VERSION;
            bt_device_manager_db_update_data(BT_DEVICE_MANAGER_DB_TYPE_REMOTE_INFO);
            return BT_STATUS_SUCCESS;
        }
    }
    bt_dmgr_report_id("[BT_DM][REMOTE][I] Create a new item to save version info ", 0);
    for (index = 0, temp_remote = remote_list; index < BT_DEVICE_MANAGER_MAX_PAIR_NUM; index++, temp_remote++) {
        if (temp_remote->sequence_num == 0 || temp_remote->sequence_num == BT_DEVICE_MANAGER_MAX_PAIR_NUM) {
            if (saved_flag == 0) {
                memcpy(temp_remote->address, addr, sizeof(bt_bd_addr_t));
                memcpy(&(temp_remote->version_info.remote_version), info, sizeof(bt_device_manager_paired_controller_version_t));
                temp_remote->info_valid_flag = BT_DEVICE_MANAGER_REMOTE_INFO_MASK_VERSION;
                temp_remote->sequence_num = 1;
                saved_flag = 1;
            }
        } else {
            temp_remote->sequence_num++;
        }
    }
    bt_device_manager_db_update_data(BT_DEVICE_MANAGER_DB_TYPE_REMOTE_INFO);
    return BT_STATUS_SUCCESS;
}

bt_status_t bt_device_manager_remote_find_profile_info(bt_bd_addr_t addr, bt_device_manager_db_remote_profile_info_t *info)
{
    uint32_t index = 0;
    bt_device_manager_db_remote_info_t *temp_remote = remote_list;
    bt_dmgr_report_id("[BT_DM][REMOTE][I] Find profile info", 0);
    if (NULL == info) {
        bt_dmgr_report_id("[BT_DM][REMOTE][E] Find profile info buffer is null", 0);
        return BT_STATUS_FAIL;
    }
    for (index = 0; index < BT_DEVICE_MANAGER_MAX_PAIR_NUM; index++, temp_remote++) {
        if (temp_remote->sequence_num && !memcmp(temp_remote->address, addr, sizeof(bt_bd_addr_t))) {
            if(temp_remote->info_valid_flag & BT_DEVICE_MANAGER_REMOTE_INFO_MASK_PROFILE) {
                memcpy(info, &(temp_remote->profile_info), sizeof(bt_device_manager_db_remote_profile_info_t));
                bt_dmgr_report_id("[BT_DM][REMOTE][I] Find profile info success", 0);
                return BT_STATUS_SUCCESS;
            }
            break;
        }
    }
    return BT_STATUS_FAIL;
}

bt_status_t bt_device_manager_remote_update_profile_info(bt_bd_addr_t addr, bt_device_manager_db_remote_profile_info_t *info)
{
    uint8_t saved_flag = 0;
    uint32_t index = 0;
    bt_device_manager_db_remote_info_t *temp_remote = remote_list;
    bt_dmgr_report_id("[BT_DM][REMOTE][I] Update profile info", 0);
    if (NULL == info) {
        bt_dmgr_report_id("[BT_DM][REMOTE][E] Update profile info buffer is null", 0);
        return BT_STATUS_FAIL;
    }
    for (index = 0; index < BT_DEVICE_MANAGER_MAX_PAIR_NUM; index++, temp_remote++) {
        if (temp_remote->sequence_num && !memcmp(temp_remote->address, addr, sizeof(bt_bd_addr_t))) {
            memcpy(&(temp_remote->profile_info), info, sizeof(bt_device_manager_db_remote_profile_info_t));
            temp_remote->info_valid_flag |= BT_DEVICE_MANAGER_REMOTE_INFO_MASK_PROFILE;
            bt_device_manager_db_update_data(BT_DEVICE_MANAGER_DB_TYPE_REMOTE_INFO);
            return BT_STATUS_SUCCESS;
        }
    }
    bt_dmgr_report_id("[BT_DM][REMOTE][I] Create a new item to save profile info ", 0);
    for (index = 0, temp_remote = remote_list; index < BT_DEVICE_MANAGER_MAX_PAIR_NUM; index++, temp_remote++) {
        if (temp_remote->sequence_num == 0 || temp_remote->sequence_num == BT_DEVICE_MANAGER_MAX_PAIR_NUM) {
            if (saved_flag == 0) {
                memcpy(temp_remote->address, addr, sizeof(bt_bd_addr_t));
                memcpy(&(temp_remote->profile_info), info, sizeof(bt_device_manager_db_remote_profile_info_t));
                temp_remote->info_valid_flag = BT_DEVICE_MANAGER_REMOTE_INFO_MASK_PROFILE;
                temp_remote->sequence_num = 1;
                saved_flag = 1;
            }
        } else {
            temp_remote->sequence_num++;
        }
    }
    bt_device_manager_db_update_data(BT_DEVICE_MANAGER_DB_TYPE_REMOTE_INFO);
    return BT_STATUS_SUCCESS;
}


