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

#include "bt_debug.h"
#if defined(MTK_AWS_MCE_ENABLE) && defined (SUPPORT_ROLE_HANDOVER_SERVICE)

#include "FreeRTOS.h"
#include "semphr.h"
#include "bt_timer_external.h"
#include "bt_role_handover.h"

extern bt_timer_ext_t *bt_timer_ext_rho;
extern uint32_t bt_timer_ext_semaphore;
extern const bt_timer_ext_t* bt_sink_srv_time_running;
extern uint32_t bt_timer_ext_get_current_tick(void);
extern void bt_timer_ext_os_start(uint32_t tick);
extern void bt_timer_ext_os_stop(void);
extern const bt_timer_ext_t* bt_timer_ext_find_recent(uint32_t current);

static bt_status_t bt_timer_ext_rho_allowed_cb(const bt_bd_addr_t *addr);
static uint8_t bt_timer_ext_rho_get_length_cb(const bt_bd_addr_t *addr);
static bt_status_t bt_timer_ext_rho_get_data_cb(const bt_bd_addr_t *addr, void * data);
static bt_status_t bt_timer_ext_rho_update_cb(bt_role_handover_update_info_t *info);
static void bt_timer_ext_rho_status_cb(const bt_bd_addr_t *addr, bt_aws_mce_role_t role, bt_role_handover_event_t event, bt_status_t status);

bt_role_handover_callbacks_t bt_timer_ext_rho_callbacks = {
    .allowed_cb = bt_timer_ext_rho_allowed_cb,/*optional if always allowed*/
    .get_len_cb = bt_timer_ext_rho_get_length_cb,  /*optional if no RHO data to partner*/
    .get_data_cb = bt_timer_ext_rho_get_data_cb,   /*optional if no RHO data to partner*/
    .update_cb = bt_timer_ext_rho_update_cb,       /*optional if no RHO data to partner*/
    .status_cb = bt_timer_ext_rho_status_cb, /*Mandatory for all users.*/
};

static uint8_t bt_timer_ext_rho_get_list_size(bt_timer_ext_t *timer_list)
{
    uint8_t num = 0;

    if (timer_list) {
        uint8_t i;

        for (i = 0; i < BT_TIMER_EXT_INSTANCE_NUM; i++) {
            if (0 != (bt_timer_ext_t *)(timer_list + i)->timer_id) {
                num++;
            }
        }
    }

    LOG_MSGID_I(BT_TIMER_EXT, "bt_timer_ext_rho_get_list_size, num is %d\r\n", 1, num);
    return num ;
}

static void bt_timer_ext_rho_get_list_data(bt_timer_ext_t *timer_list, void *data)
{
    uint8_t i;
    bt_timer_ext_t *rho_data = (bt_timer_ext_t *)data;

    if (timer_list) {
        for (i = 0; i < BT_TIMER_EXT_INSTANCE_NUM; i++) {
            bt_timer_ext_t *tmp = (bt_timer_ext_t *)(timer_list + i);
            if (0 != tmp->timer_id) {
                uint32_t current = bt_timer_ext_get_current_tick();

                ((bt_timer_ext_t *)(rho_data + i))->timer_id = tmp->timer_id;
                if (((int32_t)(tmp->time_tick - current)) <= 0) {
                    ((bt_timer_ext_t *)(rho_data + i))->time_tick = 0;
                } else {
                    ((bt_timer_ext_t *)(rho_data + i))->time_tick = tmp->time_tick - current;
                }
                ((bt_timer_ext_t *)(rho_data + i))->data = tmp->data;
                ((bt_timer_ext_t *)(rho_data + i))->cb = tmp->cb;

                LOG_MSGID_I(BT_TIMER_EXT, "bt_timer_ext_rho_get_list_data, timer_id 0x%4x, timer_ms 0x%4x\r\n", 2, tmp->timer_id, ((bt_timer_ext_t *)(rho_data + i))->time_tick);
            }
        }
    }
}

#if 0
static bt_status_t bt_timer_ext_rho_set_data(uint32_t timer_id, void *data)
{
    /* find the timer and refresh the data. */
    bt_timer_ext_t* temp = bt_timer_ext_find(timer_id);
    if (NULL == temp) {
        LOG_MSGID_I(BT_TIMER_EXT, "bt_timer_rho_set_data(%x)fail, timer isn't existed!\r\n", 1, timer_id);
        return BT_STATUS_TIMER_NOT_FOUND;
    }

    temp->data = (uint32_t)data;
    LOG_MSGID_I(BT_TIMER_EXT, "bt_timer_rho_set_data done, timer_id is 0x%4x\r\n", 1, timer_id);
    return BT_STATUS_SUCCESS;
}
#endif

static uint8_t bt_timer_ext_rho_get_data_length(void)
{
    uint8_t timer_num = bt_timer_ext_rho_get_list_size(bt_timer_ext_rho);

    LOG_MSGID_I(BT_TIMER_EXT, "bt_timer_ext_rho_get_data_length, length is %d\r\n", 1, (timer_num * sizeof(bt_timer_ext_t)));
    return (timer_num * sizeof(bt_timer_ext_t));
}

/* Stop all timer. */
static void bt_timer_ext_rho_stop_all_timer(void)
{
    uint16_t i;

    bt_timer_ext_os_stop();

    for (i = 0; i < BT_TIMER_EXT_INSTANCE_NUM; i++) {
        if (0 != (bt_timer_ext_t *)(bt_timer_ext_rho + i)->timer_id) {
            memset((bt_timer_ext_rho + i), 0x00, sizeof(bt_timer_ext_t));
        }
    }
    bt_sink_srv_time_running = NULL;

}

static void bt_timer_ext_rho_restart_timer(void)
{
    uint32_t current = 0;
    if ((bt_sink_srv_time_running) && (bt_sink_srv_time_running->timer_id == 0)) {
        bt_sink_srv_time_running = NULL;
    }

    current = bt_timer_ext_get_current_tick();
    bt_sink_srv_time_running = bt_timer_ext_find_recent(current);
    if (bt_sink_srv_time_running) {
        bt_timer_ext_os_start(bt_sink_srv_time_running->time_tick - current);
    }
}

void bt_timer_ext_rho_update_context(bt_aws_mce_role_t role, const void *data, uint32_t length, bt_status_t status)
{
    uint32_t counter = 0;

    switch (role) {
        case BT_AWS_MCE_ROLE_AGENT: {
            LOG_MSGID_I(BT_TIMER_EXT, "bt_timer_ext_rho_update_context, agent status is 0x%4x\r\n", 1, status);
            if (BT_STATUS_SUCCESS == status) {/*success, so stop all timer. */
                bt_timer_ext_rho_stop_all_timer();
            } else {/* fail, re-start the original timer list */
                /* re-start for the first timer */
                if (bt_timer_ext_rho_get_list_size(bt_timer_ext_rho) > 0) {
                    bt_timer_ext_rho_restart_timer();
                    LOG_MSGID_I(BT_TIMER_EXT, "bt_timer_ext_rho_update_context: re-start the first timer\r\n", 0);
                }
            }
        }
        break;

        case BT_AWS_MCE_ROLE_PARTNER: {
            bt_timer_ext_rho_stop_all_timer();
            if ((length > 0) && (data)) {
                bt_timer_ext_t *rho_data = (bt_timer_ext_t *)data;
                while (length) {
                    //bt_timer_ext_t rho_buf = {0};
                    //bt_timer_ext_t *rho_timer = &rho_buf;
                    //memcpy(rho_timer, (bt_timer_ext_t *)(rho_data + counter), sizeof(bt_timer_ext_t));
                    bt_timer_ext_t *rho_timer = (bt_timer_ext_t *)(rho_data + counter);
                    LOG_MSGID_I(BT_TIMER_EXT, "bt_timer_ext_rho_update_context, timer_id is 0x%4x, timer_ms is 0x%4x\r\n", 2, rho_timer->timer_id, rho_timer->time_tick);
                    bt_timer_ext_start(rho_timer->timer_id, rho_timer->data, rho_timer->time_tick, rho_timer->cb);
                    length -= sizeof(bt_timer_ext_t);
                    counter++;
                }
            } else {
                LOG_MSGID_W(BT_TIMER_EXT, "bt_timer_ext_rho_update_context: data is NULL!\r\n", 0);
            }
        }
        break;

        default:
        break;
    }
}

static bt_status_t bt_timer_ext_rho_allowed_cb(const bt_bd_addr_t *addr)
{
    return BT_STATUS_SUCCESS;
}

static uint8_t bt_timer_ext_rho_get_length_cb(const bt_bd_addr_t *addr)
{
    return bt_timer_ext_rho_get_data_length();
}

static bt_status_t bt_timer_ext_rho_get_data_cb(const bt_bd_addr_t *addr, void * data)
{
    assert(data);
    bt_timer_ext_rho_get_list_data(bt_timer_ext_rho, data);
    bt_timer_ext_os_stop();//to stop the os timer

    return BT_STATUS_SUCCESS;
}

static bt_status_t bt_timer_ext_rho_update_cb(bt_role_handover_update_info_t *info)
{
    if (BT_AWS_MCE_ROLE_PARTNER == info->role) {
        bt_timer_ext_rho_update_context(BT_AWS_MCE_ROLE_PARTNER, info->data, (uint32_t)info->length, BT_STATUS_SUCCESS);
    }
    return BT_STATUS_SUCCESS;
}

static void bt_timer_ext_rho_status_cb(const bt_bd_addr_t *addr, bt_aws_mce_role_t role, bt_role_handover_event_t event, bt_status_t status)
{
    LOG_MSGID_I(BT_TIMER_EXT, "bt_timer_ext_rho_status_cb, role is 0x%2x, event is 0x%2x, status is 0x%4x\r\n", 3, role, event, status);

    if (BT_ROLE_HANDOVER_COMPLETE_IND == event) {
        switch (role) {
            case BT_AWS_MCE_ROLE_AGENT: {
                bt_timer_ext_rho_update_context(BT_AWS_MCE_ROLE_AGENT, NULL, 0, status);
                break;
            }
            case BT_AWS_MCE_ROLE_PARTNER: {
                //if (BT_STATUS_SUCCESS == status) {
                    //bt_timer_ext_rho_stop_all_timer();
                //}
                break;
            }
            default :
                break;
        }
    }
}


#endif /* __MTK_AWS_MCE_ENABLE__ */


