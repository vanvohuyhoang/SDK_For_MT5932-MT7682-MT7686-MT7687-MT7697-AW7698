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

#ifndef __BT_DEVICE_MANAGER_DB_H__
#define __BT_DEVICE_MANAGER_DB_H__

#include "bt_type.h"
#include "bt_device_manager_internal.h"

#ifdef __cplusplus
extern "C"
{
#endif /* __cplusplus */

#ifndef PACKED
#define PACKED  __attribute__((packed))
#endif

#define BT_DEVICE_MANAGER_REMOTE_INFO_MASK_PAIRED   0x01
#define BT_DEVICE_MANAGER_REMOTE_INFO_MASK_VERSION  0x02
#define BT_DEVICE_MANAGER_REMOTE_INFO_MASK_PROFILE  0x04
typedef uint8_t bt_device_manager_remote_info_mask_t;

typedef struct {
    bt_bd_addr_t local_address;
} bt_device_manager_db_local_info_t;

typedef struct {
    uint8_t sequence_num;
    bt_bd_addr_t address;
    bt_device_manager_remote_info_mask_t info_valid_flag;
    bt_device_manager_db_remote_paired_info_t paired_info;
    bt_device_manager_db_remote_version_info_t version_info;
    bt_device_manager_db_remote_profile_info_t profile_info;
} bt_device_manager_db_remote_info_t;

typedef struct {
    uint8_t         aws_role;
    uint16_t        supported_feature;
    bt_bd_addr_t    peer_addr;
    uint8_t         slave_retry_count;
    bt_bd_addr_t    local_aws_addr;
    uint8_t         ls_enable;
    uint8_t         aws_key[16];
    uint8_t         reserved[15];
} PACKED bt_device_manager_db_aws_local_info_t;

#define BT_DEVICE_MANAGER_DB_TYPE_LOCAL_INFO        0x00
#define BT_DEVICE_MANAGER_DB_TYPE_REMOTE_INFO       0x01
#define BT_DEVICE_MANAGER_DB_TYPE_AWS_LOCAL_INFO    0x02
typedef uint8_t bt_device_manager_db_type_t;

void bt_device_manager_db_flush(bt_device_manager_db_type_t db_type);
void bt_device_manager_db_flush_all();
void bt_device_manager_db_open(bt_device_manager_db_type_t db_type);
void bt_device_manager_db_close(bt_device_manager_db_type_t db_type);
void *bt_device_manager_db_read_data(bt_device_manager_db_type_t db_type);
void bt_device_manager_db_update_data(bt_device_manager_db_type_t db_type);//Todo
void bt_device_manager_db_init();

#ifdef __cplusplus
}
#endif

#endif /* __BT_DEVICE_MANAGER_DB_H__ */

