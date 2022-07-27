/* Copyright Statement:
 *
 * (C) 2005-2016  MediaTek Inc. All rights reserved.
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. ("MediaTek") and/or its licensors.
 * Without the prior written permission of MediaTek and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 * You may only use, reproduce, modify, or distribute (as applicable) MediaTek Software
 * if you have agreed to and been bound by the applicable license agreement with
 * MediaTek ("License Agreement") and been granted explicit permission to do so within
 * the License Agreement ("Permitted User").  If you are not a Permitted User,
 * please cease any access or use of MediaTek Software immediately.
 * BY OPENING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 * THAT MEDIATEK SOFTWARE RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES
 * ARE PROVIDED TO RECEIVER ON AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL
 * WARRANTIES, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR NONINFRINGEMENT.
 * NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH RESPECT TO THE
 * SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY, INCORPORATED IN, OR
 * SUPPLIED WITH MEDIATEK SOFTWARE, AND RECEIVER AGREES TO LOOK ONLY TO SUCH
 * THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO. RECEIVER EXPRESSLY ACKNOWLEDGES
 * THAT IT IS RECEIVER'S SOLE RESPONSIBILITY TO OBTAIN FROM ANY THIRD PARTY ALL PROPER LICENSES
 * CONTAINED IN MEDIATEK SOFTWARE. MEDIATEK SHALL ALSO NOT BE RESPONSIBLE FOR ANY MEDIATEK
 * SOFTWARE RELEASES MADE TO RECEIVER'S SPECIFICATION OR TO CONFORM TO A PARTICULAR
 * STANDARD OR OPEN FORUM. RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S ENTIRE AND
 * CUMULATIVE LIABILITY WITH RESPECT TO MEDIATEK SOFTWARE RELEASED HEREUNDER WILL BE,
 * AT MEDIATEK'S OPTION, TO REVISE OR REPLACE MEDIATEK SOFTWARE AT ISSUE,
 * OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE CHARGE PAID BY RECEIVER TO
 * MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 */

#include "bt_callback_manager.h"
#include "bt_connection_manager_internal.h"
#include "bt_device_manager_config.h"
#include "bt_device_manager.h"
#include "bt_device_manager_internal.h"
#include "bt_gap.h"
#include "bt_gap_le.h"
#include "bt_type.h"
#include "bt_debug.h"
#include "bt_os_layer_api.h"
#ifdef MTK_NVDM_ENABLE
#include "nvdm.h"
#endif
#include <string.h>
#include <stdio.h>
#include "syslog.h"
#include "bt_device_manager_db.h"


#define __BT_DEVICE_MANAGER_DEBUG_INFO__

void bt_sink_paird_list_changed(bt_device_manager_paired_event_t event, bt_bd_addr_ptr_t address);
void default_bt_sink_paird_list_changed(bt_device_manager_paired_event_t event, bt_bd_addr_ptr_t address);
//static bt_status_t bt_device_manager_delete_paired_device_int(bt_bd_addr_ptr_t address);

/* Weak symbol declaration */
#if _MSC_VER >= 1500
#pragma comment(linker, "/alternatename:_bt_sink_paird_list_changed=_default_bt_sink_paird_list_changed")
#elif defined(__GNUC__) || defined(__ICCARM__) || defined(__CC_ARM)
#pragma weak bt_sink_paird_list_changed = default_bt_sink_paird_list_changed
#else
#error "Unsupported Platform"
#endif

static bt_device_manager_db_local_info_t *local_info = NULL;
static int32_t bt_device_manager_is_init;
static bt_gap_io_capability_t bt_device_manager_io_capability = 0xFF;

log_create_module(BT_DM_EDR, PRINT_LEVEL_INFO);

void bt_device_manager_dump_link_key(uint8_t *linkkey)
{
    bt_dmgr_report_id("[BT_DM] link key:%02x,%02x,%02x,%02x,  %02x,%02x,%02x,%02x,  %02x,%02x,%02x,%02x,  %02x,%02x,%02x,%02x", 16,
                       linkkey[0], linkkey[1], linkkey[2], linkkey[3], linkkey[4], linkkey[5], linkkey[6], linkkey[7],
                       linkkey[8], linkkey[9], linkkey[10], linkkey[11], linkkey[12], linkkey[13], linkkey[14], linkkey[15]);
}

void bt_device_manager_dump_bt_address(bt_device_manager_local_info_address_type_t addr_type, uint8_t *address)
{
    bt_dmgr_report_id("[BT_DM] Addr type %d, address:0x%02x:0x%02x:0x%02x:0x%02x:0x%02x:0x%02x", 7, addr_type,
         address[5], address[4], address[3], address[2], address[1], address[0]);
}

void default_bt_sink_paird_list_changed(bt_device_manager_paired_event_t event, bt_bd_addr_ptr_t address)
{
    return;
}

/*
static void bt_device_manager_notify_paired_list_changed(bt_device_manager_paired_event_t event, bt_bd_addr_ptr_t address)
{
    bt_sink_paird_list_changed(event, address);
    return;
}
*/

bt_status_t bt_device_manager_gap_event_handler(bt_msg_type_t msg, bt_status_t status, void *buff);
void bt_device_manager_get_link_key_handler(bt_gap_link_key_notification_ind_t* key_information);

void bt_device_manager_init(void)
{
    bt_dmgr_report_id("[BT_DM][I]local info init", 0);
    if (bt_device_manager_is_init == 0) {
        bt_callback_manager_register_callback(bt_callback_type_app_event, MODULE_MASK_GAP | MODULE_MASK_SYSTEM, (void*)bt_device_manager_gap_event_handler);
        bt_callback_manager_register_callback(bt_callback_type_gap_get_link_key, MODULE_MASK_GAP, (void*)bt_device_manager_get_link_key_handler);
        bt_device_manager_db_init();
        bt_device_manager_db_open(BT_DEVICE_MANAGER_DB_TYPE_LOCAL_INFO);
        local_info = (bt_device_manager_db_local_info_t*)bt_device_manager_db_read_data(BT_DEVICE_MANAGER_DB_TYPE_LOCAL_INFO);
        if (NULL == local_info) {
            bt_device_manager_assert(0 && "read local info fail");
        }
        bt_device_manager_remote_info_init();
        bt_device_manager_aws_local_info_init();
        bt_device_manager_is_init = 1;
    }
}

uint32_t bt_device_manager_get_paired_number(void)
{
    return bt_device_manager_remote_get_paired_num();
}

void bt_device_manager_get_paired_list(bt_device_manager_paired_infomation_t* info, uint32_t* read_count)
{
    bt_device_manager_remote_get_paired_list(info, read_count);
}

bool bt_device_manager_is_paired(bt_bd_addr_ptr_t address)
{
    bt_status_t status = BT_STATUS_SUCCESS;
    bt_device_manager_db_remote_paired_info_t temp_info;
    status = bt_device_manager_remote_find_paired_info((uint8_t *)address, &temp_info);
    if (status != BT_STATUS_SUCCESS) {
        return false;
    }
    return true;
}

bt_status_t bt_device_manager_delete_paired_device(bt_bd_addr_ptr_t address)
{
    return bt_device_manager_remote_delete_info((bt_bd_addr_t *)address, BT_DEVICE_MANAGER_REMOTE_INFO_MASK_PAIRED);
}

void bt_device_manager_get_link_key_handler(bt_gap_link_key_notification_ind_t* key_information)
{
    bt_status_t status = BT_STATUS_SUCCESS;
    bt_device_manager_db_remote_paired_info_t temp_info;
    memset(&temp_info, 0, sizeof(temp_info));
    status = bt_device_manager_remote_find_paired_info(key_information->address, &temp_info);
    if (status == BT_STATUS_SUCCESS) {
        memcpy(key_information, &(temp_info.paired_key), sizeof(bt_gap_link_key_notification_ind_t));
    #ifdef __BT_DEVICE_MANAGER_DEBUG_INFO__
        bt_device_manager_dump_bt_address(BT_DEVICE_MANAGER_LOCAL_INFO_ADDRESS_REMOTE, (uint8_t *)key_information->address);
        bt_device_manager_dump_link_key((uint8_t *)key_information->key);
    #endif
    }
}

bt_status_t bt_device_manager_unpair_all(void)
{
    return bt_device_manager_remote_delete_info(NULL,
        BT_DEVICE_MANAGER_REMOTE_INFO_MASK_PAIRED | BT_DEVICE_MANAGER_REMOTE_INFO_MASK_VERSION);
}

#ifdef __MTK_AVM_DIRECT__
void bt_device_manager_set_io_capability(bt_gap_io_capability_t io_capability)
{
     bt_device_manager_io_capability = io_capability;
}
#endif

bt_status_t bt_device_manager_gap_event_handler(bt_msg_type_t msg, bt_status_t status, void *buff)
{
    //bt_status_t st;
    switch (msg) {
        case BT_GAP_IO_CAPABILITY_REQ_IND: {
            if(0xFF == bt_device_manager_io_capability) {
                bt_gap_reply_io_capability_request(BT_GAP_OOB_DATA_PRESENTED_NONE, BT_GAP_SECURITY_AUTH_REQUEST_GENERAL_BONDING_AUTO_ACCEPTED);
            } else {
#ifdef __MTK_AVM_DIRECT__
                bt_gap_reply_extend_io_capability_request(BT_GAP_OOB_DATA_PRESENTED_NONE, 
                    BT_GAP_SECURITY_AUTH_REQUEST_MITM_DEDICATED_BONDING, bt_device_manager_io_capability);
#endif
            }
            break;
        }
        case BT_GAP_LINK_KEY_NOTIFICATION_IND: {
            // This event is received before BT_GAP_BONDING_COMPLETE_IND and bonding success or the old link key is phased out.
            bt_gap_link_key_notification_ind_t *key_info = (bt_gap_link_key_notification_ind_t *)buff;
            bt_device_manager_db_remote_paired_info_t paired_info;
            memset(&paired_info, 0, sizeof(paired_info));
            bt_device_manager_remote_find_paired_info(key_info->address, &paired_info);
            memcpy(&(paired_info.paired_key), key_info, sizeof(bt_gap_link_key_notification_ind_t));

            if (key_info->key_type != BT_GAP_LINK_KEY_TYPE_INVAILIDE) {
                bt_device_manager_remote_update_paired_info(key_info->address, &paired_info);
                bt_gap_read_remote_name((const bt_bd_addr_t*)(&(key_info->address)));
            } else {
                bt_device_manager_remote_delete_info(&(key_info->address), BT_DEVICE_MANAGER_DB_TYPE_REMOTE_INFO);
            }
            #ifdef __BT_DEVICE_MANAGER_DEBUG_INFO__
            bt_device_manager_dump_bt_address(BT_DEVICE_MANAGER_LOCAL_INFO_ADDRESS_REMOTE, (uint8_t *)key_info->address);
            bt_device_manager_dump_link_key((uint8_t *)key_info->key);
            #endif
            break;
        }
        case BT_GAP_READ_REMOTE_NAME_COMPLETE_IND: {
            bt_gap_read_remote_name_complete_ind_t* p = (bt_gap_read_remote_name_complete_ind_t*)buff;
            if (NULL == p) {
                bt_dmgr_report_id("[BT_DM][W] read remote name complete ind buff is NULL", 0);
                break;
            } else {
                bt_gap_connection_handle_t gap_handle = bt_sink_srv_cm_get_gap_handle((bt_bd_addr_t *)(p->address));
                bt_device_manager_db_remote_paired_info_t paired_info;
                memset(&paired_info, 0, sizeof(paired_info));
                bt_device_manager_remote_find_paired_info((uint8_t *)(p->address), &paired_info);
                memcpy(&(paired_info.name), p->name, BT_GAP_MAX_DEVICE_NAME_LENGTH);
                bt_device_manager_remote_update_paired_info((uint8_t *)(p->address), &paired_info);
                status = bt_gap_read_remote_version_information(gap_handle);
                if (status != BT_STATUS_SUCCESS) {
                    bt_dmgr_report_id("[BT_DM][W] read remote version fail status 0x%x", 1, status);
                    break;
                }
            }
            break;
        }
        case BT_GAP_READ_REMOTE_VERSION_CNF: {
            if (status != BT_STATUS_SUCCESS) {
                bt_dmgr_report_id("[BT_DM][W] read remote version cnf fail status 0x%x", 1, status);
            }
            break;
        }
        case BT_GAP_READ_REMOTE_VERSION_COMPLETE_IND: {
            bt_gap_read_remote_version_complete_ind_t *complete_ind = (bt_gap_read_remote_version_complete_ind_t *)buff;
            bt_device_manager_db_remote_version_info_t version_info;
            bt_bd_addr_t *remote_device = NULL;
            if (BT_STATUS_SUCCESS != status || NULL == complete_ind) {
                bt_dmgr_report_id("[BT_DM][W] read remote version complete ind fail status 0x%x complete_ind %p", 2, status, complete_ind);
                break;
            } else if (NULL == (remote_device = bt_connection_mananger_get_addr_by_gap_handle(complete_ind->handle))) {
                bt_dmgr_report_id("[BT_DM][E] read remote version can't find device by handle 0x%x status 0x%x", 2, complete_ind->handle, status);
                break;
            }
            version_info.remote_version.version = complete_ind->version;
            version_info.remote_version.manufacturer_id = complete_ind->manufacturer_id;
            version_info.remote_version.subversion = complete_ind->subversion;
            bt_device_manager_remote_update_version_info(*remote_device, &version_info);
            break;
        }
        default:
            break;
    }
    return BT_STATUS_SUCCESS;
}

uint32_t bt_device_manager_get_complete_paired_list(bt_device_manager_paired_device_complete_infomation_t* info, uint32_t read_count)
{
    return 0;
}

bt_status_t bt_device_manager_get_complete_paired_info(bt_bd_addr_ptr_t address, bt_device_manager_paired_device_complete_infomation_t *info)
{

    return 0;
}

bt_status_t bt_device_manager_set_complete_paired_info(bt_bd_addr_ptr_t address, bt_device_manager_paired_device_complete_infomation_t *info)
{
    return bt_device_manager_remote_update_paired_info((uint8_t *)address, (bt_device_manager_db_remote_paired_info_t *)info);
}

/*
void default_bt_device_manager_set_local_address(bt_bd_addr_t *addr)
{
    bt_dmgr_report_id("[BT_DM][I] Set local address:", 0);
    bt_connection_manager_device_local_info_dump_address(
        BT_CONNECTION_MANAGER_DEVICE_LOCAL_INFO_ADDRESS_TYPE_AWS_LOCAL, addr);
    if (bt_connection_manager_memcmp(&local_info->local_address, addr, sizeof(bt_bd_addr_t))) {
        bt_connection_manager_memcpy(&local_info->local_address, addr, sizeof(bt_bd_addr_t));
        bt_device_manager_db_update_data(BT_DEVICE_MANAGER_DB_TYPE_AWS_LOCAL_INFO);
    }
}

bt_bd_addr_t* default_bt_device_manager_get_local_address(void)
{
    bt_dmgr_report_id("[BT_DM][I] Get local address:", 0);
    bt_connection_manager_device_local_info_dump_address(
        BT_CONNECTION_MANAGER_DEVICE_LOCAL_INFO_ADDRESS_TYPE_AWS_LOCAL, &local_info->local_address);
    return &local_info->local_address;
}*/

void bt_device_manager_store_local_address(bt_bd_addr_t *addr)
{
    bt_dmgr_report_id("[BT_DM][I] Set local address:", 0);
    bt_device_manager_dump_bt_address(BT_DEVICE_MANAGER_LOCAL_INFO_ADDRESS_LOCAL, (uint8_t *)addr);
    if (memcmp(&local_info->local_address, addr, sizeof(bt_bd_addr_t))) {
        memcpy(&local_info->local_address, addr, sizeof(bt_bd_addr_t));
        bt_device_manager_db_update_data(BT_DEVICE_MANAGER_DB_TYPE_LOCAL_INFO);
    }
}

bt_bd_addr_t* bt_device_manager_get_local_address(void)
{
    bt_dmgr_report_id("[BT_DM][I] Get local address:", 0);
    bt_device_manager_dump_bt_address(BT_DEVICE_MANAGER_LOCAL_INFO_ADDRESS_LOCAL, (uint8_t *)&(local_info->local_address));
    return &local_info->local_address;
}


