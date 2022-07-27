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

 
#include "bt_type.h"
#include "bt_aws_mce.h"
#include "bt_device_manager_internal.h"
#include "bt_device_manager_db.h"

#ifndef PACKED
#define PACKED  __attribute__((packed))
#endif

static bt_device_manager_db_aws_local_info_t *bt_dm_aws_cnt = NULL;

void bt_device_manager_aws_local_info_store_fixed_address(bt_bd_addr_t *addr)
{
    bt_dmgr_report_id("[BT_DM][AWS_LOCAL_INFO][I] Store fixed address:", 0);
    bt_device_manager_store_local_address(addr);
}

bt_bd_addr_t* bt_device_manager_aws_local_info_get_fixed_address(void)
{
    bt_dmgr_report_id("[BT_DM][AWS_LOCAL_INFO][I] Get fixed address:", 0);
    return bt_device_manager_get_local_address();
}

void bt_device_manager_aws_local_info_store_local_address(bt_bd_addr_t *addr)
{
#ifdef MTK_AWS_MCE_ENABLE
    bt_dmgr_report_id("[BT_DM][AWS_LOCAL_INFO][I] Store local address:", 0);
    bt_device_manager_dump_bt_address(
        BT_DEVICE_MANAGER_LOCAL_INFO_ADDRESS_AWS_LOCAL, (uint8_t *)addr);
    if (memcmp(&bt_dm_aws_cnt->local_aws_addr, addr, sizeof(bt_bd_addr_t))) {
        memcpy(&bt_dm_aws_cnt->local_aws_addr, addr, sizeof(bt_bd_addr_t));
        bt_device_manager_db_update_data(BT_DEVICE_MANAGER_DB_TYPE_AWS_LOCAL_INFO);
    }
#else
    bt_device_manager_store_local_address(addr);
#endif
}

bt_bd_addr_t* bt_device_manager_aws_local_info_get_local_address(void)
{
#ifdef MTK_AWS_MCE_ENABLE
    bt_dmgr_report_id("[BT_DM][AWS_LOCAL_INFO][I] Get local address:", 0);
    bt_device_manager_dump_bt_address(
        BT_DEVICE_MANAGER_LOCAL_INFO_ADDRESS_AWS_LOCAL, (uint8_t *)&bt_dm_aws_cnt->local_aws_addr);
    return &bt_dm_aws_cnt->local_aws_addr;
#else
    return bt_device_manager_get_local_address();
#endif
}

void bt_device_manager_aws_local_info_store_peer_address(bt_bd_addr_t *addr)
{
    bt_dmgr_report_id("[BT_DM][AWS_LOCAL_INFO][I] Store peer address:", 0);
    bt_device_manager_dump_bt_address(
        BT_DEVICE_MANAGER_LOCAL_INFO_ADDRESS_AWS_PEER, (uint8_t *)addr);
    if (memcmp(&bt_dm_aws_cnt->peer_addr, addr, sizeof(bt_bd_addr_t))) {
        memcpy(&bt_dm_aws_cnt->peer_addr, addr, sizeof(bt_bd_addr_t));
        bt_device_manager_db_update_data(BT_DEVICE_MANAGER_DB_TYPE_AWS_LOCAL_INFO);
    }
}

bt_bd_addr_t* bt_device_manager_aws_local_info_get_peer_address(void)
{
    bt_dmgr_report_id("[BT_DM][AWS_LOCAL_INFO][I] Get peer address:", 0 );
    bt_device_manager_dump_bt_address(
        BT_DEVICE_MANAGER_LOCAL_INFO_ADDRESS_AWS_PEER, (uint8_t *)&bt_dm_aws_cnt->peer_addr);
    return &bt_dm_aws_cnt->peer_addr;
}

void bt_device_manager_aws_local_info_store_key(bt_key_t* aws_key)
{
    bt_dmgr_report_id("[BT_DM][AWS_LOCAL_INFO][I] Store aws key:", 0 );
    bt_device_manager_dump_link_key((uint8_t *)aws_key);
    if (memcmp(&bt_dm_aws_cnt->aws_key, aws_key, sizeof(bt_key_t))) {
        memcpy(&bt_dm_aws_cnt->aws_key, aws_key, sizeof(bt_key_t));
        bt_device_manager_db_update_data(BT_DEVICE_MANAGER_DB_TYPE_AWS_LOCAL_INFO);
    }
}

bt_key_t* bt_device_manager_aws_local_info_get_key(void)
{
    bt_dmgr_report_id("[BT_DM][AWS_LOCAL_INFO][I] Get aws key:", 0 );
    bt_device_manager_dump_link_key((uint8_t *)&bt_dm_aws_cnt->aws_key);
    return &bt_dm_aws_cnt->aws_key;
}

void bt_device_manager_aws_local_info_store_role(bt_aws_mce_role_t aws_role)
{
    bt_dmgr_report_id("[BT_DM][AWS_LOCAL_INFO][I] Store aws role:0x%x", 1, bt_dm_aws_cnt->aws_role);
    if (bt_dm_aws_cnt->aws_role != aws_role) {
        bt_dm_aws_cnt->aws_role = aws_role;
        bt_device_manager_db_update_data(BT_DEVICE_MANAGER_DB_TYPE_AWS_LOCAL_INFO);
    }
}

bt_aws_mce_role_t bt_device_manager_aws_local_info_get_role(void)
{
    bt_dmgr_report_id("[BT_DM][AWS_LOCAL_INFO][I] Get aws role:0x%x", 1, bt_dm_aws_cnt->aws_role);
    return bt_dm_aws_cnt->aws_role;
}

void bt_device_manager_aws_local_info_store_ls_enable(uint8_t ls_enable)
{
    bt_dmgr_report_id("[BT_DM][AWS_LOCAL_INFO][I] Store ls enable:0x%x", 1, ls_enable);
    if (bt_dm_aws_cnt->ls_enable != ls_enable) {
        bt_dm_aws_cnt->ls_enable = ls_enable;
        bt_device_manager_db_update_data(BT_DEVICE_MANAGER_DB_TYPE_AWS_LOCAL_INFO);
    }
}

uint8_t bt_device_manager_aws_local_info_get_ls_enable()
{
    bt_dmgr_report_id("[BT_DM][AWS_LOCAL_INFO][I] Get ls enable:0x%x", 1, bt_dm_aws_cnt->ls_enable);
    return bt_dm_aws_cnt->ls_enable;
}

void bt_device_manager_aws_local_info_init()
{
    bt_dmgr_report_id("[BT_DM][AWS_LOCAL_INFO][I] AWS local info init", 0);
    bt_device_manager_db_open(BT_DEVICE_MANAGER_DB_TYPE_AWS_LOCAL_INFO);
    bt_dm_aws_cnt = (bt_device_manager_db_aws_local_info_t *)bt_device_manager_db_read_data(BT_DEVICE_MANAGER_DB_TYPE_AWS_LOCAL_INFO);
    if (NULL == bt_dm_aws_cnt) {
        bt_device_manager_assert(0 && "read aws local info fail");
    }
}

