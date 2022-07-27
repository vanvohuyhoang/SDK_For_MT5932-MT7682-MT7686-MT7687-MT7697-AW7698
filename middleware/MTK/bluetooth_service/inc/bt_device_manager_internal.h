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

#ifndef __BT_DEVICE_MANAGER_INTERNAL_H__
#define __BT_DEVICE_MANAGER_INTERNAL_H__

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#ifndef WIN32
#include <syslog.h>
#else
#include "osapi.h"
#endif
#include "FreeRTOSConfig.h"
#include "bt_type.h"
#include "bt_gap.h"
#include "bt_sink_srv.h"
#include "bt_device_manager.h"
#include "bt_connection_manager_internal.h"

#ifdef __cplusplus
extern "C"
{
#endif /* __cplusplus */

#ifndef PACKED
#define PACKED  __attribute__((packed))
#endif

typedef struct {
    bt_gap_link_key_notification_ind_t paired_key;
    char name[BT_GAP_MAX_DEVICE_NAME_LENGTH + 1];
} bt_device_manager_db_remote_paired_info_t;

typedef struct {
    bt_device_manager_paired_controller_version_t remote_version;
} bt_device_manager_db_remote_version_info_t;

typedef struct {
#ifdef BT_SINK_SRV_HFP_STORAGE_SIZE
    uint8_t hfp_info[BT_SINK_SRV_HFP_STORAGE_SIZE];
#endif
#ifdef BT_SINK_SRV_A2DP_STORAGE_SIZE
    uint8_t a2dp_info[BT_SINK_SRV_A2DP_STORAGE_SIZE];
#endif
#ifdef BT_SINK_SRV_AVRCP_STORAGE_SIZE
    uint8_t avrcp_info[BT_SINK_SRV_AVRCP_STORAGE_SIZE];
#endif
#ifdef BT_SINK_SRV_PBAP_STORAGE_SIZE
    uint8_t pbap_info[BT_SINK_SRV_PBAP_STORAGE_SIZE];
#endif
} bt_device_manager_db_remote_profile_info_t;

#define BT_DEVICE_MANAGER_REMOTE_INFO_MASK_PAIRED  0x01
#define BT_DEVICE_MANAGER_REMOTE_INFO_MASK_VERSION 0x02
#define BT_DEVICE_MANAGER_REMOTE_INFO_MASK_PROFILE 0x04
typedef uint8_t bt_device_manager_remote_info_mask_t;

#define BT_DEVICE_MANAGER_LOCAL_INFO_ADDRESS_LOCAL      (0x00)
#define BT_DEVICE_MANAGER_LOCAL_INFO_ADDRESS_AWS_LOCAL  (0x01)
#define BT_DEVICE_MANAGER_LOCAL_INFO_ADDRESS_AWS_PEER   (0x02)
#define BT_DEVICE_MANAGER_LOCAL_INFO_ADDRESS_REMOTE     (0x03)
typedef uint8_t bt_device_manager_local_info_address_type_t;

#define bt_device_manager_assert configASSERT
#define __BT_DEVICE_MANAGER_DEBUG_INFO__ 
#ifdef __BT_DEVICE_MANAGER_DEBUG_INFO__
//#define bt_dmgr_report_id(_message, arg_cnt, ...) printf(_message, ##__VA_ARGS__)
#define bt_dmgr_report_id(_message, arg_cnt, ...) LOG_MSGID_I(BT_DM_EDR, _message, arg_cnt, ##__VA_ARGS__)
#else
#define bt_dmgr_report_id(_message, arg_cnt, ...);
#endif

void bt_device_manager_dump_link_key(uint8_t *linkkey);
void bt_device_manager_dump_bt_address(bt_device_manager_local_info_address_type_t addr_type, uint8_t *address);

bt_status_t bt_device_manager_remote_delete_info(bt_bd_addr_t *addr, bt_device_manager_remote_info_mask_t info_mask);//Todo
uint32_t bt_device_manager_remote_get_paired_num(void);
bt_status_t bt_device_manager_remote_top(bt_bd_addr_t addr);
bt_bd_addr_t *bt_device_manager_remote_get_dev_by_seq_num(uint8_t sequence);
void bt_device_manager_remote_get_paired_list(bt_device_manager_paired_infomation_t* info, uint32_t* read_count);
bt_status_t bt_device_manager_remote_find_paired_info(bt_bd_addr_t addr, bt_device_manager_db_remote_paired_info_t *info);
bt_status_t bt_device_manager_remote_find_paired_info_by_seq_num(uint8_t sequence, bt_device_manager_db_remote_paired_info_t *info);
bt_status_t bt_device_manager_remote_update_paired_info(bt_bd_addr_t addr, bt_device_manager_db_remote_paired_info_t *info);
bt_status_t bt_device_manager_remote_find_version_info(bt_bd_addr_t addr, bt_device_manager_db_remote_version_info_t *info);
bt_status_t bt_device_manager_remote_update_version_info(bt_bd_addr_t addr, bt_device_manager_db_remote_version_info_t *info);
bt_status_t bt_device_manager_remote_find_profile_info(bt_bd_addr_t addr, bt_device_manager_db_remote_profile_info_t *info);
bt_status_t bt_device_manager_remote_update_profile_info(bt_bd_addr_t addr, bt_device_manager_db_remote_profile_info_t *info);
void bt_device_manager_remote_info_init(void);

void bt_device_manager_aws_local_info_store_fixed_address(bt_bd_addr_t *addr);
bt_bd_addr_t* bt_device_manager_aws_local_info_get_fixed_address(void);
void bt_device_manager_aws_local_info_store_local_address(bt_bd_addr_t *addr);
bt_bd_addr_t* bt_device_manager_aws_local_info_get_local_address(void);
void bt_device_manager_aws_local_info_store_peer_address(bt_bd_addr_t *addr);
bt_bd_addr_t* bt_device_manager_aws_local_info_get_peer_address(void);
void bt_device_manager_aws_local_info_store_key(bt_key_t* aws_key);
bt_key_t* bt_device_manager_aws_local_info_get_key(void);
void bt_device_manager_aws_local_info_store_role(bt_aws_mce_role_t aws_role);
bt_aws_mce_role_t bt_device_manager_aws_local_info_get_role(void);
void bt_device_manager_aws_local_info_store_ls_enable(uint8_t ls_enable);
uint8_t bt_device_manager_aws_local_info_get_ls_enable();
void bt_device_manager_aws_local_info_init();

void bt_device_manager_store_local_address(bt_bd_addr_t *addr);
bt_bd_addr_t* bt_device_manager_get_local_address(void);

#ifdef __cplusplus
}
#endif

#endif /* __BT_DEVICE_MANAGER_INTERNAL_H__ */

