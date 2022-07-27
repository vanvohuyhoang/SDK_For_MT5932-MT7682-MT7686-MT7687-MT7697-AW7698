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

#include "bwcs_bt_interface.h"
#include "wifi_os_api.h"
#include "bt_type.h"
#include "bt_callback_manager.h"
#include "bt_gap_le.h"
#include "bt_driver_7698.h"

#define MAX_BWCS_BT_CONN_NUM 8

#define BWCS_BT_PRIVILEGE_PRIORITY 0x07F0
#define BWCS_BT_NORMAL_PRIORITY 0x7FFF

#define BWCS_BT_MAX_OFFSET_IN_TIME 80

#define BWCS_BT_RATIO_PTA 60
#define BWCS_BT_RATIO_PRIVILEDGE 20
#define BWCS_BT_RATIO_NORMAL 0



log_create_module(BWCS_BT, PRINT_LEVEL_INFO);

#define BWCS_BT_LOG_I(format, ...) LOG_I(BWCS_BT, format, ##__VA_ARGS__) 
#define BWCS_BT_LOG_W(format, ...) LOG_W(BWCS_BT, format, ##__VA_ARGS__) 
#define BWCS_BT_LOG_E(format, ...) LOG_E(BWCS_BT, format, ##__VA_ARGS__) 

typedef struct {
    bt_handle_t conn_handle;
    uint16_t conn_interval;
    bool used;
}bwcs_bt_conn_info_t;

typedef struct {
    bwcs_bt_conn_info_t conn[MAX_BWCS_BT_CONN_NUM];
    uint8_t num_of_connected_dev;
    bool is_PTA_enabled;
    bool is_priviledge_enabled;
}bwcs_bt_context_t;


extern QueueHandle_t g_bwcs_queue;

static bwcs_bt_context_t g_bwcs_context;

static bt_status_t bwcs_bt_callback(bt_msg_type_t msg, bt_status_t status, void *buff);

void bwcs_bt_init()
{
    BWCS_BT_LOG_I("Init");

    bt_callback_manager_register_callback(bt_callback_type_app_event, MODULE_MASK_SYSTEM | MODULE_MASK_GAP, bwcs_bt_callback);

}

void bwcs_bt_deinit()
{
    BWCS_BT_LOG_I("Deinit");
    bt_callback_manager_deregister_callback(bt_callback_type_app_event, bwcs_bt_callback);
    memset(&g_bwcs_context, 0, sizeof(g_bwcs_context));
}


static int8_t bwcs_bt_add_connection(bt_gap_le_connection_ind_t *conn)
{
    uint32_t i;
    if (conn == NULL) {
        /* Not handle NULL cases */
        return -1;
    }
    for (i = 0; i < MAX_BWCS_BT_CONN_NUM; i++) {
        if (g_bwcs_context.conn[i].used == false) {

            g_bwcs_context.conn[i].conn_handle = conn->connection_handle;
            g_bwcs_context.conn[i].used = true;
            g_bwcs_context.conn[i].conn_interval = conn->conn_interval;
            break;

        }
    }

    if (i == MAX_BWCS_BT_CONN_NUM) {
        /* Buffer is fool */
        return -2;
    }

    return 0;

}

static int8_t bwcs_bt_remove_connection(bt_handle_t conn_handle)
{
    uint32_t i;
    for (i = 0; i < MAX_BWCS_BT_CONN_NUM; i++) {
        if (g_bwcs_context.conn[i].used == true
            && g_bwcs_context.conn[i].conn_handle == conn_handle) {
            memset(&g_bwcs_context.conn[i], 0, sizeof(bwcs_bt_conn_info_t));
            break;
        }
    }

    if (i == MAX_BWCS_BT_CONN_NUM) {
        /* The conn handle didn't record */
        return -1;
    }

    return 0;

}

static int8_t bwcs_bt_update_connection_info(bt_gap_le_connection_update_ind_t *conn_info)
{
    uint32_t i;
    if (conn_info == NULL) {
        return -1;
    }

    for (i = 0; i < MAX_BWCS_BT_CONN_NUM; i++) {
        if (g_bwcs_context.conn[i].used == true
            && g_bwcs_context.conn[i].conn_handle == conn_info->conn_handle) {

            g_bwcs_context.conn[i].conn_interval = conn_info->conn_interval;
            break;
        }
    }

    if (i == MAX_BWCS_BT_CONN_NUM) {
        /* The conn handle didn't record */
        return -2;
    }

    return 0;

}

static bwcs_bt_conn_info_t * bwcs_bt_get_conn_by_handle(bt_handle_t conn_handle)
{
    uint32_t i;

    for (i = 0; i < MAX_BWCS_BT_CONN_NUM; i++) {
        if (g_bwcs_context.conn[i].used == true
            && g_bwcs_context.conn[i].conn_handle == conn_handle) {

            return &g_bwcs_context.conn[i];

        }
    }
    BWCS_BT_LOG_I("No conn got");

    return NULL;

}

static uint8_t bwcs_bt_conn_end_offset_calculate(uint16_t conn_interval, uint8_t ratio)
{
    uint16_t offset_in_time, offset;

    if (ratio > 100) {
        return 0;
    }

    /* Convert to ms */
    conn_interval = conn_interval * 5 / 4;

    /* offset * 0.3125 ms = offset in timezone */
    offset_in_time  = conn_interval - (conn_interval * ratio / 100);

    if (offset_in_time >= BWCS_BT_MAX_OFFSET_IN_TIME) {

        offset = 0xFF;

    } else {
        offset = offset_in_time * 16 / 5;
    }

    return offset;

}

static int8_t bwcs_bt_update_connection_event_for_one_conn(bwcs_bt_conn_info_t *conn)
{
    uint8_t offset, ratio_type;

    if (conn == NULL) {
        return -1;
    }

    BWCS_BT_LOG_I("update connection event PTA(%d), priviledge(%d)", g_bwcs_context.is_PTA_enabled, g_bwcs_context.is_priviledge_enabled);

    if (g_bwcs_context.is_priviledge_enabled) {

        ratio_type = BWCS_BT_RATIO_PRIVILEDGE;

    } else if (g_bwcs_context.is_PTA_enabled) {

        ratio_type = BWCS_BT_RATIO_PTA;

    } else {

        ratio_type = BWCS_BT_RATIO_NORMAL;
    }

    offset = bwcs_bt_conn_end_offset_calculate(conn->conn_interval, ratio_type);

    if (offset == 0) {
        return -1;
    }

    bt_driver_bwcs_set_conn_end_offset_cmd(offset);

    return 0;


}

static void bwcs_bt_update_connection_event_for_all_conn(void)
{
    uint32_t i;

    for (i = 0; i < MAX_BWCS_BT_CONN_NUM; i++) {
        if (g_bwcs_context.conn[i].used == true) {
            bwcs_bt_update_connection_event_for_one_conn(&g_bwcs_context.conn[i]);
        }
    }

}

static bt_status_t bwcs_bt_callback(bt_msg_type_t msg, bt_status_t status, void *buff)
{

    bwcs_queue_t bt_event = {
        .event_type = BWCS_EVENT_TYPE_BT,
    };

    switch (msg) {
        case BT_POWER_ON_CNF: {

            bt_event.event = BWCS_BT_EVENT_ON;
            bwcs_bt_event_notify(&bt_event);

            bt_event.event = BWCS_BT_EVENT_CH_UPDATE;
            /* Todo */
            //bt_event.payload = 0;
            //bwcs_bt_event_notify(&bt_event);
            break;
        }
        case BT_POWER_OFF_CNF: {

            bt_event.event = BWCS_BT_EVENT_OFF;
            bwcs_bt_event_notify(&bt_event);
            break;

        }
        case BT_GAP_LE_CONNECT_IND: {
            if (status == BT_STATUS_SUCCESS) {

                int8_t ret;

                bt_gap_le_connection_ind_t *ind = (bt_gap_le_connection_ind_t *)buff;

                ret = bwcs_bt_add_connection(ind);

                if (ret < 0) {
                    /* Add failed, don't do next stage */
                    break;
                }

                g_bwcs_context.num_of_connected_dev += 1;

                if (g_bwcs_context.is_PTA_enabled 
                    || g_bwcs_context.is_priviledge_enabled) {

                    ret = bwcs_bt_update_connection_event_for_one_conn(bwcs_bt_get_conn_by_handle(ind->connection_handle));
                }



                if (g_bwcs_context.num_of_connected_dev == 1) {

                    bt_event.event = BWCS_BT_EVENT_CH_UPDATE;
                    /* Todo */
                    //bt_event.payload = 0;
                    //bwcs_bt_event_notify(&bt_event);
                }
            }
            break;
        }
        case BT_GAP_LE_DISCONNECT_IND: {

            if (status == BT_STATUS_SUCCESS) {
                int8_t ret;
                bt_gap_le_disconnect_ind_t *dis_ind = (bt_gap_le_disconnect_ind_t *)buff;

                if (g_bwcs_context.num_of_connected_dev == 0) {
                    /* This means unmatch or memory corruption */
                    assert(0);
                }

                if (dis_ind == NULL) {
                    break;
                }

                ret = bwcs_bt_remove_connection(dis_ind->connection_handle);

                if (ret < 0) {
                    /* Only added connection successful will be added to bwcs bt */
                    break;
                }

                g_bwcs_context.num_of_connected_dev -= 1;

                if (g_bwcs_context.num_of_connected_dev == 0) {

                    bt_event.event = BWCS_BT_EVENT_CH_UPDATE;
                    /* Todo */
                    //bt_event.payload = 0;
                    //bwcs_bt_event_notify(&bt_event);

                }
                
            }
            break;

        }
        case BT_GAP_LE_CONNECTION_UPDATE_IND: {
            int8_t ret;
            bt_gap_le_connection_update_ind_t *conn_info = (bt_gap_le_connection_update_ind_t *)buff;

            if (conn_info == NULL) {
                break;
            }

            ret = bwcs_bt_update_connection_info(conn_info);

            if (g_bwcs_context.is_PTA_enabled 
                || g_bwcs_context.is_priviledge_enabled) {

                ret = bwcs_bt_update_connection_event_for_one_conn(bwcs_bt_get_conn_by_handle(conn_info->conn_handle));
            }
            
            if(ret < 0){

            }

            break;
        }
        default: {
            break;
        }
    }

    return BT_STATUS_SUCCESS;

}

int32_t bwcs_bt_event_notify(bwcs_queue_t *bt_event)
{
    /* Send event to bwcs service */
    if(g_bwcs_queue == NULL) {
        printf("bwcs queue invalid.\r\n");
        return -1;
    }

    if( wifi_os_queue_send(g_bwcs_queue, bt_event, 0) != 0 ) {
        return -1;
    }
    
    return 0;
}

static int8_t bwcs_bt_enable_PTA()
{
    /* Step 1, enable PTA */
    bt_driver_pta_enable_cmd(true);

    g_bwcs_context.is_PTA_enabled  = true;

    /* Step 2, configure CI related parameter */

    bwcs_bt_update_connection_event_for_all_conn();

    return 0;

}

static int8_t bwcs_bt_disable_PTA()
{

    bt_driver_pta_enable_cmd(false);

    g_bwcs_context.is_PTA_enabled  = false;

    bwcs_bt_update_connection_event_for_all_conn();
    return 0;
}

static int8_t bwcs_bt_enter_privilege_mode()
{
    bt_driver_pta_set_priority_cmd(BWCS_BT_PRIVILEGE_PRIORITY);

    g_bwcs_context.is_priviledge_enabled = true;

    bwcs_bt_update_connection_event_for_all_conn();

    return 0;
}

static int8_t bwcs_bt_exit_privilege_mode()
{

    bt_driver_pta_set_priority_cmd(BWCS_BT_NORMAL_PRIORITY);

    g_bwcs_context.is_priviledge_enabled = false;

    bwcs_bt_update_connection_event_for_all_conn();

    return 0;
}

int32_t bwcs_config_bt_cmd(bwcs_bt_cmd_t bt_cmd)
{
    /* Receive command from bwcs service */
    switch (bt_cmd) {
        case BWCS_BT_CMD_ON: {
            /* Open PTA */
            bwcs_bt_enable_PTA();
            break;
        }
        case BWCS_BT_CMD_OFF: {
            /* Close PTA */
            bwcs_bt_disable_PTA();
            break;
        }
        case BWCS_BT_CMD_ENTER_PRIVILEGE: {
            /* Enter privilege mode */
            bwcs_bt_enter_privilege_mode();
            break;
        }
        case BWCS_BT_CMD_EXIT_PRIVILEGE: {
            /* Exit privilege mode */
            bwcs_bt_exit_privilege_mode();
            break;
        }
        default: {
            break;
        }
    }

    return 0;
    //bt config interface
}

