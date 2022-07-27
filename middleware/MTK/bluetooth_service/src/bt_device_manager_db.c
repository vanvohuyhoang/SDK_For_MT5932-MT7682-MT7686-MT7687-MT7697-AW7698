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

#include "bt_device_manager_config.h"
#include "bt_device_manager_internal.h"
#include "bt_device_manager_db.h"
#include "bt_device_manager_custom_db.h"

typedef struct {
    bt_device_manager_db_local_info_t local_info;
    bt_device_manager_db_remote_info_t remote_info[BT_DEVICE_MANAGER_MAX_PAIR_NUM];
} bt_device_manager_db_local_all_t;

#define BT_DEVICE_MANAGER_DB_FLAG_INIT      0x01
#define BT_DEVICE_MANAGER_DB_FLAG_FLUSHED   0x02
typedef uint8_t bt_device_manager_db_flag_t;

typedef struct {
    bt_device_manager_db_flag_t flag;
    bt_device_manager_custom_db_item_t type;
    uint32_t buffer_size;
    uint8_t *buffer;
} bt_device_manager_db_cnt_t;

static bt_device_manager_db_local_all_t local_all_db;
static bt_device_manager_db_aws_local_info_t aws_local_db;
static bt_device_manager_db_cnt_t dev_db_cnt[BT_DEVICE_MANAGER_CUSTOM_DB_ITEM_MAX];

void bt_device_manager_db_init()
{
    bt_device_manager_custom_db_item_t db_item = 0;
    bt_dmgr_report_id("[BT_DM][DB][I] DB init", 0);
    memset(&local_all_db, 0, sizeof(local_all_db));
    memset(&aws_local_db, 0, sizeof(aws_local_db));
    memset(&dev_db_cnt, 0, sizeof(dev_db_cnt));
    while (db_item < BT_DEVICE_MANAGER_CUSTOM_DB_ITEM_MAX) {
        bool ret = true;
        if (BT_DEVICE_MANAGER_CUSTOM_DB_ITEM_LOCAL_ALL == db_item) {
            dev_db_cnt[db_item].buffer_size = sizeof(bt_device_manager_db_local_all_t);
            dev_db_cnt[db_item].buffer = (uint8_t *)&local_all_db;
        } else if (BT_DEVICE_MANAGER_CUSTOM_DB_ITEM_AWS_LOCAL == db_item) {
            dev_db_cnt[db_item].buffer_size = sizeof(bt_device_manager_db_aws_local_info_t);
            dev_db_cnt[db_item].buffer = (uint8_t *)&aws_local_db;
        }
        dev_db_cnt[db_item].flag = BT_DEVICE_MANAGER_DB_FLAG_INIT | BT_DEVICE_MANAGER_DB_FLAG_FLUSHED;
        dev_db_cnt[db_item].type = db_item;
        ret = bt_device_manager_custom_db_read(db_item, dev_db_cnt[db_item].buffer, &(dev_db_cnt[db_item].buffer_size));
        bt_device_manager_assert(ret == true && "Init local info db fail");
        db_item++;
    }
}

void bt_device_manager_db_flush(bt_device_manager_db_type_t db_type)
{
    bt_device_manager_custom_db_item_t db_item = 0;
    bt_dmgr_report_id("[BT_DM][DB][I] DB flush db type %d", 1, db_type);
    
    if (BT_DEVICE_MANAGER_DB_TYPE_LOCAL_INFO == db_type || BT_DEVICE_MANAGER_DB_TYPE_REMOTE_INFO == db_type) {
        db_item = BT_DEVICE_MANAGER_CUSTOM_DB_ITEM_LOCAL_ALL;
    } else if (BT_DEVICE_MANAGER_DB_TYPE_AWS_LOCAL_INFO == db_type) {
        db_item = BT_DEVICE_MANAGER_CUSTOM_DB_ITEM_AWS_LOCAL;
    } else {
        bt_dmgr_report_id("[BT_DM][DB][E] DB flush db type %d not find", 1, db_type);
        return;
    }
    if (!(dev_db_cnt[db_item].flag & BT_DEVICE_MANAGER_DB_FLAG_INIT)) {
        bt_dmgr_report_id("[BT_DM][DB][E] DB not opened", 0);
        return;
    }
    if (!(dev_db_cnt[db_item].flag & BT_DEVICE_MANAGER_DB_FLAG_FLUSHED)) {
        bool ret = false;
        ret = bt_device_manager_custom_db_write(db_item, dev_db_cnt[db_item].buffer, dev_db_cnt[db_item].buffer_size);
        bt_device_manager_assert(ret == true && " flush db fail");
        dev_db_cnt[db_item].flag |= BT_DEVICE_MANAGER_DB_FLAG_FLUSHED;
    }
}

void bt_device_manager_db_flush_all()
{
    bt_device_manager_custom_db_item_t db_item = 0;
    bt_dmgr_report_id("[BT_DM][DB][I] DB flush all", 0);
    while (db_item < BT_DEVICE_MANAGER_CUSTOM_DB_ITEM_MAX) {
        if ((dev_db_cnt[db_item].flag & BT_DEVICE_MANAGER_DB_FLAG_INIT) && !(dev_db_cnt[db_item].flag & BT_DEVICE_MANAGER_DB_FLAG_FLUSHED)) {
            bool ret = false;
            ret = bt_device_manager_custom_db_write(db_item, dev_db_cnt[db_item].buffer, dev_db_cnt[db_item].buffer_size);
            bt_device_manager_assert(ret == true && " flush db fail");
            dev_db_cnt[db_item].flag |= BT_DEVICE_MANAGER_DB_FLAG_FLUSHED;
        }
        db_item++;
    }
}

void bt_device_manager_db_open(bt_device_manager_db_type_t db_type)
{
    bt_device_manager_custom_db_item_t db_item = 0;
    bt_dmgr_report_id("[BT_DM][DB][I] DB open db type %d", 1, db_type);
    
    if (BT_DEVICE_MANAGER_DB_TYPE_LOCAL_INFO == db_type || BT_DEVICE_MANAGER_DB_TYPE_REMOTE_INFO == db_type) {
        db_item = BT_DEVICE_MANAGER_CUSTOM_DB_ITEM_LOCAL_ALL;
    } else if (BT_DEVICE_MANAGER_DB_TYPE_AWS_LOCAL_INFO == db_type) {
        db_item = BT_DEVICE_MANAGER_CUSTOM_DB_ITEM_AWS_LOCAL;
    } else {
        bt_dmgr_report_id("[BT_DM][DB][E] DB open db type %d not find", 1, db_type);
        return;
    }
    if (!(dev_db_cnt[db_item].flag & BT_DEVICE_MANAGER_DB_FLAG_INIT)) {
        bool ret = false;
        ret = bt_device_manager_custom_db_read(db_item, dev_db_cnt[db_item].buffer, &(dev_db_cnt[db_item].buffer_size));
        bt_device_manager_assert(ret == true && "Open db fail");
        dev_db_cnt[db_item].flag |= BT_DEVICE_MANAGER_DB_FLAG_INIT;
    }
}

void bt_device_manager_db_close(bt_device_manager_db_type_t db_type)
{
    bt_device_manager_custom_db_item_t db_item = 0;
    bt_dmgr_report_id("[BT_DM][DB][I] DB close db type %d", 1, db_type);
    bt_device_manager_db_flush(db_type);
    if (BT_DEVICE_MANAGER_DB_TYPE_LOCAL_INFO == db_type || BT_DEVICE_MANAGER_DB_TYPE_REMOTE_INFO == db_type) {
        db_item = BT_DEVICE_MANAGER_CUSTOM_DB_ITEM_LOCAL_ALL;
    } else if (BT_DEVICE_MANAGER_DB_TYPE_AWS_LOCAL_INFO == db_type) {
        db_item = BT_DEVICE_MANAGER_CUSTOM_DB_ITEM_AWS_LOCAL;
    } else {
        bt_dmgr_report_id("[BT_DM][DB][E] DB close db type %d not find", 1, db_type);
        return;
    }
    dev_db_cnt[db_item].flag &= (~BT_DEVICE_MANAGER_DB_FLAG_INIT);
}

void *bt_device_manager_db_read_data(bt_device_manager_db_type_t db_type)
{
    bt_device_manager_custom_db_item_t db_item = 0;
    void *ret_data = NULL;
    bt_dmgr_report_id("[BT_DM][DB][I] Get db data type %d", 1, db_type);
    if (BT_DEVICE_MANAGER_DB_TYPE_LOCAL_INFO == db_type) {
        db_item = BT_DEVICE_MANAGER_CUSTOM_DB_ITEM_LOCAL_ALL;
        ret_data = (void *)&(local_all_db.local_info);
    } else if (BT_DEVICE_MANAGER_DB_TYPE_REMOTE_INFO == db_type) {
        db_item = BT_DEVICE_MANAGER_CUSTOM_DB_ITEM_LOCAL_ALL;
        ret_data = (void *)&(local_all_db.remote_info[0]);
    } else if (BT_DEVICE_MANAGER_DB_TYPE_AWS_LOCAL_INFO) {
        db_item = BT_DEVICE_MANAGER_CUSTOM_DB_ITEM_AWS_LOCAL;
        ret_data = (void *)&(aws_local_db);
    } else {
        bt_dmgr_report_id("[BT_DM][DB][E] Get db data type %d not defined", 1, db_type);
    }
    if (!(dev_db_cnt[db_item].flag & BT_DEVICE_MANAGER_DB_FLAG_INIT)) {
        bt_dmgr_report_id("[BT_DM][DB][E] DB not opened", 0);
        return NULL;
    }
    return ret_data;;
}

void bt_device_manager_db_update_data(bt_device_manager_db_type_t db_type)
{
    bt_device_manager_custom_db_item_t db_item = 0;
    bt_dmgr_report_id("[BT_DM][DB][I] Update db data type %d", 1, db_type);
    if (BT_DEVICE_MANAGER_DB_TYPE_LOCAL_INFO == db_type || BT_DEVICE_MANAGER_DB_TYPE_REMOTE_INFO == db_type) {
        db_item = BT_DEVICE_MANAGER_CUSTOM_DB_ITEM_LOCAL_ALL;
    } else if (BT_DEVICE_MANAGER_DB_TYPE_AWS_LOCAL_INFO == db_type) {
        db_item = BT_DEVICE_MANAGER_CUSTOM_DB_ITEM_AWS_LOCAL;
    } else {
        bt_dmgr_report_id("[BT_DM][DB][E] Update db data type %d not find", 1, db_type);
        return;
    }
    if (!(dev_db_cnt[db_item].flag & BT_DEVICE_MANAGER_DB_FLAG_INIT)) {
        bt_dmgr_report_id("[BT_DM][DB][E] DB not opened", 0);
        return;
    }
    dev_db_cnt[db_item].flag &= (~BT_DEVICE_MANAGER_DB_FLAG_FLUSHED);
}

