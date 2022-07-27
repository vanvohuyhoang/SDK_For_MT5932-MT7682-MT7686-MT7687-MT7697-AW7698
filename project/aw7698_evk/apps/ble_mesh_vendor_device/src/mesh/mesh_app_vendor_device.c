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

#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdbool.h>

#include "bt_type.h"
#include "bt_system.h"

#include "mesh_app_util.h"
#include "bt_mesh.h"
#include "bt_mesh_debug.h"
#include "bt_mesh_flash.h"

#include "bt_mesh_model_configuration_server.h"
#include "bt_mesh_model_health_server.h"
#include "bt_mesh_model_generic_server.h"
#include "bt_mesh_model_generic_opcodes.h"
#include "bt_mesh_model_lighting_server.h"
#include "bt_mesh_model_configuration_server.h"
#include "bt_mesh_model_health_server.h"

#include "mesh_app_vendor_device_def.h"
#include "mesh_app_vendor_device_msg_handler.h"
#include "mesh_app_vendor_device_pwm.h"

#ifdef MTK_LED_ENABLE
#include "bsp_led.h"
#endif
#include "hal_sys.h"
#include "hal_cache.h"

#define RELAY_ENABLE    (1)

#define VENDOR_COMPANY_ID 0x0094
#define VENDOR_MODEL_ID1 0x002A
#define VENDOR_MODEL_ID2 0x002B
/* vendor client opcode */
#define VENDOR_OPCODE_1 0xC1
#define VENDOR_OPCODE_2 0xC2
#define VENDOR_OPCODE_3 0x00C3
#define VENDOR_OPCODE_4 0x00C4
#define VENDOR_OPCODE_5 0x00C5
#define VENDOR_OPCODE_6 0x00C6
#define VENDOR_OPCODE_7 0x00C7

#define VENDOR_MSG_HEADER 3


#define MESH_BYPASS_IO              3

#define GPIO_NUM_19		19
#define GPIO_NUM_19_MASK  ( 1<<GPIO_NUM_19 )
#define GPIO_NUM_20		20
#define GPIO_NUM_20_MASK  ( 1<<GPIO_NUM_20 )
#define GPIO_NUM_21		21
#define GPIO_NUM_21_MASK  ( 1<<GPIO_NUM_21 )
#define GPIO_INPUT		0
#define GPIO_OUTPUT		1

#define PHASE_0   0
#define PHASE_1   1
#define PHASE_2   2
#define PHASE_3   3

#define LED_VENDOR_RED      0xA1
#define LED_VENDOR_GREEN    0xA2
#define LED_VENDOR_BLUE     0xA3

#define LED_ON          1
#define LED_OFF         0

#define BSP_LED0_PIN 0
/******************************************************************************/
/* prototype                                                                  */
/******************************************************************************/
enum {
    TLV_TYPE_UUID = 0,
};

enum {
    TEST_LIGHT_OFF = 0,
    TEST_LIGHT_RED,
    TEST_LIGHT_GREEN,
    TEST_LIGHT_BLUE,
    TEST_LIGHT_CYAN,
    TEST_LIGHT_MAGENTA,
    TEST_LIGHT_YELLOW
};
/******************************************************************************/
/* global variables                                                           */
/******************************************************************************/
uint16_t g_appkey_index = 0x123;
static generic_onoff_server_model_t gOnOffServer;
static default_transition_time_server *gTransition_timer;
static uint16_t g_model_handle_onoff_server = 0;
static uint16_t g_model_handle_config_server = 0;
static uint16_t g_model_handle_health_server = 0;
static uint16_t g_model_handle_vendor = 0;
static uint16_t g_model_handle_vendor1 = 0;
#if SUPPORT_CTL
static uint16_t g_model_handle_ctl_server = 0;
#endif
#if SUPPORT_HSL
static uint16_t g_model_handle_hsl_server = 0;
#endif

static TimerHandle_t mesh_bat_timer = NULL; /**< Timer handler. */
static TimerHandle_t mesh_init_timer = NULL; /**< Timer handler. */
static TimerHandle_t mesh_led_timer = NULL; /**< Timer handler. */
static TimerHandle_t mesh_on_off_delay_timer = NULL; /**< Timer handler. */
static TimerHandle_t mesh_on_off_timer = NULL; /**< Timer handler. */

static uint8_t deviceUuid[BT_MESH_UUID_SIZE] = {
    0x00, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88, 0x99, 0xaa, 0xbb, 0xcc, 0xdd, 0xee, 0xff
};
static uint8_t g_test_state = TEST_LIGHT_OFF;
// LED
static uint8_t ledPhase = 0;
static bool mesh_inited = false;

static uint32_t gMissingCount = 0;
static uint32_t gCompareFailedCount = 0;
static uint16_t gLastIdx = 0xffff;
/*****************************************************************************/
/* main                                                                      */
/*****************************************************************************/

void vendor_led_rgb(uint8_t r, uint8_t g, uint8_t b)
{
    LOG_I(mesh_app,"R_PWM:%d,G_PWM:%d,B_PWM:%d\n", r, g, b);

    /*DRV_TMR16_Disable();
    DRV_TMR16_Reset(true);
    DRV_TMR16_Reset(false);

    if (r == 0) {
        MY_DRV_TMR16_PWM1_Set_Period_And_Set_Off(255 - r, 255);
    } else {
        MY_DRV_TMR16_PWM1_Set_Period(255 - r, 255);
    }

    if (b == 0) {
        MY_DRV_TMR16_PWM2_Set_Off(255 - b, 255);
    } else {
        MY_DRV_TMR16_PWM2(255 - b, 255);
    }

    if (g == 0) {
        MY_DRV_TMR16_PWM3_Set_Off(255 - g, 255);
    } else {
        MY_DRV_TMR16_PWM3(255 - g, 255);
    }

    DRV_TMR16_PWM_Enable( 15 );*/
}

void vendor_led_on_off( uint8_t ucLed , bool bOnOff )
{
#ifdef MTK_LED_ENABLE
    if(bOnOff) {
        bsp_led_on(BSP_LED0_PIN);
    } else {
        bsp_led_off(BSP_LED0_PIN);
    }
#endif
}

static uint32_t _get_time_in_ms(uint8_t time)
{
    uint8_t resolution = (time & 0xC0) >> 6;
    uint8_t steps = (time & 0x3F);
    uint32_t ms = 0;

    switch (resolution) {
        case 0: // 100ms
            ms = steps * 100;
            break;
        case 1: // 1 second
            ms = steps * 1000;
            break;
        case 2: // 10 seconds
            ms = steps * 10000;
            break;
        case 3: // 10 minutes
            ms = steps * 600000;
            break;
    }
    return ms;
}

extern uint32_t gTransitionTick;
static void _generic_device_timer_handler(TimerHandle_t handle)
{
    if (mesh_on_off_delay_timer == handle) {
        if (gOnOffServer.target_on_off == 1) {
            gOnOffServer.present_on_off = gOnOffServer.target_on_off;
            vendor_led_on_off( LED_VENDOR_RED, LED_ON );
        }
        gTransitionTick = xTaskGetTickCount() * portTICK_PERIOD_MS;
        bt_mesh_app_util_start_timer(mesh_on_off_timer, _get_time_in_ms(gOnOffServer.transition_time));
    }
    else if (mesh_on_off_timer == handle)
    {
        gOnOffServer.present_on_off = gOnOffServer.target_on_off;
        gOnOffServer.transition_time = 0;
        gOnOffServer.delay = 0;

        if (gOnOffServer.present_on_off == 1) {
            vendor_led_on_off( LED_VENDOR_RED, LED_ON );
        } else {
            vendor_led_on_off( LED_VENDOR_RED, LED_OFF );
        }
    }
}

/*Application messages start*/
static void msg_generic_on_off_status(uint16_t model_handle, uint8_t presentOnOff, uint8_t targetOnOff,
                                      uint8_t remainingTime, const bt_mesh_access_message_rx_t *msg)
{
    uint8_t buffer[3];
    uint8_t length = 1;

    buffer[0] = presentOnOff;

    if(remainingTime != 0)
    {
        length = 3;
        buffer[1] = targetOnOff;
        buffer[2] = remainingTime;
    }

    bt_mesh_model_generic_on_off_status(model_handle, buffer, length, msg);
}
/*Application messages end*/

void _vendor1_publish_callback(uint16_t model_handle, void *arg)
{
    VD_DEBUG_COLOR_PRINTF("Vendor1(Publish)", 1);
}

void _vendor2_publish_callback(uint16_t model_handle, void *arg)
{
    VD_DEBUG_COLOR_PRINTF("Vendor2(Publish)", 1);
}
void _generic_onoff_publish_callback(uint16_t model_handle, void *arg)
{
    VD_DEBUG_COLOR_PRINTF("_generic_onoff_publish_callback", 1);
    if (model_handle == g_model_handle_onoff_server) {
        VD_DEBUG_COLOR_PRINTF("GenericOnOffStatus(Publish)", 1);

        bt_mesh_access_message_tx_t msg;
        uint8_t buffer[1];
        buffer[0] = gOnOffServer.present_on_off;
        msg.opcode.opcode = BT_MESH_MODEL_GENERIC_ONOFF_STATUS;
        msg.opcode.company_id = MESH_MODEL_COMPANY_ID_NONE;
        msg.buffer = buffer;
        msg.length = 1;

        bt_mesh_status_t ret = bt_mesh_access_model_publish(model_handle, &msg);
        LOG_I(mesh_app, "ret = %d\n", ret);
    }
}

static void _generic_onoff_set_handler(uint16_t model_handle, const bt_mesh_access_message_rx_t *msg, const void *arg, bool reliable)
{
    LOG_I(mesh_app, "[R]Generic_OnOffSet={src=0x%04x, onoff=0x%02x, ttl=%d, tid=0x%02x, transTime=0x%02x, delay=0x%02x}, reliable=%s, ",
          msg->meta_data.src_addr, msg->buffer[0], msg->meta_data.ttl, msg->buffer[1], msg->buffer[2], msg->buffer[3],
          reliable ? "true" : "false");

    if (gOnOffServer.TID != msg->buffer[1]) { //new message
        gOnOffServer.TID = msg->buffer[1];

        LOG_I(mesh_app, "done.\n");

        gOnOffServer.target_on_off = msg->buffer[0];

        if (msg->length > 2) {
            gOnOffServer.transition_time = msg->buffer[2];
            gOnOffServer.delay = msg->buffer[3];
        } else {
            gOnOffServer.transition_time = gTransition_timer->default_transition_time;
        }

        if (gOnOffServer.transition_time == 0) {
            if (gOnOffServer.present_on_off != msg->buffer[0]) {
                gOnOffServer.present_on_off = msg->buffer[0];
                _generic_onoff_publish_callback(model_handle, NULL);
            }
        } else {
            bt_mesh_app_util_start_timer(mesh_on_off_delay_timer, _get_time_in_ms(gOnOffServer.delay * 5));
        }

        if (gOnOffServer.present_on_off == 1) {
            vendor_led_on_off( LED_VENDOR_RED, LED_ON );
        } else {
            vendor_led_on_off( LED_VENDOR_RED, LED_OFF );
        }
    } else {
        LOG_I(mesh_app, "ignored. (not new message)\n");
    }

    if (reliable) {
        msg_generic_on_off_status(model_handle, gOnOffServer.present_on_off, gOnOffServer.target_on_off, gOnOffServer.transition_time, msg);
    }
}

static void _generic_onoff_get_handler(uint16_t model_handle, const bt_mesh_access_message_rx_t *msg, const void *arg)
{
    LOG_I(mesh_app, "[R]Generic_OnOffGet={src=0x%04x, ttl=%d}", msg->meta_data.src_addr, msg->meta_data.ttl);

    msg_generic_on_off_status(model_handle, gOnOffServer.present_on_off, gOnOffServer.present_on_off, gOnOffServer.transition_time, msg);
}


static void _generic_server_msg_handler(uint16_t model_handle, const bt_mesh_access_message_rx_t *msg, const void *arg)
{
    switch (msg->opcode.opcode) {

        case BT_MESH_MODEL_GENERIC_ONOFF_SET: {
            VD_DEBUG_EVT_SIG(msg->opcode.opcode, 2);
            bt_mesh_app_util_display_hex_log(&msg->buffer[0], msg->length);
            _generic_onoff_set_handler(model_handle, msg, arg, true);
            break;
        }
        case BT_MESH_MODEL_GENERIC_ONOFF_SET_UNACKNOWLEDGED: {
            VD_DEBUG_EVT_SIG(msg->opcode.opcode, 2);
            bt_mesh_app_util_display_hex_log(&msg->buffer[0], msg->length);
            _generic_onoff_set_handler(model_handle, msg, arg, false);
            break;
        }
        case BT_MESH_MODEL_GENERIC_ONOFF_GET: {
            VD_DEBUG_EVT_SIG(msg->opcode.opcode, 2);
            bt_mesh_app_util_display_hex_log(&msg->buffer[0], msg->length);
            _generic_onoff_get_handler(model_handle, msg, arg);
            break;
        }
        case BT_MESH_MODEL_GENERIC_DEFAULT_TRANSITION_TIME_GET: {
            VD_DEBUG_EVT_PRINTF("GenericDefaultTransitionTimeGet", 2);

            bt_mesh_model_generic_default_transition_time_status(model_handle,
                gTransition_timer->default_transition_time, msg);
            break;
        }
        case BT_MESH_MODEL_GENERIC_DEFAULT_TRANSITION_TIME_SET: {
            if ((msg->buffer[0] & 0x3f) == 0x3F) {
                LOG_I(mesh_app,"invalid value [%02x]\n", msg->buffer[0]);
                return;
            }

            gTransition_timer->default_transition_time = msg->buffer[0];
            VD_DEBUG_EVT_PRINTF("GenericDefaultTransitionTimeSet", 2);
            LOG_I(mesh_app,"time  : %x\n", gTransition_timer->default_transition_time);

            bt_mesh_model_generic_default_transition_time_status(model_handle,
                gTransition_timer->default_transition_time, msg);
            break;
        }
        case BT_MESH_MODEL_GENERIC_DEFAULT_TRANSITION_TIME_SET_UNACKNOWLEDGED: {
            if ((msg->buffer[0] & 0x3f) == 0x3F) {
                LOG_I(mesh_app,"invalid value [%02x]\n", msg->buffer[0]);
                return;
            }

            gTransition_timer->default_transition_time = msg->buffer[0];
            VD_DEBUG_EVT_PRINTF("GenericDefaultTransitionTimeSetUnacknowledged", 2);
            LOG_I(mesh_app, "time  : %x\n", gTransition_timer->default_transition_time);
            break;
        }

    }
}

void msg_vendor_latency_status(uint16_t model_handle, bt_mesh_access_message_rx_t *msg, uint8_t recv_hops)
{
    VD_DEBUG_COLOR_PRINTF("VendorLatencyStatus", 1);
    bt_mesh_access_message_tx_t *reply;

    reply = (bt_mesh_access_message_tx_t *)bt_mesh_app_util_alloc(sizeof(bt_mesh_access_message_tx_t));
    reply->opcode.company_id = VENDOR_COMPANY_ID;
    reply->opcode.opcode = VENDOR_OPCODE_2;
    reply->length = 2;
    reply->buffer = bt_mesh_app_util_alloc(2);
    reply->buffer[0] = bt_mesh_config_get_default_ttl();
    reply->buffer[1] = recv_hops;

    LOG_I(mesh_app, "[T]Vendor_LatencyStatus={dst=0x%04x, initTTL=0x%02x, recvHops=0x%02x}\n",
          msg->meta_data.src_addr, reply->buffer[0], reply->buffer[1]);
    bt_mesh_access_model_reply(model_handle, msg, reply);

    bt_mesh_app_util_free(reply->buffer);
    bt_mesh_app_util_free((uint8_t *)reply);

}

#if 0
static void msg_op_send(uint8_t opcode, uint16_t dst_addr, uint8_t recv_hops, uint8_t ttl)
{
    VD_DEBUG_COLOR_PRINTF("msg_op_send", 1);

    bt_mesh_tx_params_t param;
    param.dst.value = dst_addr;
    param.dst.type = bt_mesh_utils_get_addr_type(param.dst.value);
    param.src = bt_mesh_model_get_element_address(0);
    param.ttl = ttl;
    param.data_len = 5;

    uint8_t *payload = bt_mesh_app_util_alloc(param.data_len);

    payload[0] = opcode;
    payload[1] = VENDOR_COMPANY_ID & 0x00FF;
    payload[2] = (VENDOR_COMPANY_ID & 0xFF00) >> 8;
    payload[3] = ttl;
    payload[4] = recv_hops;

    param.data = payload;
    param.security.appidx = 0x123;
    param.security.netidx = BT_MESH_GLOBAL_PRIMARY_NETWORK_KEY_INDEX;
    bt_mesh_status_t ret = bt_mesh_send_packet(&param);

    LOG_I(mesh_app, "[T]Vendor_LatencyStatus={dst=0x%04x, initTTL=0x%02x, recvHops=0x%02x}, ret = 0x%x\n",
          dst_addr, ttl, recv_hops, ret);
    bt_mesh_app_util_free(payload);
}
#endif
static void msg_op_send_ex(uint8_t opcode, uint16_t dst_addr, uint8_t recv_hops, uint8_t ttl, uint16_t idx)
{
    VD_DEBUG_COLOR_PRINTF("msg_op_send_ex", 1);

    bt_mesh_tx_params_t param;
    param.dst.value = dst_addr;
    param.dst.type = bt_mesh_utils_get_addr_type(param.dst.value);
    param.src = bt_mesh_model_get_element_address(0);
    param.ttl = ttl;
    param.data_len = 7;

    uint8_t *payload = bt_mesh_app_util_alloc(param.data_len);

    payload[0] = opcode;
    payload[1] = VENDOR_COMPANY_ID & 0x00FF;
    payload[2] = (VENDOR_COMPANY_ID & 0xFF00) >> 8;
    payload[3] = ttl;
    payload[4] = recv_hops;
    payload[5] = idx & 0x00FF;
    payload[6] = (idx & 0xFF00) >> 8;

    param.data = payload;
    param.security.appidx = 0x123;
    param.security.netidx = BT_MESH_GLOBAL_PRIMARY_NETWORK_KEY_INDEX;
    bt_mesh_status_t ret = bt_mesh_send_packet(&param);

    LOG_I(mesh_app, "[T]Vendor_LatencyStatus={dst=0x%04x, initTTL=0x%02x, recvHops=0x%02x, idx=0x%x}, ret = 0x%x\n",
           dst_addr, ttl, recv_hops, idx, ret);
    bt_mesh_app_util_free(payload);
}

static void _msg_check(uint16_t idx, const bt_mesh_access_message_rx_t *msg)
{
    if (msg->length == VENDOR_MSG_HEADER)
    {
        if (idx - gLastIdx > 1)
        {
            gMissingCount += (idx - gLastIdx - 1);
            LOG_I(mesh_app, "receive idx %d, missing %d, missingCount %d\n", idx, gLastIdx + 1, gMissingCount);
        }
        else
        {
            LOG_I(mesh_app, "receive idx %d, missingCount %d\n", idx, gMissingCount);
        }
    }
    else
    {
        uint16_t i;

        for (i = 0; i < (msg->length - VENDOR_MSG_HEADER); i++)
        {
            if (msg->buffer[VENDOR_MSG_HEADER+i] != (i&0xff))
            {
                gCompareFailedCount++;
                LOG_I(mesh_app, "failed idx %d\n", i);
                break;
            }
        }
        
        if (idx - gLastIdx > 1)
        {
            gMissingCount += (idx - gLastIdx - 1);
            LOG_I(mesh_app, "receive idx %d, missing %d, missingCnt %d, failedCnt %d\n", idx, (idx - gLastIdx - 1), gMissingCount, gCompareFailedCount);
        }
        else
        {
            LOG_I(mesh_app, "receive idx %d, missingCount %d, contentFailedCnt %d\n", idx, gMissingCount, gCompareFailedCount);
        }
    }
    gLastIdx = idx;
}

static void _vendorServerMsgHandler(uint16_t model_handle, const bt_mesh_access_message_rx_t *msg, const void *arg)
{
    VD_DEBUG_EVT_VENDOR(VENDOR_COMPANY_ID, msg->opcode.opcode, 2);
    bt_mesh_app_util_display_hex_log(&msg->buffer[0], msg->length);

    switch (msg->opcode.opcode) {
        case VENDOR_OPCODE_1: {
            uint16_t idx = msg->buffer[1] + ((msg->buffer[2]<<8)&0xff00);
            _msg_check(idx, msg);
            msg_op_send_ex(VENDOR_OPCODE_2, msg->meta_data.src_addr, msg->buffer[0] - msg->meta_data.ttl + 1, msg->meta_data.ttl, idx);
            LOG_I(mesh_app, "[R]Vendor_LatencyMeasure={src=0x%04x, initTTL=0x%02x}, metaTTL=0x%02x\n", msg->meta_data.src_addr, msg->buffer[0], msg->meta_data.ttl);
            //msg_op2_reply(model_handle, msg, msg->buf[0] - msg->meta_data.ttl + 1, msg->meta_data.ttl);
            //msg_op_send(VENDOR_OPCODE_2, msg->meta_data.src_addr, msg->buffer[0] - msg->meta_data.ttl + 1, msg->meta_data.ttl);

#if 0
            if (msg->meta_data.dst_addr == bt_mesh_model_get_element_address(g_vendor_element_index)) {
                msg_op1_send(bt_mesh_model_get_element_address(g_vendor_element_index) + 1, bt_mesh_model_get_element_address(g_vendor_element_index),  msg->buf[0], msg->meta_data.ttl - 1);
            }
#endif
            break;
        }
        case VENDOR_OPCODE_2: {
            LOG_I(mesh_app, "[R]Vendor_LatencyStatus={src=0x%04x, initTTL=0x%02x, recvTTL=0x%02x}, metaTTL=0x%02x\n",
                  msg->meta_data.src_addr, msg->buffer[0], msg->buffer[1], msg->meta_data.ttl);

#if 0
            if (msg->meta_data.dst_addr == bt_mesh_model_get_element_address(g_vendor_element_index)) {
                msg_op_send(VENDOR_OPCODE_2, bt_mesh_model_get_element_address(g_vendor_element_index) - 1, bt_mesh_model_get_element_address(g_vendor_element_index),  msg->buf[1], msg->meta_data.ttl - 1);
            }
#endif

            break;
        }
        case VENDOR_OPCODE_3: {
            uint16_t idx = msg->buffer[1] + ((msg->buffer[2]<<8)&0xff00);
            _msg_check(idx, msg);
            msg_op_send_ex(VENDOR_OPCODE_3, msg->meta_data.src_addr, msg->buffer[0] - msg->meta_data.ttl + 1, msg->meta_data.ttl, idx);
            break;
        }

        case VENDOR_OPCODE_4: {
            uint16_t idx = msg->buffer[1] + ((msg->buffer[2]<<8)&0xff00);
            _msg_check(idx, msg);
            msg_op_send_ex(VENDOR_OPCODE_4, msg->meta_data.src_addr, msg->buffer[0] - msg->meta_data.ttl + 1, msg->meta_data.ttl, idx);
            break;
        }

        case VENDOR_OPCODE_5: {
            uint16_t idx = msg->buffer[1] + ((msg->buffer[2]<<8)&0xff00);
            _msg_check(idx, msg);
            msg_op_send_ex(VENDOR_OPCODE_5, msg->meta_data.src_addr, msg->buffer[0] - msg->meta_data.ttl + 1, msg->meta_data.ttl, idx);
            break;
        }

        case VENDOR_OPCODE_6: {
            uint16_t idx = msg->buffer[1] + ((msg->buffer[2]<<8)&0xff00);
            _msg_check(idx, msg);
            msg_op_send_ex(VENDOR_OPCODE_6, msg->meta_data.src_addr, msg->buffer[0] - msg->meta_data.ttl + 1, msg->meta_data.ttl, idx);
            break;
        }
        case VENDOR_OPCODE_7: {
            uint16_t idx = msg->buffer[1] + ((msg->buffer[2]<<8)&0xff00);
            _msg_check(idx, msg);
            msg_op_send_ex(VENDOR_OPCODE_7, msg->meta_data.src_addr, msg->buffer[0] - msg->meta_data.ttl + 1, msg->meta_data.ttl, idx);
            LOG_I(mesh_app, "[R]Vendor_LatencyMeasure={src=0x%04x, initTTL=0x%02x}, metaTTL=0x%02x\n", msg->meta_data.src_addr, msg->buffer[0], msg->meta_data.ttl);
            break;
        }

        default:
            return;
    }

    return;
}

static void _vendorClientMsgHandler(uint16_t model_handle, const bt_mesh_access_message_rx_t *msg, const void *arg)
{
    VD_DEBUG_EVT_VENDOR(VENDOR_COMPANY_ID, msg->opcode.opcode, 2);
    bt_mesh_app_util_display_hex_log(&msg->buffer[0], msg->length);

    switch (msg->opcode.opcode) {
        case VENDOR_OPCODE_3: {
            // TODO: handle vendor opcode 3
            break;
        }
        case VENDOR_OPCODE_4: {
            // TODO: handle vendor opcode 4
            break;
        }
        default:
            return;
    }

    return;
}

static void _configuration_server_msg_handler(uint16_t model_handle, const bt_mesh_access_message_rx_t *msg, const void *arg)
{
    VD_DEBUG_EVT_SIG(msg->opcode.opcode, 2);
    bt_mesh_app_util_dump_pdu("configMsgHandler", msg->length, &msg->buffer[0]);
}

const bt_mesh_access_opcode_handler_t vendor_client_message_handler[] = {
    {{VENDOR_OPCODE_3, VENDOR_COMPANY_ID}, _vendorClientMsgHandler},
    {{VENDOR_OPCODE_4, VENDOR_COMPANY_ID}, _vendorClientMsgHandler},
    
};

const bt_mesh_access_opcode_handler_t vendor_server_message_handler[] = {
    {{VENDOR_OPCODE_1, VENDOR_COMPANY_ID}, _vendorServerMsgHandler},
    {{VENDOR_OPCODE_2, VENDOR_COMPANY_ID}, _vendorServerMsgHandler},
    {{VENDOR_OPCODE_3, VENDOR_COMPANY_ID}, _vendorServerMsgHandler},
    {{VENDOR_OPCODE_4, VENDOR_COMPANY_ID}, _vendorServerMsgHandler},
    {{VENDOR_OPCODE_5, VENDOR_COMPANY_ID}, _vendorServerMsgHandler},
    {{VENDOR_OPCODE_6, VENDOR_COMPANY_ID}, _vendorServerMsgHandler}, 
    {{VENDOR_OPCODE_7, VENDOR_COMPANY_ID}, _vendorServerMsgHandler},
};

#if SUPPORT_CTL
static void mesh_create_ctl_server(uint16_t element_index)
{
    uint8_t element_count;
    uint16_t *element_list;

    gCTL_server = (lighting_ctl_server_t *)bt_mesh_app_util_alloc(sizeof(lighting_ctl_server_t));
    gCTL_temperature_server = (lighting_ctl_temperature_server_t *)bt_mesh_app_util_alloc(sizeof(lighting_ctl_temperature_server_t));
    memset(gCTL_server, 0, sizeof(lighting_ctl_server_t));
    memset(gCTL_temperature_server, 0, sizeof(lighting_ctl_temperature_server_t));

    gCTL_server->range_min = 0x320;
    gCTL_server->range_max = 0x4E20;
    gCTL_server->default_temperature = 0x320;   // default value for GENERIC_ON_POWERUP_RESTORE
    gCTL_server->default_delta_uv = 0;  // default value for GENERIC_ON_POWERUP_RESTORE
    gCTL_server->target_ctl_temperature = 0x2000; // last known value for GENERIC_ON_POWERUP_RESTORE
    gCTL_server->target_ctl_delta_uv = -100;    // last known value for GENERIC_ON_POWERUP_RESTORE
    gCTL_server->lightness_server.target_lightness = 0xabcd;    // last known value for GENERIC_ON_POWERUP_RESTORE
    gCTL_server->lightness_server.last_lightness = 0x1234;
    gCTL_server->lightness_server.default_lightness = 0x0000;   // default value for GENERIC_ON_POWERUP_RESTORE
    gCTL_server->lightness_server.range_min = 0x0001;
    gCTL_server->lightness_server.range_max = 0xffff;
    gCTL_server->lightness_server.onoff_server.target_on_off = 1; // last known value for GENERIC_ON_POWERUP_RESTORE
    gCTL_server->lightness_server.onpowerup_server.on_power_up = GENERIC_ON_POWERUP_RESTORE;
    gCTL_server->lightness_server.TID = 0xF0; //default value for TID
    gCTL_server->lightness_server.level_server.TID = 0xF0; //default value for TID
    gCTL_server->lightness_server.dtt_server = gTransition_timer;
    gCTL_server->element_index = 0xFFFF;
    gCTL_server->TID = 0xF0; //default value for TID

    gCTL_temperature_server->element_index = 0xFFFF;
    gCTL_temperature_server->TID = 0xF0; //default value for TID

    /* CTL server for DemoKit LED, containing 2 elements for Light CTL Server, Light CTL Temperature Server */
    bt_mesh_model_lighting_add_ctl_setup_server(&g_model_handle_ctl_server, &element_count, &element_list, element_index, light_ctl_server_msg_handler, NULL);
    LOG_I(mesh_app, "ctl element_index = %d element_count = %d", element_index, element_count);
    gCTL_server->element_index = element_index;
    if(element_list != NULL && element_list[0] == element_index)
    {
        gCTL_temperature_server->element_index = element_list[1];
    }
    else
        LOG_I(mesh_app,"[ERROR]CTL server element_list wrong!\n");

    //bt_mesh_app_util_free element_list allocated when adding model.
    bt_mesh_app_util_free((uint8_t*)element_list);

    /* Binding temperature with gCTL_server */
    bind_ctl_temperature(gCTL_server, gCTL_temperature_server, MESH_MODEL_STATE_LIGHTING_CTL_TEMPERATURE, MESH_MODEL_BINDING_BOTH_VALUE);
}
#endif
#if SUPPORT_HSL
static void mesh_create_hsl_server(uint16_t element_index)
{
    uint8_t element_count;
    uint16_t *element_list;

    gHSL_server = (lighting_hsl_server_t *)bt_mesh_app_util_alloc(sizeof(lighting_hsl_server_t));
    gHSL_hue_server = (lighting_hsl_hue_server_t *)bt_mesh_app_util_alloc(sizeof(lighting_hsl_hue_server_t));
    gHSL_saturation_server = (lighting_hsl_saturation_server_t *)bt_mesh_app_util_alloc(sizeof(lighting_hsl_saturation_server_t));
    memset(gHSL_server, 0, sizeof(lighting_hsl_server_t));
    memset(gHSL_hue_server, 0, sizeof(lighting_hsl_hue_server_t));
    memset(gHSL_saturation_server, 0, sizeof(lighting_hsl_saturation_server_t));

    /* Default blue */
    gHSL_server->default_hue = 0xaaaa;
    gHSL_server->default_saturation = 0xffff;
    gHSL_server->hue_range_min = 0;
    gHSL_server->hue_range_max = 0xFFFF;
    gHSL_server->saturation_range_min = 0;
    gHSL_server->saturation_range_max = 0xFFFF;
    gHSL_server->lightness_server.default_lightness = 0x8000;
    gHSL_server->lightness_server.range_min = 0x0001;
    gHSL_server->lightness_server.range_max = 0xffff;
    gHSL_server->lightness_server.onpowerup_server.on_power_up = GENERIC_ON_POWERUP_DEFAULT;
    gHSL_server->lightness_server.TID = 0xF0; //default value for TID
    gHSL_server->lightness_server.level_server.TID = 0xF0; //default value for TID
    gHSL_server->element_index = 0xFFFF;
    gHSL_server->TID = 0xF0; //default value for TID
    gHSL_server->lightness_server.dtt_server = gTransition_timer;
    gHSL_hue_server->element_index = 0xFFFF;
    gHSL_hue_server->TID = 0xF0; //default value for TID
    gHSL_saturation_server->element_index = 0xFFFF;
    gHSL_saturation_server->TID = 0xF0; //default value for TID

    /* HSL server for DemoKit LED, containing 3 elements for Light HSL Server, Light HSL Hue Server, Light HSL Saturation Server */
    bt_mesh_model_lighting_add_hsl_setup_server(&g_model_handle_hsl_server, &element_count, &element_list, element_index, light_hsl_server_msg_handler, NULL);

    LOG_I(mesh_app, "hsl element_index = %d element_count = %d", element_index, element_count);
    gHSL_server->element_index = element_index;

    if(element_list != NULL && element_list[0] == element_index)
    {
        gHSL_hue_server->element_index = element_list[1];
        gHSL_saturation_server->element_index = element_list[2];
    }
    else
        LOG_I(mesh_app,"[ERROR]HSL server element_list wrong!\n");

    //bt_mesh_app_util_free element_list allocated when adding model.
    bt_mesh_app_util_free((uint8_t*)element_list);

    /* Binding hue and saturation with gHSL_server */
    bind_hsl_hue(gHSL_hue_server, gHSL_server, MESH_MODEL_STATE_LIGHTING_HSL_HUE, MESH_MODEL_BINDING_BOTH_VALUE);
    bind_hsl_saturation(gHSL_saturation_server, gHSL_server, MESH_MODEL_STATE_LIGHTING_HSL_SATURATION, MESH_MODEL_BINDING_BOTH_VALUE);
}
#endif

static void mesh_create_device(void)
{
    uint8_t composition_data_header[10] = {
        0x94, 0x00, // cid
        0x1A, 0x00, // pid
        0x01, 0x00, // vid
        0x64, 0x00, // crpl, BT_MESH_REPLAY_PROTECTION_LIST_DEFAULT=10
#if RELAY_ENABLE
        BT_MESH_FEATURE_RELAY | BT_MESH_FEATURE_PROXY | BT_MESH_FEATURE_FRIEND, 0x00, // features
#else
        BT_MESH_FEATURE_PROXY | BT_MESH_FEATURE_FRIEND, 0x00, // features
#endif
    };
    uint16_t element_index;
    uint16_t model_handle;

    bt_mesh_model_add_params_t model_params;

    memset(&gOnOffServer, 0, sizeof(generic_onoff_server_model_t));

    gTransition_timer = (default_transition_time_server*)bt_mesh_app_util_alloc(sizeof(default_transition_time_server));
    memset(gTransition_timer, 0, sizeof(default_transition_time_server));

    gOnOffServer.TID = 0xF0; //default value for TID

    bt_mesh_model_set_composition_data_header(composition_data_header, 10);
    bt_mesh_model_add_element(&element_index, BT_MESH_MODEL_ELEMENT_LOCATION_MAIN);

    bt_mesh_model_add_configuration_server(&g_model_handle_config_server, _configuration_server_msg_handler);
    bt_mesh_model_add_health_server(&g_model_handle_health_server, element_index, NULL);

    /* onoff server for EVB LED1 */
    bt_mesh_model_add_generic_onoff_server(&g_model_handle_onoff_server, element_index,
                                           _generic_server_msg_handler,
                                           _generic_onoff_publish_callback);

    bt_mesh_model_add_generic_default_transition_server(&model_handle, element_index,
                                            _generic_server_msg_handler,
                                            NULL);//todo

    /* vendor model 1 */
    model_params.model_id = MESH_VENDOR_MODEL_ID(VENDOR_COMPANY_ID, VENDOR_MODEL_ID1);
    model_params.element_index = element_index;
    model_params.opcode_handlers = vendor_server_message_handler;
    model_params.opcode_count = sizeof(vendor_server_message_handler) / sizeof(bt_mesh_access_opcode_handler_t);
    model_params.publish_timeout_cb = _vendor1_publish_callback;
    bt_mesh_model_add_model(&g_model_handle_vendor, &model_params);

    /* vendor model 2 */
    model_params.model_id = MESH_VENDOR_MODEL_ID(VENDOR_COMPANY_ID, VENDOR_MODEL_ID2);
    model_params.element_index = element_index;
    model_params.opcode_handlers = vendor_client_message_handler;
    model_params.opcode_count = sizeof(vendor_client_message_handler) / sizeof(bt_mesh_access_opcode_handler_t);
    model_params.publish_timeout_cb = _vendor2_publish_callback;

    bt_mesh_model_add_model(&g_model_handle_vendor1, &model_params);
    
#if SUPPORT_CTL
    bt_mesh_model_add_element(&element_index, BT_MESH_MODEL_ELEMENT_LOCATION_FIRST);
    mesh_create_ctl_server(element_index);
#endif
    
#if SUPPORT_HSL
    bt_mesh_model_add_element(&element_index, BT_MESH_MODEL_ELEMENT_LOCATION_SECOND);
    mesh_create_hsl_server(element_index);
#endif
    bind_generic_power_up(MESH_MODEL_STATE_GENERIC_ON_POWER_UP, MESH_MODEL_BINDING_BOTH_VALUE);
}

void _led_timer_cb(TimerHandle_t handle)
{
    switch ( ledPhase++ ) {
        case PHASE_0:
            vendor_led_on_off( LED_VENDOR_BLUE, LED_OFF );
            break;

        case PHASE_1:
            vendor_led_on_off( LED_VENDOR_BLUE, LED_ON );
            break;

        case PHASE_2:
            vendor_led_on_off( LED_VENDOR_BLUE, LED_OFF );
            break;

        case PHASE_3:
            vendor_led_on_off( LED_VENDOR_BLUE, LED_ON );
            bt_mesh_app_util_stop_timer(mesh_led_timer);
            break;
    }
}

static void _mesh_check_init(TimerHandle_t handle)
{
    if (!mesh_inited) {
        LOG_I(mesh_app, "Mesh initialize failed, going to SW reset...");
        
        hal_cache_disable();
        hal_cache_deinit();
        hal_sys_reboot(HAL_SYS_REBOOT_MAGIC, WHOLE_SYSTEM_REBOOT_COMMAND);        
    }
}

static void _mesh_check_battery_level(TimerHandle_t handle)
{
    /*uint16_t voltage; //voltage in 10mV

    voltage = AB_ADC_Get_Voltage(VBAT);

    switch(voltage)
    {
        case 360 ... 65535:
            bt_mesh_debug_printf(bt_mesh_log_dir_no, bt_mesh_log_error, "[WARNING] Voltage is too high!%dmV\n", voltage*10);
            break;
        case 0 ... 200:
            bt_mesh_debug_printf(bt_mesh_log_dir_no, bt_mesh_log_error, "[WARNING] Voltage is too low!%dmV\n", voltage*10);
            //AB_PWR_Off();
            break;
        default:
            break;
    }*/
}

static void mesh_timer_init(void)
{
    bt_mesh_app_util_init_timer(&mesh_bat_timer, "mesh bat", true, _mesh_check_battery_level);
    bt_mesh_app_util_init_timer(&mesh_init_timer, "mesh init", false, _mesh_check_init);
    bt_mesh_app_util_init_timer(&mesh_led_timer, "mesh led", true, _led_timer_cb);
    bt_mesh_app_util_start_timer(mesh_bat_timer, 10000);
    bt_mesh_app_util_start_timer(mesh_init_timer, 30000);
    bt_mesh_app_util_start_timer(mesh_led_timer, 200);
    
    bt_mesh_app_util_init_timer(&mesh_on_off_delay_timer, "mesh on off delay", false, _generic_device_timer_handler);
    bt_mesh_app_util_init_timer(&mesh_on_off_timer, "mesh on off", false, _generic_device_timer_handler);
}

static void mesh_led_init(void)
{
#ifdef MTK_LED_ENABLE
    bsp_led_init(BSP_LED0_PIN);
#endif
}

static void vendor_led_init(void)
{
}

/*
void _mesh_ota_cb(OTA_STATUS_ENUM status)
{

    mesh_firmware_updating = false;	//thus OTA app may reset the timer in case the OTA time too long

    switch (status) {
        case OTA_STARTED:
            bt_mesh_debug_printf(bt_mesh_log_dir_no, bt_mesh_log_verbose, "OTA started!!!");
            mesh_firmware_updating = true;
            break;
        case OTA_FINISHED:
            bt_mesh_debug_printf(bt_mesh_log_dir_no, bt_mesh_log_verbose, "OTA finised!!!");
            mesh_firmware_updating = false;
            break;
        case OTA_APPLY:
            bt_mesh_debug_printf(ble_mesh_log_dir_no, ble_mesh_log_verbose, "OTA applied, need to reboot.");
            AB_PWR_Rst();
            break;
        case OTA_ACCESSED_WHILE_DISABLED:
            bt_mesh_debug_printf(ble_mesh_log_dir_no, ble_mesh_log_verbose, "OTA accessed while disabled.");
            break;
    }
}*/

static void mesh_ota_init(void)
{
    /*OTA_init(_mesh_ota_cb);

    if (1) { // may detect ADC and decide whether OTA should be enabled
        OTA_change_permission(OTA_ENABLED);
    }
    LOG_I(mesh_app, "OTA service init done.");*/
}

void mesh_dump_model_handle(void)
{
    LOG_I(mesh_app, "mesh_dump_model_handle\n");
    LOG_I(mesh_app, "g_model_handle_config_server %x\n", g_model_handle_config_server);
    LOG_I(mesh_app, "g_model_handle_health_server %x\n", g_model_handle_health_server);
    LOG_I(mesh_app, "g_model_handle_vendor %x\n", g_model_handle_vendor);
    LOG_I(mesh_app, "g_model_handle_vendor1 %x\n", g_model_handle_vendor1);
    LOG_I(mesh_app, "g_model_handle_onoff_server %x\n", g_model_handle_onoff_server);
}

#define MESH_WIFI_SERVICE_TIME (20)    //32, 22, 18 
static void meshinit(void)
{
    LOG_I(mesh_app, "Mesh initialising...\n");

    bt_mesh_init_params_t *initparams = (bt_mesh_init_params_t *)bt_mesh_app_util_alloc(sizeof(bt_mesh_init_params_t));
    initparams->role = BT_MESH_ROLE_PROVISIONEE;

    // init role
    initparams->role = BT_MESH_ROLE_PROVISIONEE;

    // init provision parameter
    initparams->provisionee = (bt_mesh_prov_provisionee_params_t *)bt_mesh_app_util_alloc(sizeof(bt_mesh_prov_provisionee_params_t));
    initparams->provisionee->cap.number_of_elements = bt_mesh_model_get_element_count();
    initparams->provisionee->cap.algorithms = BT_MESH_PROV_CAPABILITY_ALGORITHM_FIPS_P256_ELLIPTIC_CURVE; // bit 0: P-256, bit 1~15: RFU
    initparams->provisionee->cap.public_key_type = BT_MESH_PROV_CAPABILITY_OOB_PUBLIC_KEY_TYPE_INBAND;
    initparams->provisionee->cap.static_oob_type = BT_MESH_PROV_CAPABILITY_OOB_STATIC_TYPE_SUPPORTED;
    initparams->provisionee->cap.output_oob_size = 0x00;
    initparams->provisionee->cap.output_oob_action = 0x0000;
    initparams->provisionee->cap.input_oob_size = 0x00;
    initparams->provisionee->cap.input_oob_action = 0x0000;

    // init config parameter
    initparams->config = (bt_mesh_config_init_params_t *)bt_mesh_app_util_alloc(sizeof(bt_mesh_config_init_params_t));
    memset(initparams->config, 0, sizeof(bt_mesh_config_init_params_t));
    memcpy(initparams->config->device_uuid, deviceUuid, BT_MESH_UUID_SIZE);
    initparams->config->oob_info = 0;
    initparams->config->uri = NULL;
    initparams->config->default_ttl = 4;

    initparams->friend = NULL;

    bt_mesh_init(initparams);
    bt_mesh_app_util_free((uint8_t *)(initparams->provisionee));
    bt_mesh_app_util_free((uint8_t *)(initparams->config));
    bt_mesh_app_util_free((uint8_t *)initparams);
}

bt_status_t bt_app_event_callback_demo(bt_msg_type_t msg, bt_status_t status, void *buff)
{
    switch (msg) {
        case BT_POWER_ON_CNF: {
            LOG_I(mesh_app, "BT_POWER_ON_CNF\n");
            bt_mesh_app_util_check_device_uuid(deviceUuid);
            // init wifi service time, default value 22
			bt_mesh_app_util_change_wifi_service_time(MESH_WIFI_SERVICE_TIME);
            mesh_led_init();
            vendor_led_init();
            mesh_ota_init();
            mesh_timer_init();
            // create mesh device
            mesh_create_device();
            meshinit();
            break;
        }
        case BT_GAP_LE_ADVERTISING_REPORT_IND: {
            bt_gap_le_advertising_report_ind_t *report = (bt_gap_le_advertising_report_ind_t *)buff;

            if (report->event_type == BT_GAP_LE_ADV_REPORT_EVT_TYPE_ADV_NONCONN_IND) {
            {
                uint8_t *ptr;
                uint8_t len;
                ptr = report->data;
                len = report->data_length;
                while(len > 0) {
                    if(ptr[0] == 4 && ptr[1] == BT_GAP_LE_AD_TYPE_MANUFACTURER_SPECIFIC)
                    {
                        /* | len | ad_type | company_id  | control | */
                        /* | len |   0xff  | 0x94 | 0x00 |   0x??  | */
                        uint16_t company_id;
                        memcpy(&company_id, ptr+2, 2);
                        if (company_id == VENDOR_COMPANY_ID) {
                            if (g_test_state == ptr[4]) {
                                return BT_STATUS_SUCCESS;
                            }

                            g_test_state = ptr[4];
                            LOG_I(mesh_app,"recv control message %d\n", ptr[4]);
                            switch(g_test_state) {
                                case TEST_LIGHT_OFF:
                                    vendor_led_rgb(0,0,0);
                                    break;
                                case TEST_LIGHT_RED:
                                    vendor_led_rgb(255,0,0);
                                    break;
                                case TEST_LIGHT_GREEN:
                                    vendor_led_rgb(0,255,0);
                                    break;
                                case TEST_LIGHT_BLUE:
                                    vendor_led_rgb(0,0,255);
                                    break;
                                case TEST_LIGHT_CYAN:
                                    vendor_led_rgb(0,255,255);
                                    break;
                                case TEST_LIGHT_MAGENTA:
                                    vendor_led_rgb(255,0,255);
                                    break;
                                case TEST_LIGHT_YELLOW:
                                    vendor_led_rgb(255,255,0);
                                    break;
                            }
                        }
                    }

                    ptr += (ptr[0] + 1);
                    len -= (ptr[0] + 1);
                }
            }
            break;
         }
         }
    }
    return BT_STATUS_SUCCESS;
}

bt_mesh_status_t bt_mesh_app_event_callback(bt_mesh_event_id evt_id, bool status, bt_mesh_evt_t *evt_data)
{
    switch (evt_id) {
        case BT_MESH_EVT_INIT_DONE: {
            LOG_I(mesh_app, "\n==============================\n");
            LOG_I(mesh_app, "BT_MESH_EVT_INIT_DONE\n");
            VD_DEBUG_DUMP_PDU("UUID", BT_MESH_UUID_SIZE, deviceUuid);
            LOG_I(mesh_app, "==============================\n\n");
            vendor_led_on_off( LED_VENDOR_BLUE, LED_ON );
            pwm_init_123();
            bt_mesh_enable();
            mesh_inited = true;
            break;
        }
        case BT_MESH_EVT_PROV_SHOW_OOB_PUBLIC_KEY: {
            LOG_I(mesh_app, "BT_MESH_EVT_PROV_SHOW_OOB_PUBLIC_KEY\n");
            bt_mesh_evt_prov_show_pk_t *p = (bt_mesh_evt_prov_show_pk_t *)evt_data;
            VD_DEBUG_DUMP_PDU("PublicKey", 64, p->pk);
            bt_mesh_app_util_free(p->pk);
            break;
        }
        case BT_MESH_EVT_PROV_REQUEST_OOB_AUTH_VALUE: {
            LOG_I(mesh_app, "BT_MESH_EVT_PROV_REQUEST_OOB_AUTH_VALUE\n");
            bt_mesh_evt_prov_request_auth_t *p = (bt_mesh_evt_prov_request_auth_t *)evt_data;
            LOG_I(mesh_app, "\tMethod : %d\n", p->method);
            LOG_I(mesh_app, "\tAction : %d\n", p->action);
            LOG_I(mesh_app, "\tSize   : %d\n", p->size);
            switch (p->method) {
                case BT_MESH_PROV_START_AUTHEN_METHOD_STATIC_OOB:
                    LOG_I(mesh_app, "\tMethod : Static OOB\n");
                    uint8_t auth_value[16] = {
                        0x0a, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                        0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08
                    };
                    bt_mesh_provision_provide_oob_auth_value(auth_value, 16);
                    break;
                case BT_MESH_PROV_START_AUTHEN_METHOD_OUTPUT_OOB:
                    LOG_I(mesh_app, "\tMethod : Ouput OOB\n");
                    break;
                case BT_MESH_PROV_START_AUTHEN_METHOD_INPUT_OOB:
                    LOG_I(mesh_app, "\tMethod : Input OOB\n");
                    break;
                default:
                    LOG_I(mesh_app, "\tMethod : %d, invalid\n", p->method);
                    break;
            }
            break;
        }
        case BT_MESH_EVT_PROV_SHOW_OOB_AUTH_VALUE: {
            bt_mesh_evt_prov_show_pk_t *p = (bt_mesh_evt_prov_show_pk_t *)evt_data;
            bt_mesh_app_util_dump_pdu("PublicKey", 64, p->pk);
            bt_mesh_app_util_free(p->pk);
            break;
        }
        case BT_MESH_EVT_PROV_DONE: {
            bt_mesh_evt_prov_done_t *p = (bt_mesh_evt_prov_done_t *)evt_data;
            LOG_I(mesh_app_must, "BT_MESH_EVT_PROV_DONE %s\n", status ? "SUCCESS" : "FAILED");
            LOG_I(mesh_app, "\tUnicastAddr: 0x%lx", p->address);
            LOG_I(mesh_app,  "\t");
            VD_DEBUG_DUMP_PDU("DeviceKey", BT_MESH_DEVKEY_SIZE, p->device_key);
            break;
        }
        case BT_MESH_EVT_CONFIG_RESET: {
            LOG_I(mesh_app, "BT_MESH_EVT_CONFIG_RESET, ready to do SW reset...\n");
            hal_cache_disable();
            hal_cache_deinit();
            hal_sys_reboot(HAL_SYS_REBOOT_MAGIC, WHOLE_SYSTEM_REBOOT_COMMAND);
            break;
        }
        case BT_MESH_EVT_HEARTBEAT: {
            LOG_I(mesh_app, "BT_MESH_EVT_HEARTBEAT\n");
            break;
        }
        case BT_MESH_EVT_IV_UPDATE: {
            LOG_I(mesh_app, "BT_MESH_EVT_IV_UPDATE\n");
            bt_mesh_evt_iv_update_t *p = (bt_mesh_evt_iv_update_t *)evt_data;
            LOG_I(mesh_app, "\tState    : 0x%x\n", p->state);
            LOG_I(mesh_app, "\tIV Index : 0x%08lx\n", p->iv_index);
            break;
        }
        case BT_MESH_EVT_KEY_REFRESH: {
            LOG_I(mesh_app, "BT_MESH_EVT_KEY_REFRESH\n");
            bt_mesh_evt_key_refresh_t *p = (bt_mesh_evt_key_refresh_t *)evt_data;
            LOG_I(mesh_app, "\tNetkeyIndex : 0x%03x\n", p->netkey_index);
            LOG_I(mesh_app, "\tPhase       : 0x%02x\n", p->phase);
        }
        case BT_MESH_EVT_BEARER_GATT_STATUS: {
            bt_mesh_evt_bearer_gatt_status_t *p = (bt_mesh_evt_bearer_gatt_status_t *)evt_data;
            LOG_I(mesh_app, "BT_MESH_EVT_BEARER_GATT_STATUS");
            LOG_I(mesh_app, "\tHandle:%ld status:%x", p->handle, p->status);
            break;
        }
        case BT_MESH_EVT_FRIENDSHIP_STATUS: {
            LOG_I(mesh_app, "BT_MESH_EVT_FRIENDSHIP_STATUS");
            bt_mesh_evt_friendship_status_t *p = (bt_mesh_evt_friendship_status_t *)evt_data;
            LOG_I(mesh_app, "\tLPN Address : 0x%03x", p->address);
            LOG_I(mesh_app, "\tStatus      : %x", p->status);
            break;
        }
    }
    
    return BT_MESH_SUCCESS;
}

extern void bt_mesh_control_msg_set_retry_count(uint8_t retry);

/******************************************************************************/
/* command line                                                               */
/******************************************************************************/
bt_status_t cmd_config(void *no, uint16_t argc, char **argv)
{
    if (argc < 2) {
        return BT_STATUS_FAIL;
    }

    if (!strcmp(argv[1], "feature")) {
        // config feature <feature>: relay=1, proxy: 2, friend=4, lpn=8
        if (argc < 4) {
            LOG_I(mesh_app, "USAGE: config feature <features>, <features>: relay=1, proxy: 2, friend=4, lpn=8\n");
            return BT_STATUS_FAIL;
        }
        if (atoi(argv[3]) != 0) {
            bt_mesh_config_enable_features(atoi(argv[2]));
        } else {
            bt_mesh_config_disable_features(atoi(argv[2]));
        }
    } else if (!strcmp(argv[1], "ttl")) {
        if (argc < 3) {
            LOG_I(mesh_app, "USAGE: config ttl <ttl>\n");
            return BT_STATUS_FAIL;
        }
        uint8_t ttl = 0;
        bt_mesh_app_util_str2u8HexNum(argv[2], &ttl);
        bt_mesh_config_set_default_ttl(ttl);
    } else if (!strcmp(argv[1], "ptout")) {
        if (argc < 4) {
            LOG_I(mesh_app, "USAGE: config ptout <tout1> <tout2>\n");
            return BT_STATUS_FAIL;
        }
        uint32_t nid = 0, node_identy = 0;
        bt_mesh_app_util_str2u32HexNum(argv[2], &nid);
        bt_mesh_app_util_str2u32HexNum(argv[3], &node_identy);
        bt_mesh_proxy_set_timeout(nid, node_identy);
    } else if (!strcmp(argv[1], "padv_tout")) {
        if (argc < 4) {
            LOG_I(mesh_app, "USAGE: config padv_tout <pb_adv_tout> <beacon_tout>\n");
            return BT_STATUS_FAIL;
        }
        uint32_t pb_adv_tout = 0, beacon_tout = 0;
        bt_mesh_app_util_str2u32HexNum(argv[2], &pb_adv_tout);
        bt_mesh_app_util_str2u32HexNum(argv[3], &beacon_tout);
        bt_mesh_provision_adv_set_timeout(pb_adv_tout, beacon_tout);
    } else if(!strcmp(argv[1], "retry")) {
        if (argc < 3) {
            LOG_I(mesh_app,"USAGE: config retry <retry>\n");
            return BT_STATUS_FAIL;
        }
        uint8_t retry = 0;
        bt_mesh_app_util_str2u8HexNum(argv[2], &retry);
        bt_mesh_control_msg_set_retry_count(retry);
    }


    return BT_STATUS_SUCCESS;
}

/*static bt_status_t cmd_iv(void *no, uint16_t argc, char **argv)
{
    if (argc < 2) {
        return BT_STATUS_FAIL;
    }

    if (!strcmp(argv[1], "test")) {
        if (argc < 3) {
            LOG_I(mesh_app,"USAGE: iv test <enable>\n");
            return BT_STATUS_FAIL;
        }
        bt_mesh_beacon_set_iv_update_test_mode(atoi(argv[2]));
    }
    return BT_STATUS_SUCCESS;
}*/

static bt_status_t cmd_delete(void *no, uint16_t argc, char **argv)
{
    if (argc != 1) {
        return BT_STATUS_FAIL;
    }

    printf("delete all mesh sectors.\n");
    bt_mesh_flash_reset(BT_MESH_FLASH_SECTOR_ALL);
    return BT_STATUS_SUCCESS;
}

/*static int cmd_delete_seq_f(void *no, uint16_t argc, char **argv)
{
    if (argc != 1) {
        return BT_STATUS_FAIL;
    }

    printf("delete mesh sequence number sector.\n");
    bt_mesh_flash_reset(BT_MESH_FLASH_SECTOR_SEQUENCE_NUMBER);
    return BT_STATUS_SUCCESS;
}

static int cmd_delete_data_f(void *no, uint16_t argc, char **argv)
{
    if (argc != 1) {
        return BT_STATUS_FAIL;
    }

    printf("delete mesh data sector.\n");
    bt_mesh_flash_reset(BT_MESH_FLASH_SECTOR_DATA);
    return BT_STATUS_SUCCESS;
}

static int cmd_delete_flash_f(void *no, uint16_t argc, char **argv)
{
    if (argc != 1) {
        return BT_STATUS_FAIL;
    }
#ifdef CHIP_AB1613
        printf("self erase flash valid pattern to enter download mode\n");
        hal_flash_erase(0x200000, HAL_FLASH_BLOCK_4K);
#else
        printf("invalid command, chip not 1613\n");

#endif
        return BT_STATUS_SUCCESS;

}*/

extern void bt_mesh_ota_updater_debug_set_fail(uint8_t fail);

static bt_status_t cmd_ota_error(void *no, uint16_t argc, char **argv)
{
    uint8_t fail;

    if (argc != 2) {
        return BT_STATUS_FAIL;
    }

    bt_mesh_app_util_str2u8HexNum(argv[1], &fail);

    bt_mesh_ota_updater_debug_set_fail(fail);
    return BT_STATUS_SUCCESS;
}

const mesh_app_callback_table_t cmds[] = {
    {"power", cmd_power},
    {"dump", cmd_dump},
    {"ota_error", cmd_ota_error},
    {"delete", cmd_delete},
    {"config", cmd_config},
    //{"iv", cmd_iv},
    {"bearer", cmd_bearer},
};

extern void bt_hci_log_enable(bool enable);
extern void bt_driver_trigger_controller_codedump(void);

bt_status_t mesh_app_io_callback(uint8_t len, char *param[])
{
    uint16_t i;
    if (len < 1) {
        LOG_I(mesh_app, "%s, invalid param\n", __FUNCTION__);
        return BT_STATUS_FAIL;
    }
    
    LOG_I(mesh_app, "%s name is %s, param len %d\n", __FUNCTION__, param[0], len);
    for (i = 0; i < sizeof(cmds) / sizeof(mesh_app_callback_table_t); i++) {
        if (!strcmp(cmds[i].name, param[0])) {
            return cmds[i].io_callback(NULL, (uint16_t)len, param);
        }
    }

    if (!strncmp(param[0], "hci", 3)) {
        if (!strncmp(param[1], "on", 2)) {
            bt_hci_log_enable(true);
        } else if (!strncmp(param[1], "off", 3)) {
            bt_hci_log_enable(false);
        }
    } else if (!strncmp(param[0], "dump_n9", 7)) {
        LOG_I(mesh_app, "start dump n9");
        bt_driver_trigger_controller_codedump();
        assert(0);
    } else if (!strncmp(param[0], "assert", 6)) {
        assert(0);
    } else {
        LOG_I(mesh_app, "%s cmd not found\n", __FUNCTION__);
    }

    return BT_STATUS_FAIL;
}
