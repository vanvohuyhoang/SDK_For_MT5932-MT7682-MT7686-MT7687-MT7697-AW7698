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


#include "bt_timer_external.h"

#ifdef __BT_TIMER_EXT_SUPPORT__
#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"
#include "portmacro.h"
#include "queue.h"
#include <timers.h>
#include <string.h>
#include "syslog.h"
#include "bt_platform.h"


log_create_module(BT_TIMER_EXT, PRINT_LEVEL_INFO);

static uint32_t bt_sink_srv_events = 0;

uint32_t bt_timer_ext_semaphore = 0;

static bt_timer_ext_t bt_timer_ext_array[BT_TIMER_EXT_INSTANCE_NUM];
static TimerHandle_t bt_sink_srv_os_timer = NULL; /**< Timer handler. */
const bt_timer_ext_t* bt_sink_srv_time_running;

#if defined(MTK_AWS_MCE_ENABLE) && defined (SUPPORT_ROLE_HANDOVER_SERVICE)
#include "bt_role_handover.h"

bt_timer_ext_t *bt_timer_ext_rho = (bt_timer_ext_t *)(&bt_timer_ext_array); 
extern bt_role_handover_callbacks_t bt_timer_ext_rho_callbacks;
#endif

uint32_t bt_timer_ext_get_current_tick(void);
static void bt_timer_ext_expire(TimerHandle_t timer);
static bool bt_timer_ext_os_is_active(void);
void bt_timer_ext_os_start(uint32_t tick);
void bt_timer_ext_os_stop(void);
const bt_timer_ext_t* bt_timer_ext_find_recent(uint32_t current);

static bool g_init_flag = false;      /**< This variable Indicates if the module is initialized or not. */

void bt_timer_ext_init(void)
{
    if (g_init_flag) {
        LOG_MSGID_I(BT_TIMER_EXT, "bt_timer_ext_init(), it has been initialized by others!\r\n", 0);
        return ;
    }
    if (NULL == bt_sink_srv_os_timer) {
        bt_sink_srv_os_timer = xTimerCreate( "bt timer ext", 0xffff, pdFALSE, NULL, bt_timer_ext_expire);
        bt_timer_ext_semaphore = (uint32_t)xSemaphoreCreateBinary();
        if (bt_timer_ext_semaphore) {
            xSemaphoreGive((SemaphoreHandle_t)bt_timer_ext_semaphore);
        }
    }
    memset(bt_timer_ext_array, 0, sizeof(bt_timer_ext_array));
    g_init_flag = true;

#if defined(MTK_AWS_MCE_ENABLE) && defined (SUPPORT_ROLE_HANDOVER_SERVICE)
    bt_status_t result = bt_role_handover_register_callbacks(BT_ROLE_HANDOVER_MODULE_EXTERNAL_TIMER, &bt_timer_ext_rho_callbacks);
    LOG_MSGID_I(BT_TIMER_EXT, "RHO register result: 0x%4x\r\n", 1, result);
#endif
}

/**
static void bt_timer_ext_deinit(void)
{
    LOG_MSGID_I(BT_TIMER_EXT, "[sink][timer] bt_timer_ext_deinit()\r\n", 0);
    if (bt_sink_srv_os_timer != NULL) {
        bt_timer_ext_os_stop();
        xTimerDelete(bt_sink_srv_os_timer, 0);
        bt_sink_srv_os_timer = NULL;
    }
    memset(bt_timer_ext_array, 0, sizeof(bt_timer_ext_array));
}
*/

bt_timer_ext_status_t bt_timer_ext_start(uint32_t timer_id, uint32_t data, uint32_t time_ms, bt_timer_ext_timeout_callback_t cb)
{
    uint32_t current;
    uint32_t length;
    int32_t diff, is_timeout;
    bt_timer_ext_t *bt_timer_ext;

    if (false == g_init_flag) {
        LOG_MSGID_I(BT_TIMER_EXT, "please call bt_timer_ext_init() to init the ext timer module firstly!\r\n", 0);
        return BT_TIMER_EXT_STATUS_FAIL;
    }

    xSemaphoreTake((SemaphoreHandle_t)bt_timer_ext_semaphore, portMAX_DELAY);
    if (NULL != bt_timer_ext_find(timer_id)) {
        LOG_MSGID_I(BT_TIMER_EXT, "bt_timer_ext_start(%x)fail, timer is existed!\r\n", 1, timer_id);
        xSemaphoreGive((SemaphoreHandle_t)bt_timer_ext_semaphore);
        return BT_TIMER_EXT_STATUS_FAIL;
    }

    LOG_MSGID_I(BT_TIMER_EXT, "bt_timer_ext_start, id 0x%8x, data 0x%8x, duration 0x%8x, cb 0x%8x\r\n", 4, timer_id, data, time_ms, cb);
    //bt_timer_ext_check_status(0);
    bt_timer_ext = bt_timer_ext_find(0);

    if (NULL == bt_timer_ext) {
        LOG_MSGID_I(BT_TIMER_EXT, "bt_timer_ext_start fail, OOM!\r\n", 0);
        xSemaphoreGive((SemaphoreHandle_t)bt_timer_ext_semaphore);
        return BT_TIMER_EXT_STATUS_OOM;
    }

    current = bt_timer_ext_get_current_tick();
    length = time_ms/portTICK_PERIOD_MS + 1;
    memset(bt_timer_ext, 0x0, sizeof(bt_timer_ext_t));
    bt_timer_ext->timer_id = timer_id;
    bt_timer_ext->data = data;
    bt_timer_ext->time_tick = current + length;
    bt_timer_ext->cb = cb;

    if ((bt_sink_srv_time_running != NULL) && (bt_sink_srv_time_running->timer_id != 0)) {
        is_timeout = bt_sink_srv_time_running->time_tick - current;
        diff = is_timeout - length;
        if ((is_timeout < 0) || (diff < 0)) //is_timeout <= 0, means timeout; diff < 0, means bt_timer_ext_array[i] is before bt_timer_ext.
        {
            bt_timer_ext = NULL;
        }
    }

    if (bt_timer_ext) {
        bt_timer_ext_os_stop();
        bt_timer_ext_os_start(length);
        bt_sink_srv_time_running = bt_timer_ext; 
        LOG_MSGID_I(BT_TIMER_EXT, "bt_timer_ext_start success, timer duration %d!\r\n", 1, length);
    } 
    xSemaphoreGive((SemaphoreHandle_t)bt_timer_ext_semaphore);
    return BT_TIMER_EXT_STATUS_SUCCESS;
}

bt_timer_ext_status_t bt_timer_ext_stop(uint32_t timer_id)
{
    uint32_t current = bt_timer_ext_get_current_tick();
    xSemaphoreTake((SemaphoreHandle_t)bt_timer_ext_semaphore, portMAX_DELAY);
    bt_timer_ext_t* temp = bt_timer_ext_find(timer_id);
    if (temp == NULL) {
        LOG_MSGID_I(BT_TIMER_EXT, "bt_timer_ext_stop(%x) fail, timer isn't existed!\r\n", 1, timer_id);
        xSemaphoreGive((SemaphoreHandle_t)bt_timer_ext_semaphore);
        return BT_TIMER_EXT_STATUS_FAIL;
    }
    //bt_timer_ext_check_status(0);
    LOG_MSGID_I(BT_TIMER_EXT, "bt_timer_ext_stop, Timer id is 0x%4x", 1, timer_id);
    temp->timer_id = 0;
    if (temp == bt_sink_srv_time_running) {
        bt_timer_ext_os_stop();
        bt_sink_srv_time_running = bt_timer_ext_find_recent(current);
        if (bt_sink_srv_time_running != NULL) {
            bt_timer_ext_os_start(bt_sink_srv_time_running->time_tick - current);
        }
    }
    xSemaphoreGive((SemaphoreHandle_t)bt_timer_ext_semaphore);
    return BT_TIMER_EXT_STATUS_SUCCESS;
}

static void bt_timer_ext_check_status(void)
{
    uint32_t current = 0;
    int32_t diff = 0;
    uint32_t i;
    bt_timer_ext_t expire_timer[BT_TIMER_EXT_INSTANCE_NUM] = {0};
    //xSemaphoreTake((SemaphoreHandle_t)bt_timer_ext_semaphore, portMAX_DELAY);
    
    LOG_MSGID_I(BT_TIMER_EXT, "bt_timer_ext_check_status!\r\n", 0);
    current = bt_timer_ext_get_current_tick();
    for (i = 0; i < BT_TIMER_EXT_INSTANCE_NUM; i++) {
        if (bt_timer_ext_array[i].timer_id != 0) {
            diff = (int32_t)(bt_timer_ext_array[i].time_tick - current);
            if (diff <= 0) {
                expire_timer[i].timer_id = bt_timer_ext_array[i].timer_id;
                expire_timer[i].data = bt_timer_ext_array[i].data;
                expire_timer[i].cb = bt_timer_ext_array[i].cb;
                bt_timer_ext_array[i].timer_id = 0;
                LOG_MSGID_I(BT_TIMER_EXT, "BT ext timer id(%x), expired!\r\n", 1, expire_timer[i].timer_id);
            }
        }
    }
    if ((bt_sink_srv_time_running) && (bt_sink_srv_time_running->timer_id == 0)) {
        bt_sink_srv_time_running = NULL;
        current = bt_timer_ext_get_current_tick();
        bt_sink_srv_time_running = bt_timer_ext_find_recent(current);
        if (bt_sink_srv_time_running) {
            bt_timer_ext_os_start(bt_sink_srv_time_running->time_tick - current);
        }
    }
    //xSemaphoreGive((SemaphoreHandle_t)bt_timer_ext_semaphore);

    for (i = 0; i < BT_TIMER_EXT_INSTANCE_NUM; i++) {
        if (NULL != expire_timer[i].cb) {
            expire_timer[i].cb(expire_timer[i].timer_id, expire_timer[i].data);
        }
        memset(&expire_timer[i], 0x0, sizeof(bt_timer_ext_t));
    }
}

void bt_timer_ext_handle_interrupt(void)
{
    uint32_t current_events;
    taskDISABLE_INTERRUPTS();
    current_events = bt_sink_srv_events;
    bt_sink_srv_events = 0;
    taskENABLE_INTERRUPTS();

    if (current_events & BT_TIMER_EXT_EXPIRED) {
        bt_timer_ext_check_status();
    }
    return;
}

bt_timer_ext_t* bt_timer_ext_find(uint32_t timer_id)
{
    bt_timer_ext_t* timer = NULL;
    uint32_t i;
    for (i = 0; i < BT_TIMER_EXT_INSTANCE_NUM; i++) {
        if (bt_timer_ext_array[i].timer_id == timer_id)
        {
            timer = (bt_timer_ext_t*)(bt_timer_ext_array + i);
            break;
        }
    }
    return timer;
}

const bt_timer_ext_t* bt_timer_ext_find_recent(uint32_t current)
{
    const bt_timer_ext_t* timer = NULL;
    int32_t diff, diff2;
    uint32_t i;
    for (i = 0; i < BT_TIMER_EXT_INSTANCE_NUM; i++) {
        if (bt_timer_ext_array[i].timer_id != 0)
        {
            diff = (int32_t)(bt_timer_ext_array[i].time_tick - current);
            if (diff <= 0) {
                continue;
            }
            else if (timer == NULL) {
                timer = bt_timer_ext_array + i;
            }
            else {
                diff2 = (int32_t)(timer->time_tick - current);
                if (diff < diff2)
                {
                    timer = bt_timer_ext_array + i;
                }

            }
        }
    }
    return timer;
}

uint32_t bt_timer_ext_get_current_tick(void)
{
    return xTaskGetTickCount();
}

static void bt_timer_ext_expire(TimerHandle_t timer)
{
    taskDISABLE_INTERRUPTS();
    bt_sink_srv_events |= BT_TIMER_EXT_EXPIRED;
    taskENABLE_INTERRUPTS();
    bt_trigger_interrupt(1);
}
    
static bool bt_timer_ext_os_is_active(void)
{
    if ((bt_sink_srv_os_timer != NULL) && (xTimerIsTimerActive(bt_sink_srv_os_timer) != pdFALSE)) {
        return true;
    }
    else {
        return false;
    }
}


void bt_timer_ext_os_start(uint32_t tick)
{
    if (bt_sink_srv_os_timer == NULL) { 
        LOG_MSGID_I(BT_TIMER_EXT, "bt_timer_ext_os_start fail!\r\n", 0);
        return;
    }
    if (bt_timer_ext_os_is_active()) {
        bt_timer_ext_os_stop();
    }    
    if (xTimerChangePeriod(bt_sink_srv_os_timer, tick, 0) == pdPASS) {
        int32_t ret = xTimerReset(bt_sink_srv_os_timer, 0);
        LOG_MSGID_I(BT_TIMER_EXT, "bt_timer_ext_os_start result 0x%x!\r\n", 1, ret);
    }
}

void bt_timer_ext_os_stop(void)
{
    if ((bt_sink_srv_os_timer != NULL) && (bt_timer_ext_os_is_active())) {
        xTimerStop(bt_sink_srv_os_timer, 0);
        LOG_MSGID_I(BT_TIMER_EXT, "bt_timer_ext_os_stop!\r\n", 0);
    }
}
#endif

