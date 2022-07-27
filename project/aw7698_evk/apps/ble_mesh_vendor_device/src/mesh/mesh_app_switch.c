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
#include "FreeRTOS.h"
#include "queue.h"
#include "task.h"
#include "task_def.h"

#include "bt_type.h"
#include "bt_system.h"
#include "bt_gap_le.h"

#include "mesh_app_util.h"

#include "bt_mesh.h"
#include "bt_mesh_model_configuration_server.h"
#include "bt_mesh_model_configuration_client.h"
#include "bt_mesh_model_health_server.h"
#include "bt_mesh_model_health_client.h"
#include "bt_mesh_model_generic_client.h"
#include "bt_mesh_model_lighting_client.h"

#include "bt_mesh_ota_distributor.h"
#include "bt_mesh_ota_initiator.h"

#include "mesh_app_switch_def.h"
#include "mesh_app_switch_mesh_msg.h"

#ifdef MTK_LED_ENABLE
#include "bsp_led.h"
#endif
#include "nvdm.h"

#define RELAY_ENABLE    (1)
#define PROVISIONER_ENABLE (1)
#define VENDOR_COMPANY_ID 0x0094
#define VENDOR_MODEL_ID1 0x002A
#define VENDOR_MODEL_ID2 0x002B
#define VENDOR_OPCODE_1 0xC1
#define VENDOR_OPCODE_2 0xC2

#define MESH_BYPASS_IO              3

/* GPIO settings */
#define MESH_GPIO_SWITCH_EVB_LED1         19
#define MESH_GPIO_SWITCH_DK_LED_GROUP1    20
#define MESH_GPIO_SWITCH_DK_LED_GROUP2    21
#define MESH_GPIO_SWITCH_MASK       ( (1<<MESH_GPIO_SWITCH_EVB_LED1) |(1<<MESH_GPIO_SWITCH_DK_LED_GROUP1)|(1<<MESH_GPIO_SWITCH_DK_LED_GROUP2)) //0x00000300

#define MESH_GPIO_LED1            22
#define MESH_GPIO_LED1_MASK       (1 << MESH_GPIO_LED1)
#define MESH_GPIO_LED2            23
#define MESH_GPIO_LED2_MASK       (1 << MESH_GPIO_LED2)

#define GPIO_INPUT      0
#define GPIO_OUTPUT     1

#define LED_SWITCH_R   0xA1
#define LED_SWITCH_G   0xA2
#define LED_ON          1
#define LED_OFF         0

#define BSP_LED0_PIN 0

#define PHASE_0   0
#define PHASE_1   1
#define PHASE_2   2
#define PHASE_3   3

/* Check mesh status settings */
#define MESH_CHECK_MAX 5
#define MESH_CHECK_TIMEOUT 500 // ms
#define MESH_CONFIGURE_RETRY_MAX 5
#define MESH_CONFIGURE_TIMEOUT 5000 // ms

#define SWITCH_PUBLISH_GROUP_ADDRESS_1    0xC001
#define SWITCH_PUBLISH_GROUP_ADDRESS_2    0xC002

/* Provisioner capability */
#define MESH_PROVISIONER_NODES_MAX  50

#define  MESH_PROVISION_SERVICE_UUID            0x1827
#define  MESH_PROXY_SERVICE_UUID                0x1828

#define MAX_NVDM_BLOCK_SIZE (36*MESH_PROVISIONER_NODES_MAX) //sizeof(switch_mesh_provisioned_node_data_t) = 36
static uint8_t temp_nvdm_buffer[MAX_NVDM_BLOCK_SIZE] = {0};
extern const bt_mesh_ota_distributor_new_firmware_t new_firmware;
#define MESH_LATENCYTEST_TIMER_ID (mesh_latency_test_timer)
#define MESH_PACKETLOSS_TIMER_ID (mesh_packet_loss_test_timer)
#define MESH_THROUGHPUT_TIMER_ID (mesh_throughput_test_timer)
#define MESH_RX_TIMER_ID (mesh_rx_timer)
#define MESH_THROUGHPUT_RX_TIMER_ID (mesh_throughput_rx_timer)
#define MESH_PACKETLOSS_RX_TIMER_ID (mesh_packet_loss_rx_timer)
#define MESH_SEND_NEXT_TIMER_ID (mesh_send_next_timer)
#define MESH_LATENCY_TX_TIMER_ID (mesh_latency_tx_timer)


#define VENDOR_OPCODE_3 0xC3    // for small packet lost test    
#define VENDOR_OPCODE_4 0xC4    // for large packet lost test
#define VENDOR_OPCODE_5 0xC5    // for small packet throughput test
#define VENDOR_OPCODE_6 0xC6    // for large packet throughput test
#define VENDOR_OPCODE_7 0xC7    // for large packet throughput test

#define MESH_RX_TIMEOUT 5000
#define MESH_TX_TIMEOUT 3000

#define ADDR_GROUP_MASK             0xc000
#define ADDR_GROUP_VALUE            0xc000
#define SEND_NEXT_TIMEOUT           200  //should large than resolution of timer

enum {
    STATE_IDLE,
    STATE_BUSY,
};

/******************************************************************************/
/* prototype                                                                  */
/******************************************************************************/
static void _configure_callback(TimerHandle_t handle);
static void _ack_callback(TimerHandle_t handle);
void _get_remote_info_timeout_callback(TimerHandle_t handle);
static void switch_led_on_off( uint8_t ucLed , bool bOnOff );
static void msg_generic_on_off_get(uint16_t dst_addr, uint8_t ttl);
static void msg_generic_on_off_set(uint16_t dst_addr, uint8_t ttl,
                                   uint8_t onOff, uint8_t tid, uint8_t transTime,
                                   uint8_t delay, bool reliable);
void msg_config_get_composition(uint16_t dst_addr);
void msg_config_add_appkey(uint16_t appkeyIdx, uint16_t netkeyIdx, uint8_t *appkey, uint16_t dst_addr);
void msg_config_bind_model(uint16_t element_addr, uint16_t appkeyidx, uint16_t dst_addr, uint32_t model_id);
static void _handle_composition_data(bt_mesh_composition_data_t *data);
static void _latencytest_timer_handler(TimerHandle_t handle);
static void switch_delete_prov_addr(uint16_t prov_current_addr);
void msg_config_node_reset(uint16_t dst_addr);
extern void bt_mesh_ota_distributor_set_unicast_missing_dispatch(bool enabled);
extern bt_mesh_access_msg_status_code_t bt_mesh_ota_initiator_bind_models_app(uint16_t msg_appkey_index);
extern bt_mesh_access_msg_status_code_t bt_mesh_ota_distributor_bind_models_app(uint16_t msg_appkey_index);

static void _throughput_check_rx(void);
static void _rx_timer_handler(TimerHandle_t idx);
static void _throughput_timer_handler(TimerHandle_t idx);
static void _latencytest_check_send(void);
static void _throughput_check_send(void);
static void _packetloss_check_send(void);
static void _latencytest_set_timer(uint32_t ms);
static void _packetloss_set_timer(uint32_t ms);
static void _throughput_set_timer(uint32_t ms);
static bool msg_op_send(uint8_t opcode, uint16_t dst_addr, uint8_t ttl);
static void _latencytest_release_timer(void);
static void _rx_timer_post_handler(TimerHandle_t idx);

static void _rx_release_timer(TimerHandle_t handle);
static void _packetloss_release_timer(void);
static void _throughput_release_timer(void);
static bool msg_packetloss_send(uint16_t dst_addr, uint8_t ttl);
static bool msg_throughput_send(uint16_t dst_addr, uint8_t ttl);
static bool _isGroupDst(void);
static void _latencytest_send_next_timer_handler(TimerHandle_t idx);
static void _latencytest_check_send_ex(void);

static uint8_t bt_mesh_wifi_service_time_changed = false;
	
enum {
    TLV_TYPE_UUID = 0,
    TLV_TYPE_NODE_0 = 10,
    TLV_TYPE_NODE_1 = 11,
    TLV_TYPE_NODE_2 = 12,
    TLV_TYPE_NODE_3 = 13,
    TLV_TYPE_NODE_4 = 14,
    TLV_TYPE_NODE_5 = 15,
    TLV_TYPE_NODE_6 = 16,
    TLV_TYPE_NODE_7 = 17,
    TLV_TYPE_NODE_8 = 18,
    TLV_TYPE_NODE_9 = 19,
    TLV_TYPE_NODE_10 = 20,
    TLV_TYPE_NODE_11 = 21,
    TLV_TYPE_NODE_12 = 22,
    TLV_TYPE_NODE_13 = 23,
    TLV_TYPE_NODE_14 = 24,
    TLV_TYPE_NODE_15 = 25,
    TLV_TYPE_NODE_16 = 26,
    TLV_TYPE_NODE_17 = 27,
    TLV_TYPE_NODE_18 = 28,
    TLV_TYPE_NODE_19 = 29,
    TLV_TYPE_NODE_20 = 30,
    TLV_TYPE_NODE_21 = 31,
    TLV_TYPE_NODE_22 = 32,
    TLV_TYPE_NODE_23 = 33,
    TLV_TYPE_NODE_24 = 34,
    TLV_TYPE_NODE_25 = 35,
    TLV_TYPE_NODE_26 = 36,
    TLV_TYPE_NODE_27 = 37,
    TLV_TYPE_NODE_28 = 38,
    TLV_TYPE_NODE_29 = 39, /* TLV_TYPE_NODE_0 + MESH_PROVISIONER_NODES_MAX - 1 */
};

enum {
    SWITCH_SETTING_STATE_IDLE,
    SWITCH_SETTING_STATE_PROVISIONING,
    SWITCH_SETTING_STATE_PROVISIONED,
    SWITCH_SETTING_STATE_GET_COMPOSITION,
    SWITCH_SETTING_STATE_GET_COMPOSITION_DONE,
    SWITCH_SETTING_STATE_ADD_APPKEY,
    SWITCH_SETTING_STATE_ADD_APPKEY_DONE,
    SWITCH_SETTING_STATE_BIND_MODEL,
    SWITCH_SETTING_STATE_BIND_MODEL_DONE,
    SWITCH_SETTING_STATE_DONE,
};

enum {
    SWITCH_NODE_STATE_NORMAL = 0,
    SWITCH_NODE_STATE_RESETING = 1,
};

typedef struct {
    uint16_t address;
    uint8_t devkey[BT_MESH_DEVKEY_SIZE];
    uint8_t uuid[BT_MESH_UUID_SIZE];
    uint8_t state;
} switch_mesh_provisioned_node_data_t;

typedef struct {
    bt_addr_t          addr;
    int8_t              rssi;
    bt_mesh_gatt_service_t type;
    uint8_t            *data;
    uint8_t             data_len;
} connectabt_mesh_device_t;
/******************************************************************************/
/* global variables                                                           */
/******************************************************************************/
static uint8_t gSwitchState = SWITCH_SETTING_STATE_IDLE;
static int8_t gConfigureRetry = MESH_CONFIGURE_RETRY_MAX;

// OTA
//static bool mesh_firmware_updating = false;

// LED
static uint8_t ledPhase = 0;

// switch information
static uint8_t deviceUuid[BT_MESH_UUID_SIZE] = {
    0x00, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88, 0x99, 0xaa, 0xbb, 0xcc, 0xdd, 0xee, 0xff
};

static uint32_t gAutoCount = 100;
static uint32_t gTimes = 100;
static uint32_t gAutoTimeout = 1000;
static uint32_t gIsSmall = true;
static uint8_t gAutoTtl = 4;
static uint16_t gLatencyDst = 0;
static uint32_t gAvgTimes = 10;
static uint16_t gCrtCnt = 0;
static uint32_t gAvgThroughput = 0;
static uint32_t gNumDst = 1;
static uint8_t gCrtNumDst = 1;
static uint32_t gRxTimeout = MESH_RX_TIMEOUT;
static uint32_t gTxTimeout = MESH_TX_TIMEOUT;
static uint8_t gState = STATE_IDLE;
static bool gLatencyRspCmp = false;

static TimerHandle_t mesh_bat_timer = NULL; /**< Timer handler. */
static TimerHandle_t mesh_switch_timer = NULL; /**< Timer handler. */
static TimerHandle_t mesh_config_timer = NULL; /**< Timer handler. */
static TimerHandle_t mesh_switch_led_timer = NULL; /**< Timer handler. */
static TimerHandle_t mesh_latency_test_timer = NULL; /**< Timer handler. */
static TimerHandle_t mesh_packet_loss_test_timer = NULL; /**< Timer handler. */
static TimerHandle_t mesh_throughput_test_timer = NULL; /**< Timer handler. */
static TimerHandle_t mesh_rx_timer = NULL; /**< Timer handler. */
static TimerHandle_t mesh_throughput_rx_timer = NULL; /**< Timer handler. */
static TimerHandle_t mesh_packet_loss_rx_timer = NULL; /**< Timer handler. */
static TimerHandle_t mesh_send_next_timer = NULL; /**< Timer handler. */
static TimerHandle_t mesh_latency_tx_timer = NULL; /**< Timer handler. */

TimerHandle_t mesh_get_remote_info_timer = NULL; /**< Timer handler. */
TimerHandle_t mesh_node_reset_timer = NULL; /**< Timer handler. */
static TimerHandle_t wifi_service_change_timer = NULL; /**< Timer handler. */

static void *gMeshDeviceQueue = NULL; // connectabt_mesh_device_t

// device provisioning
uint16_t g_appkey_index = 0x123;
static uint8_t g_application_key[BT_MESH_APPKEY_SIZE] = {
    0x63, 0x96, 0x47, 0x71, 0x73, 0x4f, 0xbd, 0x76, 0xe3, 0xb4, 0x05, 0x19, 0xd1, 0xd9, 0x4a, 0x48
};
static uint8_t g_device_key[BT_MESH_DEVKEY_SIZE] = {0};
static uint16_t g_prov_current_netidx = BT_MESH_GLOBAL_PRIMARY_NETWORK_KEY_INDEX;
uint16_t g_prov_current_addr = 0;
static uint8_t g_prov_current_uuid[BT_MESH_UUID_SIZE];

static bt_mesh_prov_provisioner_params_t *g_start_params = NULL;
 
// TLV flash sector
const uint8_t __attribute__ ((section (".ap_sector"), aligned(4096))) tlv_data_sector[0x3000] = {
    [0x0000 ... 0x2FFF] = 0xFF,
};

// generic and reliable message check
static uint32_t gTid = 1;
static bool gIsChecking = false;
static uint8_t gCheckTid = 1;
static bool gCheckOnOff = false;
static uint8_t gCheckTtl = 0;
static uint8_t gCheckCount = 0;

// element index
uint16_t g_config_client_element_index = MESH_MODEL_INVALID_ELEMENT_INDEX;
static void *binding_model_list = NULL; // AB_queue of switch_mesh_binding_model_t

// model handle
static uint16_t g_model_handle_config_server;
uint16_t g_model_handle_health_server;
static uint16_t g_model_handle_config_client;
uint16_t g_model_handle_health_client;
uint16_t g_model_handle_onoff_client;
static uint16_t g_model_handle_level_client;
uint16_t g_model_handle_vendor;
static uint16_t g_model_handle_light_lightness_client;
static uint16_t g_model_handle_light_ctl_client;
static uint16_t g_model_handle_light_hsl_client;

static uint32_t gLatencyTick = 0;
static uint32_t gProvisioningTick = 0;
static uint32_t gLatencyTickAvg = 0;
static uint32_t gLatencyTickAvgHops = 0;
static uint32_t gLatencyTickIdx = 0;
static uint32_t gLatencyTickIdxHops = 0;

static uint32_t gSendCheckTick = 0;
static uint32_t gTxHopsIdx = 0;
static uint32_t gRxHopsIdx = 0;
static uint32_t gTxHopsAvg = 0;
static uint32_t gRxHopsAvg = 0;


#ifdef PROVISIONER_ENABLE
static bool g_provision_by_gatt = false;
#endif
/******************************************************************************/
/* callback functions                                                         */
/******************************************************************************/
static void _node_reset_retry_callback(TimerHandle_t handle)
{
    if (mesh_node_reset_timer != handle) {
        return;
    }

    nvdm_status_t status;
    uint32_t size = MAX_NVDM_BLOCK_SIZE;
    int8_t count = 0, i = 0;
    bool need_reset_timer = false;
    
    status = nvdm_read_data_item("MESH_APP", "node_list", temp_nvdm_buffer, &size);
    if (NVDM_STATUS_OK != status) {
        LOG_I(mesh_app, "Failed to read node info, status %x\n", status);
        return;
    }

    gConfigureRetry--;
    
    count = size / sizeof(switch_mesh_provisioned_node_data_t);
    for (i = count - 1; i >= 0; i--) {
        switch_mesh_provisioned_node_data_t *node_entry = (switch_mesh_provisioned_node_data_t *)(temp_nvdm_buffer + i * sizeof(switch_mesh_provisioned_node_data_t));
        LOG_I(mesh_app, "[%d] addr 0x%04x, state 0x%04x, gConfigureRetry %d", i, node_entry->address, node_entry->state, gConfigureRetry);

        if (node_entry->state == SWITCH_NODE_STATE_RESETING) {
            if (gConfigureRetry > 0) {
                /* sending node reset message */
                msg_config_node_reset(node_entry->address);
                need_reset_timer = true;
            } else {
                uint16_t address = node_entry->address;
                /*ABCDEF, if need to delete D, step 1)change D to 0, step 2)memcpy EF to DE, ABCEFF, step 3) clear F (last one) to 0*/
                memset(node_entry, 0, sizeof(switch_mesh_provisioned_node_data_t));
                
                //if delete the last one, no need to memcpy
                if (count > 1 && i != count - 1) {
                    memcpy(temp_nvdm_buffer + i * sizeof(switch_mesh_provisioned_node_data_t),
                           temp_nvdm_buffer + (i + 1) * sizeof(switch_mesh_provisioned_node_data_t),
                           (count - i - 1) *sizeof(switch_mesh_provisioned_node_data_t));
                    //clear the last one
                    memset(temp_nvdm_buffer + (count - 1) * sizeof(switch_mesh_provisioned_node_data_t), 0, sizeof(switch_mesh_provisioned_node_data_t));
                }
                count -= 1;
                
                if (count != 0) {
                    status = nvdm_write_data_item("MESH_APP", "node_list", NVDM_DATA_ITEM_TYPE_RAW_DATA, temp_nvdm_buffer, count * sizeof(switch_mesh_provisioned_node_data_t));
                } else {
                    status = nvdm_delete_data_item("MESH_APP", "node_list");
                }
                
                if (NVDM_STATUS_OK != status) {
                    LOG_I(mesh_app,"[%d] force to delete failed, count 0x%x\n", status, count);
                } else {
                    bt_mesh_config_delete_device_key(address);
                    LOG_I(mesh_app,"force to delete 0x%04x done.\n", address);
                }
            }
        }
    }
    if (need_reset_timer) {
        bt_mesh_app_util_start_timer(mesh_node_reset_timer, 1000);
    }
}

static void _configure_callback(TimerHandle_t handle)
{
    LOG_I(mesh_app, "_configure_callback timer_id 0x%x, state %d\n", handle, gSwitchState);

    switch (gSwitchState) {
        case SWITCH_SETTING_STATE_GET_COMPOSITION: {
            gSwitchState = SWITCH_SETTING_STATE_PROVISIONED;
            msg_config_get_composition(g_prov_current_addr);
            break;
        }
        case SWITCH_SETTING_STATE_ADD_APPKEY: {
            gSwitchState = SWITCH_SETTING_STATE_GET_COMPOSITION_DONE;
            msg_config_add_appkey(g_appkey_index, g_prov_current_netidx,
                                  g_application_key, g_prov_current_addr);
            break;
        }
        case SWITCH_SETTING_STATE_BIND_MODEL: {
            gSwitchState = SWITCH_SETTING_STATE_ADD_APPKEY_DONE;
            /* retry binding model and application key */
            switch_mesh_binding_model_t *entry = bt_mesh_os_layer_ds_queue_first(binding_model_list);
            if (entry != NULL) {
                msg_config_bind_model(
                    g_prov_current_addr + entry->element_index,
                    g_appkey_index, g_prov_current_addr, entry->model_id);
            } else {
                LOG_I(mesh_app_must, "no more model, all model binding is done\n");
            }
            break;
        }
        default: {
            LOG_I(mesh_app, "wrong state %d\n", gSwitchState);
            break;
        }
    }
}

static void _ack_callback(TimerHandle_t handle)
{
    LOG_I(mesh_app, "_ack_callback %d, resend Tid:%x OnOff:%d\n", gIsChecking, gCheckTid, gCheckOnOff);
    if (gIsChecking && gCheckCount < MESH_CHECK_MAX) {
        msg_generic_on_off_set(BT_MESH_ADDR_GROUP_NODES_VALUE, gCheckTtl, gCheckOnOff, gCheckTid, 0, 0, true);
        bt_mesh_app_util_start_timer(mesh_switch_timer, MESH_CHECK_TIMEOUT);
        gCheckCount++;
    }
}

/*static void _mesh_ota_cb(OTA_STATUS_ENUM status)
{
    mesh_firmware_updating = false;	//thus OTA app may reset the timer in case the OTA time too long

    switch (status) {
        case OTA_STARTED:
            LOG_I(mesh_app,"OTA started!!!\n");
            mesh_firmware_updating = true;
            break;
        case OTA_FINISHED:
            LOG_I(mesh_app,"OTA finised!!!\n");
            mesh_firmware_updating = false;
            break;
        case OTA_APPLY:
            printf("OTA applied, need to reboot.\n");
            AB_PWR_Rst();
            break;
        case OTA_ACCESSED_WHILE_DISABLED:
            printf("OTA accessed while disabled.\n");
            break;
    }
}*/

static void wifi_service_change_callback(TimerHandle_t handle)
{
    LOG_I(mesh_app, "wifi_service_change_callback\n");
	bt_mesh_app_util_change_wifi_service_time(20);
}

static void mesh_change_ble_wifi_schedule()
{
    if (!bt_mesh_wifi_service_time_changed) {
		bt_mesh_app_util_change_wifi_service_time(30);
		bt_mesh_app_util_start_timer(wifi_service_change_timer, 100);				
		bt_mesh_wifi_service_time_changed = true;
	} else {
		LOG_W(mesh_app, "ble wifi schedule changed before\n");
	}
}

static void _switch_led_timer_cb(TimerHandle_t handle)
{
    switch ( ledPhase++ ) {
        case PHASE_0: {
            switch_led_on_off( LED_SWITCH_R, LED_ON );
            switch_led_on_off( LED_SWITCH_G, LED_ON );
            break;
        }
        case PHASE_1: {
            switch_led_on_off( LED_SWITCH_R, LED_OFF );
            switch_led_on_off( LED_SWITCH_G, LED_OFF );
            break;
        }
        case PHASE_2: {
            switch_led_on_off( LED_SWITCH_R, LED_ON );
            switch_led_on_off( LED_SWITCH_G, LED_ON );
            break;
        }
        case PHASE_3: {
            switch_led_on_off( LED_SWITCH_R, LED_OFF );
            //switch_led_on_off( LED_SWITCH_G, LED_OFF );
            bt_mesh_app_util_stop_timer(mesh_switch_led_timer);
            break;
        }
    }


}

static void _handle_composition_data(bt_mesh_composition_data_t *data)
{
    LOG_I(mesh_app, "CID: 0x%04x\n", data->cid);
    LOG_I(mesh_app, "PID: 0x%04x\n", data->pid);
    LOG_I(mesh_app, "VID: 0x%04x\n", data->vid);
    LOG_I(mesh_app, "CRPL: 0x%04x\n", data->crpl);
    LOG_I(mesh_app, "Features: 0x%04x\n", data->features);
    if (binding_model_list != NULL) {
        bt_mesh_os_layer_ds_queue_free(binding_model_list);
    }
    binding_model_list = bt_mesh_os_layer_ds_queue_alloc();

    if (data->elements != NULL && bt_mesh_os_layer_ds_dlist_count(data->elements) > 0) {
        switch_mesh_binding_model_t *model_entry = NULL;
        uint32_t i, j = 0;
        bt_mesh_composition_element_t *entry = (bt_mesh_composition_element_t *)bt_mesh_os_layer_ds_dlist_first(data->elements);
        while (entry != NULL) {
            LOG_I(mesh_app, "    Element#%d, LOC: 0x%04x\n", j, entry->loc);
            LOG_I(mesh_app, "\tNumS: %d {", entry->num_s);
            if (entry->num_s > 0) {
                for (i = 0; i < entry->num_s ; i++) {
                    if (entry->sig_models[i] != BT_MESH_MODEL_SIG_MODEL_ID_CONFIGURATION_SERVER) {
                        model_entry = bt_mesh_os_layer_ds_queue_entry_alloc(sizeof(switch_mesh_binding_model_t));
                        model_entry->element_index = j;
                        model_entry->model_id = entry->sig_models[i];
                        bt_mesh_os_layer_ds_queue_push(binding_model_list, model_entry);
                        LOG_I(mesh_app, "%04x, ", entry->sig_models[i]);
                    }
                }
            }
            LOG_I(mesh_app, "}\n");
            LOG_I(mesh_app, "\tNumV: %d {", entry->num_v);
            if (entry->num_v > 0) {
                for (i = 0; i < entry->num_v ; i++) {
                    model_entry = bt_mesh_os_layer_ds_queue_entry_alloc(sizeof(switch_mesh_binding_model_t));
                    model_entry->element_index = j;
                    model_entry->model_id = entry->vendor_models[i];
                    bt_mesh_os_layer_ds_queue_push(binding_model_list, model_entry);
                    LOG_I(mesh_app, "%08lx, ", entry->vendor_models[i]);
                }
            }
            LOG_I(mesh_app, "}\n");
            entry = bt_mesh_os_layer_ds_dlist_next(data->elements, (void *)entry);
            j++;
        }
    }
}
/******************************************************************************/
/* device key management                                                      */
/******************************************************************************/
static void switch_load_prov_addr(void)
{
    uint8_t i = 0;
    nvdm_status_t status;
    uint32_t size = MAX_NVDM_BLOCK_SIZE;

    status = nvdm_read_data_item("MESH_APP", "node_list", temp_nvdm_buffer, &size);
    if (NVDM_STATUS_OK != status) {
        LOG_I(mesh_app, "Failed to read node info, status %x\n", status);
        return;
    }
    
    for (i = 0; i < size / sizeof(switch_mesh_provisioned_node_data_t); i++) {
        switch_mesh_provisioned_node_data_t *node_entry = (switch_mesh_provisioned_node_data_t *)bt_mesh_app_util_alloc(sizeof(switch_mesh_provisioned_node_data_t));
        memcpy(node_entry, temp_nvdm_buffer + i * sizeof(switch_mesh_provisioned_node_data_t), sizeof(switch_mesh_provisioned_node_data_t));
        //bt_mesh_os_layer_ds_dlist_insert(g_node_list, node_entry);
        LOG_I(mesh_app, "[%d:%d] 0x%04x, ", i, node_entry->state, node_entry->address);
        SWITCH_DEBUG_DUMP_PDU("uuid", BT_MESH_UUID_SIZE, node_entry->uuid);
        SWITCH_DEBUG_DUMP_PDU("devkey", BT_MESH_UUID_SIZE, node_entry->devkey);

        /* add into mesh stack */
        bt_mesh_config_add_device_key(node_entry->devkey, node_entry->address);
        bt_mesh_app_util_free((uint8_t *)node_entry);
    }
}

static void switch_delete_prov_addr(uint16_t prov_current_addr)
{
    uint8_t i = 0;
    nvdm_status_t status;
    uint32_t size = MAX_NVDM_BLOCK_SIZE;
    uint8_t count = 0;
    bool need_start_time = false;

    status = nvdm_read_data_item("MESH_APP", "node_list", temp_nvdm_buffer, &size);
    if (NVDM_STATUS_OK != status) {
        LOG_I(mesh_app, "Failed to read node info, status %x\n", status);
        return;
    }
    count = size / sizeof(switch_mesh_provisioned_node_data_t);
    gConfigureRetry = MESH_CONFIGURE_RETRY_MAX;
    for (i = 0; i < count; i++) {
        switch_mesh_provisioned_node_data_t *node_entry = (switch_mesh_provisioned_node_data_t *)(temp_nvdm_buffer + i * sizeof(switch_mesh_provisioned_node_data_t));
        LOG_I(mesh_app, "[%d] 0x%04x, ", i, node_entry->address);
        SWITCH_DEBUG_DUMP_PDU("devkey", BT_MESH_DEVKEY_SIZE, node_entry->devkey);
        if (prov_current_addr == BT_MESH_ADDR_GROUP_NODES_VALUE || node_entry->address == prov_current_addr) {
            node_entry->state = SWITCH_NODE_STATE_RESETING;
            /* sending node reset message */
            msg_config_node_reset(node_entry->address);
            need_start_time = true;
        }
    }
    if (need_start_time) {
        bt_mesh_app_util_start_timer(mesh_node_reset_timer, 1000);
        //update state
        status = nvdm_write_data_item("MESH_APP", "node_list", NVDM_DATA_ITEM_TYPE_RAW_DATA, temp_nvdm_buffer, size);
        if (NVDM_STATUS_OK != status) {
            LOG_I(mesh_app, "Failed to update node info, status %x.\n", status);
        } else {
            LOG_I(mesh_app, "Successfully update state to NVDM");
        }
    }
}

static void switch_store_prov_addr( uint16_t prov_current_addr, uint8_t *p_device_key, uint8_t *p_uuid)
{
    nvdm_status_t status;
    uint32_t size = MAX_NVDM_BLOCK_SIZE;

    /* delete previous record, will cause node reset */
    switch_delete_prov_addr(prov_current_addr);

    switch_mesh_provisioned_node_data_t *node_entry = (switch_mesh_provisioned_node_data_t *)bt_mesh_app_util_alloc(sizeof(switch_mesh_provisioned_node_data_t));
    node_entry->address = prov_current_addr;
    memcpy(node_entry->devkey, p_device_key, BT_MESH_DEVKEY_SIZE);
    memcpy(node_entry->uuid, p_uuid, BT_MESH_UUID_SIZE);
    node_entry->state = SWITCH_NODE_STATE_NORMAL;

    status = nvdm_read_data_item("MESH_APP", "node_list", temp_nvdm_buffer, &size);
    if (NVDM_STATUS_OK != status) {
        LOG_I(mesh_app, "Failed to read node info, addr %x, status %x.\n", node_entry->address, status);
    }
    if (size + sizeof(switch_mesh_provisioned_node_data_t) > MAX_NVDM_BLOCK_SIZE) {
        LOG_I(mesh_app, "can't be stored because reach 1 mesh app NVDM block size.\n", node_entry->address, status);
        return;
    }
    memcpy(temp_nvdm_buffer + size, node_entry, sizeof(switch_mesh_provisioned_node_data_t));
    status = nvdm_write_data_item("MESH_APP", "node_list", NVDM_DATA_ITEM_TYPE_RAW_DATA, temp_nvdm_buffer, size + sizeof(switch_mesh_provisioned_node_data_t));
    if (NVDM_STATUS_OK != status) {
        LOG_I(mesh_app, "Failed to store node info, status %x.\n", status);
    } else {
        LOG_I(mesh_app, "Successfully store node info to NVDM, addr %x", node_entry->address);
    }
    bt_mesh_app_util_free((uint8_t *)node_entry);
}

/******************************************************************************/
/* mesh messages                                                              */
/******************************************************************************/
static void msg_vendor(uint16_t company_id, uint8_t opcode, uint16_t dst_addr, uint16_t datalen)
{
    bt_mesh_tx_params_t param;
    param.dst.value = dst_addr;
    param.dst.type = bt_mesh_utils_get_addr_type(param.dst.value);
    param.src = bt_mesh_model_get_element_address(0);
    param.ttl = bt_mesh_config_get_default_ttl();  // default TTL value
    param.data_len = 3 + datalen; // total length of this packet

    if (param.data_len > 380) {
        param.data_len = 380;
        datalen = 377;
    }

    // set packet buffer, please refer to Mesh Profile Specification 3.7.3
    uint16_t i = 0;
    uint8_t *payload = bt_mesh_app_util_alloc(param.data_len);
    payload[0] = opcode;
    payload[1] = company_id & 0x00FF;
    payload[2] = (company_id & 0xFF00) >> 8;
    for(i = 0; i < datalen ; i++) {
        payload[3+i] = i;
    }

    param.data = payload;
    param.security.appidx = g_appkey_index; // which application key used to send
    param.security.device_key = NULL;
    bt_mesh_status_t ret = bt_mesh_send_packet(&param);
    bt_mesh_app_util_free(payload);

    LOG_I(mesh_app, "[T]Vendor_msg={%02x:%04x, dst=0x%04x}, result:%d\n", opcode, company_id, dst_addr, ret);
}

/*Application messages start*/
static void msg_generic_on_off_get(uint16_t dst_addr, uint8_t ttl)
{
    bt_mesh_access_message_tx_meta_t tx_meta = {{0}};
    tx_meta.dst_addr.value = dst_addr;
    tx_meta.dst_addr.type = bt_mesh_utils_get_addr_type(dst_addr);
    tx_meta.appkey_index = g_appkey_index;
    tx_meta.ttl = ttl;
    tx_meta.dst_addr.virtual_uuid = NULL;
    bt_mesh_status_t ret = bt_mesh_model_generic_onoff_client_get(g_model_handle_onoff_client, &tx_meta);
    LOG_I(mesh_app, "[T]Generic_OnOffGet={dst=0x%04x}, result:%d\n", dst_addr, ret);
}

static void msg_generic_on_off_set_uuid(uint8_t *uuid, uint8_t ttl,
                                   uint8_t onOff, uint8_t tid, uint8_t transTime,
                                   uint8_t delay, bool reliable)
{
    bt_mesh_access_message_tx_meta_t tx_meta = {{0}};
    tx_meta.dst_addr.value = bt_mesh_utils_get_virtual_address(uuid);
    tx_meta.dst_addr.type = BT_MESH_ADDRESS_TYPE_VIRTUAL;
    tx_meta.dst_addr.virtual_uuid = uuid;
    tx_meta.appkey_index = g_appkey_index;
    tx_meta.ttl = ttl;
    
    uint8_t buf[4];
    buf[0] = onOff ? 1 : 0;
    buf[1] = gTid++;
    buf[2] = 0;
    buf[3] = 0;
    
    bt_status_t ret = bt_mesh_model_generic_onoff_client_set(g_model_handle_onoff_client, &tx_meta,
                                 buf, 4, reliable);
    
    LOG_I(mesh_app,"[T][virtual]Generic_OnOffSet={dst=0x%04x, onoff=0x%02x, tid=0x%02x, transTime=0x%02x, delay=0x%02x}, reliable=%s, result:%d\n",
           tx_meta.dst_addr.value, onOff, tid, transTime, delay, reliable ? "true" : "false", ret);

}

static void msg_generic_on_off_set(uint16_t dst_addr, uint8_t ttl,
                                   uint8_t onOff, uint8_t tid, uint8_t transTime,
                                   uint8_t delay, bool reliable)
{
    bt_mesh_access_message_tx_meta_t tx_meta = {{0}};
    tx_meta.dst_addr.value = dst_addr;
    tx_meta.dst_addr.type = bt_mesh_utils_get_addr_type(dst_addr);
    tx_meta.appkey_index = g_appkey_index;
    tx_meta.ttl = ttl;
    tx_meta.dst_addr.virtual_uuid = NULL;

    uint8_t buf[4];
    buf[0] = onOff ? 1 : 0;
    buf[1] = tid;
    buf[2] = transTime;
    buf[3] = delay;

    bt_mesh_status_t ret = bt_mesh_model_generic_onoff_client_set(g_model_handle_onoff_client, &tx_meta,
                                buf, 4, reliable);

    LOG_I(mesh_app, "[T]Generic_OnOffSet={dst=0x%04x, onoff=0x%02x, tid=0x%02x, transTime=0x%02x, delay=0x%02x}, reliable=%s, result:%d\n",
          dst_addr, onOff, tid, transTime, delay, reliable ? "true" : "false", ret);
}

static void msg_generic_level_get(uint16_t dst_addr, uint8_t ttl)
{
    bt_mesh_access_message_tx_meta_t tx_meta = {{0}};
    tx_meta.dst_addr.value = dst_addr;
    tx_meta.dst_addr.type = bt_mesh_utils_get_addr_type(dst_addr);
    tx_meta.appkey_index = g_appkey_index;
    tx_meta.ttl = ttl;
    tx_meta.dst_addr.virtual_uuid = NULL;

    bt_mesh_status_t ret = bt_mesh_model_generic_level_client_get(g_model_handle_level_client, &tx_meta);
    LOG_I(mesh_app, "[T]Generic_LevelGet={dst=0x%04x}, result:%d\n", dst_addr, ret);
}

static void msg_generic_level_set(uint16_t dst_addr, uint8_t ttl,
                                  int16_t level, uint8_t tid, uint8_t transTime,
                                  uint8_t delay, bool reliable)
{
    bt_mesh_access_message_tx_meta_t tx_meta = {{0}};
    tx_meta.dst_addr.value = dst_addr;
    tx_meta.dst_addr.type = bt_mesh_utils_get_addr_type(dst_addr);
    tx_meta.appkey_index = g_appkey_index;
    tx_meta.ttl = ttl;
    tx_meta.dst_addr.virtual_uuid = NULL;

    uint8_t buf[5];
    memcpy(buf, &level, 2);
    buf[2] = tid;
    buf[3] = transTime;
    buf[4] = delay;

    bt_mesh_status_t ret = bt_mesh_model_generic_level_client_set(g_model_handle_level_client, &tx_meta,
                                buf, 5, reliable);

    LOG_I(mesh_app, "[T]Generic_LevelSet={dst=0x%04x, level=%d, tid=0x%02x, transTime=0x%02x, delay=0x%02x}, reliable=%s, result:%d\n",
          dst_addr, level, tid, transTime, delay, reliable ? "true" : "false", ret);
}

/*Application messages end*/



/* SIG config messages start */
void msg_config_get_composition(uint16_t dst_addr)
{
    if (gSwitchState == SWITCH_SETTING_STATE_PROVISIONED) {
        if (gConfigureRetry > 0) {
            bt_mesh_status_t ret = bt_mesh_model_configuration_client_get_composition_data(
                                            0xff,
                                            bt_mesh_model_get_element_address(g_config_client_element_index),
                                            dst_addr,
                                            bt_mesh_config_get_default_ttl(),
                                            BT_MESH_GLOBAL_PRIMARY_NETWORK_KEY_INDEX
                                        );
            LOG_I(mesh_app, "ret: %d\n", ret);
            if (ret == 0) {
                gConfigureRetry--;
                gSwitchState = SWITCH_SETTING_STATE_GET_COMPOSITION;
                bt_mesh_app_util_start_timer(mesh_config_timer, MESH_CONFIGURE_TIMEOUT);
                if(gConfigureRetry == 1) {
                    mesh_change_ble_wifi_schedule();
                }
            } else {
                LOG_I(mesh_app, "error!\n");
            }
        } else {
            LOG_I(mesh_app, "stop retry get composition data\n");
            gSwitchState = SWITCH_SETTING_STATE_PROVISIONED;
        }
    } else {
        LOG_I(mesh_app, "wrong state %d\n", gSwitchState);
    }
}


void msg_config_add_appkey(uint16_t appkeyIdx, uint16_t netkeyIdx, uint8_t *appkey, uint16_t dst_addr)
{
    if (gSwitchState == SWITCH_SETTING_STATE_GET_COMPOSITION_DONE) {
        if (gConfigureRetry > 0) {
            bt_mesh_status_t ret = bt_mesh_model_configuration_client_add_app_key(
                                            appkeyIdx, netkeyIdx, appkey,
                                            bt_mesh_model_get_element_address(g_config_client_element_index),
                                            dst_addr,
                                            bt_mesh_config_get_default_ttl(),
                                            BT_MESH_GLOBAL_PRIMARY_NETWORK_KEY_INDEX
                                        );
            LOG_I(mesh_app, "ret: %d\n", ret);
            if (ret == 0) {
                gConfigureRetry--;
                gSwitchState = SWITCH_SETTING_STATE_ADD_APPKEY;
                bt_mesh_app_util_start_timer(mesh_config_timer, MESH_CONFIGURE_TIMEOUT);
                if(gConfigureRetry == 1) {
                    mesh_change_ble_wifi_schedule();
                }
            } else {
                LOG_I(mesh_app, "error!\n");
            }
        } else {
            LOG_I(mesh_app, "stop retry add appkey\n");
            gSwitchState = SWITCH_SETTING_STATE_GET_COMPOSITION_DONE;
        }
    } else {
        LOG_I(mesh_app, "wrong state %d\n", gSwitchState);
    }
}


void msg_config_bind_model(uint16_t element_addr, uint16_t appkeyidx, uint16_t dst_addr, uint32_t model_id)
{
    if (gSwitchState == SWITCH_SETTING_STATE_ADD_APPKEY_DONE) {
        if (gConfigureRetry > 0) {
            bt_mesh_status_t ret = bt_mesh_model_configuration_client_bind_model_app(
                                            element_addr,
                                            appkeyidx,
                                            model_id,
                                            bt_mesh_model_get_element_address(g_config_client_element_index),
                                            dst_addr,
                                            bt_mesh_config_get_default_ttl(),
                                            BT_MESH_GLOBAL_PRIMARY_NETWORK_KEY_INDEX
                                        );
            LOG_I(mesh_app, "binding other_server_model:0x%04x\n", model_id);
            LOG_I(mesh_app, "ret: %d\n", ret);
            gConfigureRetry--;
            gSwitchState = SWITCH_SETTING_STATE_BIND_MODEL;
            bt_mesh_app_util_start_timer(mesh_config_timer, MESH_CONFIGURE_TIMEOUT);
			if(gConfigureRetry == 1) {
				mesh_change_ble_wifi_schedule();
			}
        } else  {
            LOG_I(mesh_app, "stop retry binding other server_model\n");
            gSwitchState = SWITCH_SETTING_STATE_ADD_APPKEY_DONE;
        }
    } else {
        LOG_I(mesh_app, "wrong state %d\n", gSwitchState);
    }
}


void msg_config_node_reset(uint16_t dst_addr)
{
    bt_mesh_status_t ret = bt_mesh_model_configuration_client_reset_node(
                                    bt_mesh_model_get_element_address(g_config_client_element_index),
                                    dst_addr,
                                    bt_mesh_config_get_default_ttl(),
                                    BT_MESH_GLOBAL_PRIMARY_NETWORK_KEY_INDEX);

    LOG_I(mesh_app, "[T]Config_NodeReset={dst=0x%04x}, result:%d\n", dst_addr, ret);
}

void msg_config_handle_subscription(uint8_t action, uint16_t dst_addr,
                                    uint16_t element_addr, uint16_t sub_addr, const uint8_t *uuid, uint32_t model_id)
{
    bt_mesh_status_t ret;
    bt_mesh_address_t subscribe_addr;
    subscribe_addr.type = bt_mesh_utils_get_addr_type(sub_addr);
    subscribe_addr.value = sub_addr;
    subscribe_addr.virtual_uuid = uuid;
    switch (action) {
        // add
        case 0:  {
            ret = bt_mesh_model_configuration_client_add_model_subscription(
                      element_addr,
                      subscribe_addr,
                      model_id,
                      bt_mesh_model_get_element_address(g_config_client_element_index),
                      dst_addr,
                      bt_mesh_config_get_default_ttl(),
                      BT_MESH_GLOBAL_PRIMARY_NETWORK_KEY_INDEX);
            break;
        }
        // delete
        case 1:  {
            ret = bt_mesh_model_configuration_client_delete_model_subscription(
                      element_addr,
                      subscribe_addr,
                      model_id,
                      bt_mesh_model_get_element_address(g_config_client_element_index),
                      dst_addr,
                      bt_mesh_config_get_default_ttl(),
                      BT_MESH_GLOBAL_PRIMARY_NETWORK_KEY_INDEX);
            break;
        }
        // delete all
        case 2: {
            ret = bt_mesh_model_configuration_client_delete_all_model_subscription(
                      element_addr,
                      model_id,
                      bt_mesh_model_get_element_address(g_config_client_element_index),
                      dst_addr,
                      bt_mesh_config_get_default_ttl(),
                      BT_MESH_GLOBAL_PRIMARY_NETWORK_KEY_INDEX);
            break;
        }
        // update
        case 3: {
            ret = bt_mesh_model_configuration_client_overwrite_model_subscription(
                      element_addr,
                      subscribe_addr,
                      model_id,
                      bt_mesh_model_get_element_address(g_config_client_element_index),
                      dst_addr,
                      bt_mesh_config_get_default_ttl(),
                      BT_MESH_GLOBAL_PRIMARY_NETWORK_KEY_INDEX);
            break;
        }
        default:
            ret = BT_MESH_ERROR_FAIL;
    }

    if (ret != BT_MESH_SUCCESS) {
        LOG_I(mesh_app, "result: %d\n", ret);
    }
}

void msg_config_set_publication(uint16_t dst_addr,
                                bt_mesh_model_publication_param_t *publish_params)
{
    bt_mesh_status_t ret;

    ret = bt_mesh_model_configuration_client_set_model_publication(
              publish_params,
              bt_mesh_model_get_element_address(g_config_client_element_index),
              dst_addr,
              bt_mesh_config_get_default_ttl(),
              BT_MESH_GLOBAL_PRIMARY_NETWORK_KEY_INDEX);

    if (ret != BT_MESH_SUCCESS) {
        LOG_I(mesh_app, "result: %d\n", ret);
    }
}

void msg_config_get_publication(uint16_t dst_addr,
                                uint16_t element_addr, uint32_t model_id)
{
    bt_mesh_status_t ret;

    ret = bt_mesh_model_configuration_client_get_model_publication(
              element_addr,
              model_id,
              bt_mesh_model_get_element_address(g_config_client_element_index),
              dst_addr,
              bt_mesh_config_get_default_ttl(),
              BT_MESH_GLOBAL_PRIMARY_NETWORK_KEY_INDEX);

    if (ret != BT_MESH_SUCCESS) {
        LOG_I(mesh_app, "result: %d\n", ret);
    }
}

void msg_config_set_heartbeat_publication(uint16_t dst_addr, 
    bt_mesh_heartbeat_publication_param_t *publish_params)
{
    bt_mesh_model_configuration_client_set_heartbeat_publication(publish_params,
       bt_mesh_model_get_element_address(g_config_client_element_index),
       dst_addr,
       bt_mesh_config_get_default_ttl(),
       BT_MESH_GLOBAL_PRIMARY_NETWORK_KEY_INDEX);
}

void msg_config_get_heartbeat_publication(uint16_t dst_addr)
{
    bt_mesh_model_configuration_client_get_heartbeat_publication(
       bt_mesh_model_get_element_address(g_config_client_element_index),
       dst_addr,
       bt_mesh_config_get_default_ttl(),
       BT_MESH_GLOBAL_PRIMARY_NETWORK_KEY_INDEX);
}

void msg_config_set_heartbeat_subscription(uint16_t dst_addr, 
    bt_mesh_heartbeat_subscription_param_t *subscribe_params)
{
    bt_mesh_model_configuration_client_set_heartbeat_subscription(subscribe_params,
       bt_mesh_model_get_element_address(g_config_client_element_index),
       dst_addr,
       bt_mesh_config_get_default_ttl(),
       BT_MESH_GLOBAL_PRIMARY_NETWORK_KEY_INDEX);
}

void msg_config_get_heartbeat_subscription(uint16_t dst_addr)
{
    bt_mesh_model_configuration_client_get_heartbeat_subscription(
       bt_mesh_model_get_element_address(g_config_client_element_index),
       dst_addr,
       bt_mesh_config_get_default_ttl(),
       BT_MESH_GLOBAL_PRIMARY_NETWORK_KEY_INDEX);
}

void msg_config_set_beacon(uint16_t dst_addr, bool enable)
{
    bt_mesh_status_t ret;

    ret = bt_mesh_model_configuration_client_set_beacon(
              enable ? BT_MESH_FEATURE_STATE_ENABLED : BT_MESH_FEATURE_STATE_DISABLED,
              bt_mesh_model_get_element_address(g_config_client_element_index),
              dst_addr,
              bt_mesh_config_get_default_ttl(),
              BT_MESH_GLOBAL_PRIMARY_NETWORK_KEY_INDEX);

    if (ret != BT_MESH_SUCCESS) {
        LOG_I(mesh_app, "result: %d\n", ret);
    }
}

void msg_config_set_relay(uint16_t dst_addr, bool enable, uint8_t count, uint8_t steps)
{
    bt_mesh_status_t ret;

    ret = bt_mesh_model_configuration_client_set_relay(
              enable ? BT_MESH_FEATURE_STATE_ENABLED : BT_MESH_FEATURE_STATE_DISABLED,
              count, steps,
              bt_mesh_model_get_element_address(g_config_client_element_index),
              dst_addr,
              bt_mesh_config_get_default_ttl(),
              BT_MESH_GLOBAL_PRIMARY_NETWORK_KEY_INDEX);

    if (ret != BT_MESH_SUCCESS) {
        LOG_I(mesh_app, "result: %d\n", ret);
    }
}

void msg_config_set_friend(uint16_t dst_addr, bool enable)
{
    bt_mesh_status_t ret;

    ret = bt_mesh_model_configuration_client_set_friend(
              enable ? BT_MESH_FEATURE_STATE_ENABLED : BT_MESH_FEATURE_STATE_DISABLED,
              bt_mesh_model_get_element_address(g_config_client_element_index),
              dst_addr,
              bt_mesh_config_get_default_ttl(),
              BT_MESH_GLOBAL_PRIMARY_NETWORK_KEY_INDEX);

    if (ret != BT_MESH_SUCCESS) {
        LOG_I(mesh_app, "result: %d\n", ret);
    }
}

void msg_config_set_ni(uint16_t dst_addr, uint16_t keyidx, bool enable)
{
    bt_mesh_status_t ret;

    ret = bt_mesh_model_configuration_client_set_node_identity(
              keyidx,
              enable ? BT_MESH_FEATURE_STATE_ENABLED : BT_MESH_FEATURE_STATE_DISABLED,
              bt_mesh_model_get_element_address(g_config_client_element_index),
              dst_addr,
              bt_mesh_config_get_default_ttl(),
              BT_MESH_GLOBAL_PRIMARY_NETWORK_KEY_INDEX);

    if (ret != BT_MESH_SUCCESS) {
        LOG_I(mesh_app, "result: %d\n", ret);
    }
}

void msg_config_set_proxy(uint16_t dst_addr, bool enable)
{
    bt_mesh_status_t ret;

    ret = bt_mesh_model_configuration_client_set_gatt_proxy(
              enable ? BT_MESH_FEATURE_STATE_ENABLED : BT_MESH_FEATURE_STATE_DISABLED,
              bt_mesh_model_get_element_address(g_config_client_element_index),
              dst_addr,
              bt_mesh_config_get_default_ttl(),
              BT_MESH_GLOBAL_PRIMARY_NETWORK_KEY_INDEX);

    if (ret != BT_MESH_SUCCESS) {
        LOG_I(mesh_app, "result: %d\n", ret);
    }
}

void msg_config_set_default_ttl(uint16_t dst_addr, uint8_t ttl)
{
    bt_mesh_status_t ret;

    ret = bt_mesh_model_configuration_client_set_default_ttl(
              ttl,
              bt_mesh_model_get_element_address(g_config_client_element_index),
              dst_addr,
              bt_mesh_config_get_default_ttl(),
              BT_MESH_GLOBAL_PRIMARY_NETWORK_KEY_INDEX);

    if (ret != BT_MESH_SUCCESS) {
        LOG_I(mesh_app, "result: %d\n", ret);
    }
}

void msg_config_set_network_transmit(uint16_t dst_addr, uint8_t count, uint8_t interval_steps)
{
    bt_mesh_status_t ret;

    ret = bt_mesh_model_configuration_client_set_network_transmit(
              count, interval_steps,
              bt_mesh_model_get_element_address(g_config_client_element_index),
              dst_addr,
              bt_mesh_config_get_default_ttl(),
              BT_MESH_GLOBAL_PRIMARY_NETWORK_KEY_INDEX);

    if (ret != BT_MESH_SUCCESS) {
        LOG_I(mesh_app, "result: %d\n", ret);
    }
}

/* SIG config messages end */
/* Callback */
extern void mesh_app_get_remote_info_handler(uint16_t model_handle,
        const bt_mesh_access_message_rx_t *msg, const config_client_evt_t *event);
static void _config_client_msg_handler(uint16_t model_handle,
                                       const bt_mesh_access_message_rx_t *msg, const config_client_evt_t *event)
{

    SWITCH_DEBUG_EVT_SIG(msg->opcode.opcode);
    bt_mesh_app_util_display_hex_log(&msg->buffer[0], msg->length);
    mesh_app_get_remote_info_handler(model_handle, msg, event);

    switch (event->evt_id) {
#ifdef PROVISIONER_ENABLE
        case CONFIG_CLIENT_EVT_COMPOSITION_DATA_STATUS: {
            SWITCH_DEBUG_EVT_SRC("ConfigCompositionDataStatus", msg->meta_data.src_addr);
            if (gSwitchState == SWITCH_SETTING_STATE_PROVISIONED || gSwitchState == SWITCH_SETTING_STATE_GET_COMPOSITION) {
                bt_mesh_app_util_stop_timer(mesh_config_timer);
                if (binding_model_list != NULL) {
                    bt_mesh_os_layer_ds_queue_free(binding_model_list);
                    binding_model_list = NULL;
                }

                if (event->data.composition_data_status.composition_data != NULL) {
                    _handle_composition_data(event->data.composition_data_status.composition_data);
                }

                gSwitchState = SWITCH_SETTING_STATE_GET_COMPOSITION_DONE;
                gConfigureRetry = MESH_CONFIGURE_RETRY_MAX;
                msg_config_add_appkey(g_appkey_index, g_prov_current_netidx,
                                      g_application_key, g_prov_current_addr);
            }
            break;
        }

        case CONFIG_CLIENT_EVT_APPKEY_STATUS: {
            SWITCH_DEBUG_EVT_SRC("ConfigAppkeyStatus", msg->meta_data.src_addr);
            if (gSwitchState == SWITCH_SETTING_STATE_GET_COMPOSITION_DONE || gSwitchState == SWITCH_SETTING_STATE_ADD_APPKEY) {
                if (event->data.appkey_status.status == BT_MESH_ACCESS_MSG_STATUS_SUCCESS) {
                    bt_mesh_app_util_stop_timer(mesh_config_timer);
                    gSwitchState = SWITCH_SETTING_STATE_ADD_APPKEY_DONE;
                    gConfigureRetry = MESH_CONFIGURE_RETRY_MAX;

                    /* start binding model and application key */
                    switch_mesh_binding_model_t *entry = bt_mesh_os_layer_ds_queue_first(binding_model_list);
                    if (entry != NULL) {
                        gSwitchState = SWITCH_SETTING_STATE_ADD_APPKEY_DONE;
                        msg_config_bind_model(
                            g_prov_current_addr + entry->element_index,
                            g_appkey_index, g_prov_current_addr, entry->model_id);
                    } else {
                        LOG_I(mesh_app, "no more model, all model binding is done\n");
                    }
                } else {
                    gSwitchState = SWITCH_SETTING_STATE_IDLE;
                    LOG_I(mesh_app, "add appkey failed, error code: %d\n", event->data.appkey_status.status);
                }
            }
            break;
        }
        case CONFIG_CLIENT_EVT_MODEL_APP_STATUS: {
            SWITCH_DEBUG_EVT_SRC("ConfigModelAppStatus", msg->meta_data.src_addr);
            if (gSwitchState == SWITCH_SETTING_STATE_ADD_APPKEY_DONE || gSwitchState == SWITCH_SETTING_STATE_BIND_MODEL) {
                if (event->data.model_app_status.status == BT_MESH_ACCESS_MSG_STATUS_SUCCESS) {
                    bt_mesh_app_util_stop_timer(mesh_config_timer);
                    gSwitchState = SWITCH_SETTING_STATE_BIND_MODEL;
                    gConfigureRetry = MESH_CONFIGURE_RETRY_MAX;

                    /* continue binding model and application key */
                    switch_mesh_binding_model_t *entry = bt_mesh_os_layer_ds_queue_pop(binding_model_list);
                    bt_mesh_os_layer_ds_queue_entry_free(entry);
                    entry = bt_mesh_os_layer_ds_queue_first(binding_model_list);
                    if (entry != NULL) {
                        gSwitchState = SWITCH_SETTING_STATE_ADD_APPKEY_DONE;
                        msg_config_bind_model(
                            g_prov_current_addr + entry->element_index,
                            g_appkey_index, g_prov_current_addr, entry->model_id);
                    } else {
                        gSwitchState = SWITCH_SETTING_STATE_DONE;
                        LOG_I(mesh_app, "no more model, all model binding is done\n");
                    }
                } else {
                    gSwitchState = SWITCH_SETTING_STATE_IDLE;
                    LOG_I(mesh_app, "model bind app failed, error code: %d, try next model\n", event->data.model_app_status.status);
                    switch_mesh_binding_model_t *entry = bt_mesh_os_layer_ds_queue_pop(binding_model_list);
                    bt_mesh_os_layer_ds_queue_entry_free(entry);
                    entry = bt_mesh_os_layer_ds_queue_first(binding_model_list);
                    if (entry != NULL) {
                        gSwitchState = SWITCH_SETTING_STATE_ADD_APPKEY_DONE;
                        msg_config_bind_model(
                            g_prov_current_addr + entry->element_index,
                            g_appkey_index, g_prov_current_addr, entry->model_id);
                    } else {
                        gSwitchState = SWITCH_SETTING_STATE_DONE;
                        LOG_I(mesh_app, "no more model, all model binding is done\n");
                    }
                }
            }
            break;
        }
        case CONFIG_CLIENT_EVT_NODE_RESET_STATUS: {
            SWITCH_DEBUG_EVT_SRC("ConfigNodeResetStatus", msg->meta_data.src_addr);
            bt_mesh_app_util_stop_timer(mesh_node_reset_timer);
            uint8_t i = 0;
            bool is_rest = false;

            nvdm_status_t status;
            uint32_t size = MAX_NVDM_BLOCK_SIZE;
            uint8_t count = 0;
            
            status = nvdm_read_data_item("MESH_APP", "node_list", temp_nvdm_buffer, &size);
            if (NVDM_STATUS_OK != status) {
                LOG_I(mesh_app, "Failed to read node info, status %x\n", status);
                return;
            }
            
            count = size / sizeof(switch_mesh_provisioned_node_data_t);
            for (i = 0; i < count; i++) {
                switch_mesh_provisioned_node_data_t node_entry;
                memcpy(&node_entry,
                       temp_nvdm_buffer + i * sizeof(switch_mesh_provisioned_node_data_t),
                       sizeof(switch_mesh_provisioned_node_data_t));
                LOG_I(mesh_app, "[%d] 0x%04x, ", i, node_entry.address);
                SWITCH_DEBUG_DUMP_PDU("devkey", BT_MESH_UUID_SIZE, node_entry.devkey);

                if (node_entry.address == msg->meta_data.src_addr) {
                    
                    // delete this node
                    if (count > 1) {
                        if (i != count - 1) {
                            memcpy(temp_nvdm_buffer + i * sizeof(switch_mesh_provisioned_node_data_t),
                                   temp_nvdm_buffer + (i + 1) * sizeof(switch_mesh_provisioned_node_data_t),
                                   (count - i - 1) *sizeof(switch_mesh_provisioned_node_data_t));
                        }
                        //if delete the last one, no need to memcpy
                        status = nvdm_write_data_item("MESH_APP", "node_list", NVDM_DATA_ITEM_TYPE_RAW_DATA, temp_nvdm_buffer, size - sizeof(switch_mesh_provisioned_node_data_t));
                    } else {
                        //only one, delete the item.
                        status = nvdm_delete_data_item("MESH_APP", "node_list");
                    }
                    if (NVDM_STATUS_OK != status) {
                        LOG_I(mesh_app,"[%d] force to delete failed, count 0x%x\n", status, count);
                    } else {
                        LOG_I(mesh_app,"force to delete 0x%04x done.\n", node_entry.address);
                    }                    
                    bt_mesh_config_delete_device_key(msg->meta_data.src_addr);
                } else if (node_entry.state == SWITCH_NODE_STATE_RESETING) {
                    is_rest = true;
                }
            }
            if (!is_rest) {
                //printf("CONFIG_CLIENT_EVT_NODE_RESET_STATUS scan ud disable \n");
                bt_mesh_provision_scan_unprovisioned_device(false);
            }
            break;
        }
        case CONFIG_CLIENT_EVT_MODEL_PUBLICATION_STATUS: {
            SWITCH_DEBUG_EVT_SRC("ConfigPublicationStatus", msg->meta_data.src_addr);
            const config_client_evt_model_publication_status_t *evt = &event->data.model_publication_status;
            LOG_I(mesh_app, "Status       : 0x%02x", evt->status);
            LOG_I(mesh_app, "ElementAddr  : 0x%04x", evt->element_address);
            LOG_I(mesh_app, "PublishAddr  : 0x%04x", evt->publish_address);
            LOG_I(mesh_app, "AppkeyIndex  : 0x%03x", evt->appkey_index);
            LOG_I(mesh_app, "Credential   : 0x%01x", evt->credential_flag);
            LOG_I(mesh_app, "PublishTTL   : 0x%02x", evt->publish_ttl);
            LOG_I(mesh_app, "PublishPeriod: 0x%02x", evt->publish_period);
            LOG_I(mesh_app, "PublishCount : 0x%02x", evt->publish_retransmit_count);
            LOG_I(mesh_app, "IntervalSteps: 0x%02x", evt->publish_retransmit_interval_steps);
            LOG_I(mesh_app, "ModelID      : 0x%08lx", evt->model_id);
            break;
        }
        case CONFIG_CLIENT_EVT_HEARTBEAT_PUBLICATION_STATUS: {
            SWITCH_DEBUG_EVT_SRC("ConfigHeartbeatPublicationStatus", msg->meta_data.src_addr);
            const config_client_evt_heartbeat_publication_status_t *evt = &event->data.heartbeat_publication_status;
            LOG_I(mesh_app, "Status       : 0x%02x", evt->status);
            LOG_I(mesh_app, "Destination  : 0x%04x", evt->destination);
            LOG_I(mesh_app, "CountLog     : 0x%02x", evt->count_log);
            LOG_I(mesh_app, "PeriodLog    : 0x%02x", evt->period_log);
            LOG_I(mesh_app, "TTL          : 0x%02x", evt->ttl);
            LOG_I(mesh_app, "Features     : 0x%04x", evt->features);
            LOG_I(mesh_app, "NetkeyIndex  : 0x%04x", evt->netkey_index);
            break;
        }
        case CONFIG_CLIENT_EVT_HEARTBEAT_SUBSCRIPTION_STATUS: {
            SWITCH_DEBUG_EVT_SRC("ConfigHeartbeatSubscriptionStatus", msg->meta_data.src_addr);
            const config_client_evt_heartbeat_subscription_status_t *evt = &event->data.heartbeat_subscription_status;
            LOG_I(mesh_app, "Status       : 0x%02x", evt->status);
            LOG_I(mesh_app, "Source       : 0x%04x", evt->source);
            LOG_I(mesh_app, "Destination  : 0x%04x", evt->destination);
            LOG_I(mesh_app, "PeriodLog    : 0x%02x", evt->period_log);
            LOG_I(mesh_app, "CountLog     : 0x%02x", evt->count_log);
            LOG_I(mesh_app, "MinHops      : 0x%02x", evt->min_hops);
            LOG_I(mesh_app, "MaxHops      : 0x%02x", evt->max_hops);
            break;
        }
#endif
        default: {
            SWITCH_DEBUG_EVT_SIG(msg->opcode.opcode);
            bt_mesh_app_util_display_hex_log(&msg->buffer[0], msg->length);
            return;
        }

    }
}

static void _health_client_msg_handler(uint16_t model_handle,
                                       const bt_mesh_access_message_rx_t *msg, const bt_mesh_health_client_evt_t *event)
{
    switch(event->evt_id) {
        case BT_MESH_HEALTH_CLIENT_EVT_CURRENT_STATUS: {
            const bt_mesh_health_client_evt_current_status_t *evt = &event->data.current_status;
            SWITCH_DEBUG_EVT_SRC("HealthCurrentStatus", msg->meta_data.src_addr);
            LOG_I(mesh_app, "TestId    : 0x%02x", evt->test_id);
            LOG_I(mesh_app, "CompanyId : 0x%04x", evt->company_id);
            LOG_I(mesh_app, "FaultArray:");
            bt_mesh_app_util_display_hex_log(evt->fault_array, evt->fault_array_length);
            break;
        }
        default: {
            /* dump raw msg */
            SWITCH_DEBUG_EVT_SIG(msg->opcode.opcode);
            bt_mesh_app_util_display_hex_log(&msg->buffer[0], msg->length);
            break;
        }
    }
}

extern void mesh_gateway_lwip_send(const void *data, size_t size);
static void _generic_client_msg_handler(uint16_t model_handle,
                                        const bt_mesh_access_message_rx_t *msg, const bt_mesh_generic_client_evt_t *event)
{
    SWITCH_DEBUG_EVT_SIG(msg->opcode.opcode);

    switch (event->evt_id) {
        case BT_MESH_GENERIC_CLIENT_EVT_ONOFF_STATUS: {
            const bt_mesh_generic_client_evt_onoff_status_t *evt = &event->data.onoff_status;
            SWITCH_DEBUG_EVT_SRC("GenericOnOffStatus", msg->meta_data.src_addr);
            LOG_I(mesh_app, "[R]Generic_OnOffStatus={src=0x%04x, onoff=0x%02x}\n", msg->meta_data.src_addr, evt->present_onoff);
            if (!bt_mesh_utils_is_local_address(msg->meta_data.src_addr)) {
                if (gIsChecking && (evt->present_onoff == gCheckOnOff)) {
                    if (evt->present_onoff == gCheckOnOff) {
                        char buf[30] = {0};
                        LOG_I(mesh_app, "recv %d, expect %d, stopped!!\n", evt->present_onoff, gCheckOnOff);
                        snprintf(buf, 30, "led is %s", (gCheckOnOff ? "on" : "off"));
                        mesh_gateway_lwip_send((void *)buf, strlen((const char *)buf));
                        bt_mesh_app_util_stop_timer(mesh_switch_timer);
                        gIsChecking = false;
                    } else if (evt->present_onoff != gCheckOnOff) {
                        LOG_I(mesh_app, "recv %d, expect %d, resend!!\n", evt->present_onoff, gCheckOnOff);
                        msg_generic_on_off_set(BT_MESH_ADDR_GROUP_NODES_VALUE, gCheckTtl, gCheckOnOff, gCheckTid, 0, 0, true);
                        bt_mesh_app_util_start_timer(mesh_switch_timer, MESH_CHECK_TIMEOUT);
                        gCheckCount++;
                    }
                }
            }
            break;
        }
        case BT_MESH_GENERIC_CLIENT_EVT_LEVEL_STATUS: {
            const bt_mesh_generic_client_evt_level_status_t *evt = &event->data.level_status;
            SWITCH_DEBUG_EVT_SRC("GenericLevelStatus", msg->meta_data.src_addr);
            LOG_I(mesh_app, "[R]Generic_LevelStatus={src=0x%04x, level=%d}\n", msg->meta_data.src_addr, evt->present_level);
            break;
        }
        default:
            break;
    }
}

static void bt_mesh_app_get_tick(uint32_t *ticks)
{
    *ticks = xTaskGetTickCount() * portTICK_PERIOD_MS;
}

static bool _isGroupDst(void)
{
    return ((gLatencyDst & ADDR_GROUP_MASK) == ADDR_GROUP_VALUE);
}

static bool _isValidIdx(const bt_mesh_access_message_rx_t *msg)
{
    SWITCH_DEBUG_DUMP_PDU("msg buffer", msg->length, msg->buffer);

    uint16_t idx = msg->buffer[2] + ((msg->buffer[3]<<8)&0xff00);

    if (idx != (gTimes-(gAutoCount+1)))
    {
        LOG_I(mesh_app, "rx idx: 0x%x, wanted idx: 0x%x, dst_addr: 0x%x\n", idx, gTimes-(gAutoCount+1), msg->meta_data.src_addr);
        return false;
    }
    return true;
}

static bool _isNextStepHandle(const bt_mesh_access_message_rx_t *msg)
{
    if (!_isValidIdx(msg))
    {
        return false;
    }
    if (_isGroupDst())
    {        
        if (gCrtNumDst)
        {
            gCrtNumDst--;
            if (gCrtNumDst)
            {
                // wait the response of other vendordevice
                return false;
            }
        }
        else
        {
            return false;
        }
        gCrtNumDst = gNumDst;
    }
    return true;
}

static void _vendorMsgHandler(uint16_t model_handle, const bt_mesh_access_message_rx_t *msg, const void *arg)
{
    SWITCH_DEBUG_EVT_VENDOR(VENDOR_COMPANY_ID, msg->opcode.opcode);
    bt_mesh_app_util_display_hex_log(&msg->buffer[0], msg->length);
    LOG_I(mesh_app, "[R]_vendorMsgHandler, count %d", gAutoCount);
    if (gState == STATE_IDLE)
        return;

    switch (msg->opcode.opcode) {
        case VENDOR_OPCODE_2: {
            uint32_t ticks;
            bt_mesh_app_get_tick(&ticks);
            ticks -= gLatencyTick;
            uint8_t tx_hops = msg->buffer[1];
            uint8_t rx_hops = (msg->buffer[0] - msg->meta_data.ttl + 1);
    
            LOG_I(mesh_app, "[R]Vendor_LatencyStatus={src=0x%04x, initTTL=0x%02x, recvHops=0x%02x}, metaTTL=0x%02x\n",
                   msg->meta_data.src_addr, msg->buffer[0], msg->buffer[1], msg->meta_data.ttl);
    
            uint8_t totalhops = tx_hops + rx_hops;
    
            if (totalhops >= 2) {
                if (gLatencyDst == 0xFFFF) {
                    uint8_t extra_hops;
                    extra_hops = (totalhops >> 1) - 1;
                    totalhops += extra_hops;
                    LOG_I(mesh_app, "(0xffff) latency total: ticks (%ld)\n", ticks);
                } else {
                    if (!_isValidIdx(msg))
                    {
                        return;
                    }
                    if (gIsSmall)
                    {
                        LOG_I(mesh_app, "tx_hops(%d) rx_hops(%d) total_hops(%d)\n", tx_hops, rx_hops, totalhops);
                        LOG_I(mesh_app, "latency avg: src=0x%04x ticks (%ld) / hops (%d) = %ld\n", msg->meta_data.src_addr, ticks, totalhops, ticks / totalhops);
                    }
                    else
                    {
                        LOG_I(mesh_app, "latency avg: src=0x%04x ticks (%ld)\n", msg->meta_data.src_addr, ticks);
                    }
                    gLatencyTickIdx += ticks;
                    gTxHopsIdx += tx_hops;
                    gRxHopsIdx += rx_hops;
                    gLatencyTickIdxHops += (ticks/totalhops);
                    if (!_isNextStepHandle(msg))
                    {
                        return;
                    }
                    LOG_I(mesh_app, "valid latency: %ld\n", gLatencyTickIdxHops);
                    gLatencyTickAvg += gLatencyTickIdx;
                    gLatencyTickAvgHops += gLatencyTickIdxHops;
                    gTxHopsAvg += gTxHopsIdx;
                    gRxHopsAvg += gRxHopsIdx;
                    gLatencyTickIdx = 0;
                    gLatencyTickIdxHops = 0;
                    gTxHopsIdx = 0;
                    gRxHopsIdx = 0;
                }
            }
            
            if (gAutoCount == 0)
            {
                gState = STATE_IDLE;
                if (gLatencyDst != 0xFFFF) {
                    LOG_I(mesh_app, "********************************\n");
                    LOG_I(mesh_app, "Total Latency:ticks(%ld)\n", gLatencyTickAvg);
                    gLatencyTickAvg = gLatencyTickAvg / (gTimes-gTid)/ gNumDst;
                    LOG_I(mesh_app, "AVG Latency(%d times):ticks(%ld)\n", gTimes-gTid, gLatencyTickAvg);
                    if (gIsSmall)
                    {
                        LOG_I(mesh_app, "Total tx_hops(%ld) rx_hops(%ld)\n", gTxHopsAvg, gRxHopsAvg);
                        gLatencyTickAvgHops = gLatencyTickAvgHops / (gTimes-gTid)/ gNumDst;
                        LOG_I(mesh_app, "AVG Latency:ticks(%ld) per hop\n", gLatencyTickAvgHops);
                    }
                    LOG_I(mesh_app, "********************************\n");
                }
                gLatencyTickIdx = 0;
                gLatencyTickAvg = 0;
                gLatencyTickAvgHops = 0;
                gTxHopsAvg = 0;
                gRxHopsAvg = 0;
                gNumDst = 1;
                gCrtNumDst = 1;
                LOG_I(mesh_app, "autotest done\n");
            }
    
            _latencytest_release_timer();
            _rx_release_timer(MESH_RX_TIMER_ID);
            bt_mesh_app_util_start_timer_ext("mesh send next", &MESH_SEND_NEXT_TIMER_ID, false, SEND_NEXT_TIMEOUT, _latencytest_send_next_timer_handler);
            break;
        }
    
        case VENDOR_OPCODE_3:
        case VENDOR_OPCODE_4:
        {
            if (!_isNextStepHandle(msg))
            {
                return;
            }
            _packetloss_release_timer();
            _rx_release_timer(MESH_PACKETLOSS_RX_TIMER_ID);
            _packetloss_check_send();
            break;
        }
    
        case VENDOR_OPCODE_5:
        case VENDOR_OPCODE_6:
        {
            if (!_isNextStepHandle(msg))
            {
                return;
            }
            _throughput_release_timer();
            _rx_release_timer(MESH_THROUGHPUT_RX_TIMER_ID);
            if (gAutoCount == 0)
            {
                _throughput_check_rx();
            }
            else
            {
                _throughput_check_send();
            }
            break;
        }
        case VENDOR_OPCODE_7:
        {
            uint32_t ticks;
            bt_mesh_app_get_tick(&ticks);
            ticks -= gLatencyTick;
            uint8_t tx_hops = msg->buffer[1];
            uint8_t rx_hops = (msg->buffer[0] - msg->meta_data.ttl + 1);

            LOG_I(mesh_app, "[R]Vendor_LatencyStatus={src=0x%04x, initTTL=0x%02x, recvHops=0x%02x}, metaTTL=0x%02x",
                   msg->meta_data.src_addr, msg->buffer[0], msg->buffer[1], msg->meta_data.ttl);

            uint8_t totalhops = tx_hops + rx_hops;

            if (totalhops >= 2) {
                if (gLatencyDst == 0xFFFF) {
                    uint8_t extra_hops;
                    extra_hops = (totalhops >> 1) - 1;
                    totalhops += extra_hops;
                    LOG_I(mesh_app, "(0xffff) latency total: ticks (%ld)\n", ticks);
                } else {
                    if (!_isValidIdx(msg))
                    {
                        return;
                    }
                    
                    if (gIsSmall)
                    {                        
                        LOG_I(mesh_app, "tx_hops(%d) rx_hops(%d) total_hops(%d)\n", tx_hops, rx_hops, totalhops);
                        LOG_I(mesh_app, "latency avg: src=0x%04x ticks (%ld) / hops (%d) = %ld\n", msg->meta_data.src_addr, ticks, totalhops, ticks / totalhops);
                    }
                    else
                    {
                        LOG_I(mesh_app, "latency avg: src=0x%04x ticks (%ld)\n", msg->meta_data.src_addr, ticks);
                    }
                    gLatencyTickIdx += ticks;
                    gTxHopsIdx += tx_hops;
                    gRxHopsIdx += rx_hops;
                    gLatencyTickIdxHops += (ticks/totalhops);
                    if (!_isNextStepHandle(msg))
                    {
                        return;
                    }
                    LOG_I(mesh_app, "valid latency\n");
                    gLatencyTickAvg += gLatencyTickIdx;
                    gLatencyTickAvgHops += gLatencyTickIdxHops;
                    gTxHopsAvg += gTxHopsIdx;
                    gRxHopsAvg += gRxHopsIdx;
                    gLatencyTickIdx = 0;
                    gLatencyTickIdxHops = 0;
                    gTxHopsIdx = 0;
                    gRxHopsIdx = 0;
                    gLatencyRspCmp = true;
                }
            }

            if (gAutoCount == 0)
            {                
                gState = STATE_IDLE;
                
                if (gLatencyDst != 0xFFFF) {
                    LOG_I(mesh_app, "********************************\n");
                    LOG_I(mesh_app, "Total Latency:ticks(%ld)\n", gLatencyTickAvg);
                    gLatencyTickAvg = gLatencyTickAvg / (gTimes-gTid)/ gNumDst;
                    LOG_I(mesh_app, "AVG Latency(%d times):ticks(%ld)\n", gTimes-gTid, gLatencyTickAvg);
                    if (gIsSmall)
                    {
                        LOG_I(mesh_app, "Total tx_hops(%ld) rx_hops(%ld)\n", gTxHopsAvg, gRxHopsAvg);
                        gLatencyTickAvgHops = gLatencyTickAvgHops / (gTimes-gTid)/ gNumDst;
                        LOG_I(mesh_app, "AVG Latency:ticks(%ld) per hop\n", gLatencyTickAvgHops);
                    }
                    LOG_I(mesh_app, "********************************\n");
                }
                gLatencyTickIdx = 0;
                gLatencyTickAvg = 0;
                gLatencyTickAvgHops = 0;
                gTxHopsAvg = 0;
                gRxHopsAvg = 0;
                gNumDst = 1;
                gCrtNumDst = 1;
                LOG_I(mesh_app, "autotest done\n");
            }

            break;
        }
        default:
            return;
    }
    
    return;
}

static void _lightness_client_msg_handler(uint16_t model_handle, const bt_mesh_access_message_rx_t *msg, const bt_mesh_lightness_client_evt_t *event)
{
    SWITCH_DEBUG_EVT_SIG(msg->opcode.opcode);

    switch (event->evt_id) {
        case BT_MESH_LIGHT_LIGHTNESS_CLIENT_EVT_LIGHTNESS_STATUS: {
            const bt_mesh_lightness_client_evt_status_t *evt = &event->data.status;
            LOG_I(mesh_app, "PresentLightness : %04x\n", evt->present_lightness);
            if (msg->length > 2) {
                LOG_I(mesh_app, "TargetLightness  : %04x\n", evt->target_lightness);
                LOG_I(mesh_app, "RemainingTime: %02x\n", evt->remaining_time);
            }
            break;
        }
        case BT_MESH_LIGHT_LIGHTNESS_CLIENT_EVT_LINEAR_STATUS: {
            const bt_mesh_lightness_client_evt_linear_status_t *evt = &event->data.linear_status;
            LOG_I(mesh_app, "PresentLightness : %04x\n", evt->present_lightness);
            if (msg->length > 2) {
                LOG_I(mesh_app, "TargetLightness  : %04x\n", evt->target_lightness);
                LOG_I(mesh_app, "RemainingTime: %02x\n", evt->remaining_time);
            }
            break;
        }
        case BT_MESH_LIGHT_LIGHTNESS_CLIENT_EVT_LAST_STATUS: {
            const bt_mesh_lightness_client_evt_last_status_t *evt = &event->data.last_status;
            LOG_I(mesh_app, "Lightness : %04x\n", evt->lightness);
            break;
        }
        case BT_MESH_LIGHT_LIGHTNESS_CLIENT_EVT_DEFAULT_STATUS: {
            const bt_mesh_lightness_client_evt_default_status_t *evt = &event->data.default_status;
            LOG_I(mesh_app, "Lightness : %04x\n", evt->lightness);
            break;
        }
        case BT_MESH_LIGHT_LIGHTNESS_CLIENT_EVT_RANGE_STATUS: {
            const bt_mesh_lightness_client_evt_range_status_t *evt = &event->data.range_status;
            LOG_I(mesh_app, "Status   : %04x\n", evt->status_code);
            LOG_I(mesh_app, "RangeMin : %04x\n", evt->range_min);
            LOG_I(mesh_app, "RangeMax : %04x\n", evt->range_max);
            break;
        }
        default: {
            bt_mesh_app_util_display_hex_log(&msg->buffer[0], msg->length);
            break;
        }
    }
}

static void _ctl_client_msg_handler(uint16_t model_handle, const bt_mesh_access_message_rx_t *msg, const bt_mesh_ctl_client_evt_t *event)
{
    SWITCH_DEBUG_EVT_SIG(msg->opcode.opcode);

    switch (event->evt_id) {
        case BT_MESH_LIGHT_CTL_CLIENT_EVT_STATUS: {
            const bt_mesh_ctl_client_evt_status_t *evt = &event->data.status;
            LOG_I(mesh_app, "PresentLightness  : %04x\n", evt->present_ctl_lightness);
            LOG_I(mesh_app, "PresentTemperature: %04x\n", evt->present_ctl_temperature);
            if (msg->length > 4) {
                LOG_I(mesh_app, "TargetLightness    : %04x\n", evt->target_ctl_lightness);
                LOG_I(mesh_app, "TargetTemperature  : %04x\n", evt->target_ctl_temperature);
                LOG_I(mesh_app, "RemainingTime: %02x\n", evt->remaining_time);
            }
            break;
        }
        case BT_MESH_LIGHT_CTL_CLIENT_EVT_TEMPERATURE_RANGE_STATUS: {
            const bt_mesh_ctl_client_evt_temperature_range_status_t *evt = &event->data.temperature_range_status;
            LOG_I(mesh_app, "Status   : %04x\n", evt->status_code);
            LOG_I(mesh_app, "RangeMin : %04x\n", evt->range_min);
            LOG_I(mesh_app, "RangeMax : %04x\n", evt->range_max);
            break;
        }
        case BT_MESH_LIGHT_CTL_CLIENT_EVT_TEMPERATURE_STATUS: {
            const bt_mesh_ctl_client_evt_temperature_status_t *evt = &event->data.temperature_status;
            LOG_I(mesh_app, "PresentTemperature: %04x\n", evt->present_ctl_temperature);
            LOG_I(mesh_app, "PresentDeltaUV    : %04x\n", evt->present_ctl_delta_uv);
            if (msg->length > 4) {
                LOG_I(mesh_app, "TargetTemperature  : %04x\n", evt->target_ctl_temperature);
                LOG_I(mesh_app, "TargetDeltaUV      : %04x\n", evt->target_ctl_delta_uv);
                LOG_I(mesh_app, "RemainingTime      : %02x\n", evt->remaining_time);
            }
            break;
        }
        case BT_MESH_LIGHT_CTL_CLIENT_EVT_DEFAULT_STATUS: {
            const bt_mesh_ctl_client_evt_default_status_t *evt = &event->data.default_status;
            LOG_I(mesh_app, "Lightness   : %04x\n", evt->lightness);
            LOG_I(mesh_app, "Temperature : %04x\n", evt->temperature);
            LOG_I(mesh_app, "DeltaUV     : %04x\n", evt->delta_uv);
            break;
        }
        default: {
            bt_mesh_app_util_display_hex_log(&msg->buffer[0], msg->length);
            break;
        }
    }
}

static void _hsl_client_msg_handler(uint16_t model_handle, const bt_mesh_access_message_rx_t *msg, const bt_mesh_hsl_client_evt_t *event)
{
    SWITCH_DEBUG_EVT_SIG(msg->opcode.opcode);

    switch (event->evt_id) {
        case BT_MESH_LIGHT_HSL_CLIENT_EVT_HUE_STATUS: {
            const bt_mesh_hsl_client_evt_hue_status_t *evt = &event->data.hue_status;
            LOG_I(mesh_app, "PresentHue : %04x\n", evt->present_hue);
            if (msg->length > 2) {
                LOG_I(mesh_app, "TargetHue  : %04x\n", evt->target_hue);
                LOG_I(mesh_app, "RemainingTime: %02x\n", evt->remaining_time);
            }
            break;
        }
        case BT_MESH_LIGHT_HSL_CLIENT_EVT_SATURATION_STATUS: {
            const bt_mesh_hsl_client_evt_saturation_status_t *evt = &event->data.saturation_status;
            LOG_I(mesh_app, "PresentSaturation : %04x\n", evt->present_saturation);
            if (msg->length > 2) {
                LOG_I(mesh_app, "TargetSaturation  : %04x\n", evt->target_saturation);
                LOG_I(mesh_app, "RemainingTime: %02x\n", evt->remaining_time);
            }
            break;
        }
        case BT_MESH_LIGHT_HSL_CLIENT_EVT_STATUS:
        case BT_MESH_LIGHT_HSL_CLIENT_EVT_TARGET_STATUS: {
            const bt_mesh_hsl_client_evt_status_t *evt = &event->data.status;
            LOG_I(mesh_app, "Lightness     : %04x\n", evt->lightness);
            LOG_I(mesh_app, "Hue           : %04x\n", evt->hue);
            LOG_I(mesh_app, "Saturation    : %04x\n", evt->saturation);
            if (msg->length > 6) {
                LOG_I(mesh_app,  "RemainingTime : %04x\n",  evt->remaining_time);
            }
            break;
        }
        case BT_MESH_LIGHT_HSL_CLIENT_EVT_DEFAULT_STATUS: {
            const bt_mesh_hsl_client_evt_default_status_t *evt = &event->data.default_status;
            LOG_I(mesh_app, "Lightness     : %04x\n", evt->lightness);
            LOG_I(mesh_app, "Hue           : %04x\n", evt->hue);
            LOG_I(mesh_app, "Saturation    : %04x\n", evt->saturation);
            break;
        }
        case BT_MESH_LIGHT_HSL_CLIENT_EVT_RANGE_STATUS: {
            const bt_mesh_hsl_client_evt_range_status_t *evt = &event->data.range_status;
            LOG_I(mesh_app, "Status   : %02x\n", evt->status_code);
            LOG_I(mesh_app, "HueMin   : %04x\n", evt->hue_min);
            LOG_I(mesh_app, "HueMin   : %04x\n", evt->hue_max);
            LOG_I(mesh_app, "SatMin   : %04x\n", evt->saturation_min);
            LOG_I(mesh_app, "SatMax   : %04x\n", evt->saturation_max);
            break;
        }
        default: {
            bt_mesh_app_util_display_hex_log(&msg->buffer[0], msg->length);
            break;
        }
    }
}

const bt_mesh_access_opcode_handler_t vendor_message_handler[] = {
    {{VENDOR_OPCODE_1, VENDOR_COMPANY_ID}, _vendorMsgHandler},
    {{VENDOR_OPCODE_2, VENDOR_COMPANY_ID}, _vendorMsgHandler},
    {{VENDOR_OPCODE_3, VENDOR_COMPANY_ID}, _vendorMsgHandler},
    {{VENDOR_OPCODE_4, VENDOR_COMPANY_ID}, _vendorMsgHandler},
    {{VENDOR_OPCODE_5, VENDOR_COMPANY_ID}, _vendorMsgHandler},
    {{VENDOR_OPCODE_6, VENDOR_COMPANY_ID}, _vendorMsgHandler},
    {{VENDOR_OPCODE_7, VENDOR_COMPANY_ID}, _vendorMsgHandler}
};

/******************************************************************************/
/* initialization                                                             */
/******************************************************************************/
static void _mesh_check_battery_level(TimerHandle_t handle)
{
    /*uint16_t voltage; //voltage in 10mV

    voltage = AB_ADC_Get_Voltage(VBAT);

    switch(voltage)
    {
        case 360 ... 65535:
            LOG_I(mesh_app,"[WARNING] Voltage is too high!%dmV\n", voltage*10);
            break;
        case   0 ... 200:
            LOG_I(mesh_app,"[WARNING] Voltage is too low!%dmV\n", voltage*10);
            //AB_PWR_Off();
            break;
        default:
            break;
    }*/
}

static void mesh_timer_init(void)
{
    bt_mesh_app_util_init_timer(&mesh_bat_timer, "mesh bat", true, _mesh_check_battery_level);
    bt_mesh_app_util_init_timer(&mesh_switch_timer, "mesh switch", false, _ack_callback);
    bt_mesh_app_util_init_timer(&mesh_config_timer, "mesh config", false, _configure_callback);
    bt_mesh_app_util_init_timer(&mesh_switch_led_timer, "mesh led", false, _switch_led_timer_cb);
    bt_mesh_app_util_init_timer(&mesh_rx_timer, "mesh rx", false, _rx_timer_handler);
    bt_mesh_app_util_init_timer(&mesh_throughput_rx_timer, "mesh th rx", false, _rx_timer_handler);
    bt_mesh_app_util_init_timer(&mesh_packet_loss_rx_timer, "mesh pl rx", false, _rx_timer_handler);
    bt_mesh_app_util_init_timer(&mesh_get_remote_info_timer, "mesh remote info", false, _get_remote_info_timeout_callback);
    
    bt_mesh_app_util_init_timer(&mesh_node_reset_timer, "mesh node reset", false, _node_reset_retry_callback);
    bt_mesh_app_util_init_timer(&wifi_service_change_timer, "wifi serv change", false, wifi_service_change_callback);
}

static void mesh_create_device(void)
{
    uint8_t composition_data_header[10] = {
        0x94, 0x00, // cid
        0x1A, 0x00, // pid
        0x01, 0x00, // vid
        0x64, 0x00, // crpl
#if RELAY_ENABLE
        BT_MESH_FEATURE_RELAY | BT_MESH_FEATURE_PROXY, 0x00, // features
#else
        BT_MESH_FEATURE_PROXY, 0x00, // features
#endif
    };

    uint16_t element_index;
    bt_mesh_model_set_composition_data_header(composition_data_header, 10);
    bt_mesh_model_add_element(&element_index, BT_MESH_MODEL_ELEMENT_LOCATION_FRONT);

    // add configuration server
    bt_mesh_model_add_configuration_server(&g_model_handle_config_server, NULL);
    // add health server
    bt_mesh_model_add_health_server(&g_model_handle_health_server, element_index, NULL);

    // add vendor model
    bt_mesh_model_add_params_t model_params;
    model_params.model_id = MESH_VENDOR_MODEL_ID(VENDOR_COMPANY_ID, VENDOR_MODEL_ID1);
    model_params.element_index = element_index;
    model_params.opcode_handlers = vendor_message_handler;
    model_params.opcode_count = sizeof(vendor_message_handler) / sizeof(bt_mesh_access_opcode_handler_t);
    model_params.publish_timeout_cb = NULL;
    bt_mesh_model_add_model(&g_model_handle_vendor, &model_params);

    // add configuration client
    g_config_client_element_index = element_index;
#ifdef PROVISIONER_ENABLE
    bt_mesh_model_add_configuration_client(&g_model_handle_config_client, element_index, _config_client_msg_handler);
#endif

    // add health client
    bt_mesh_model_add_health_client(&g_model_handle_health_client, element_index, _health_client_msg_handler);

    // add generic onoff client
    bt_mesh_model_add_generic_onoff_client(&g_model_handle_onoff_client, element_index, _generic_client_msg_handler);

    // add generic level client
    bt_mesh_model_add_generic_level_client(&g_model_handle_level_client, element_index, _generic_client_msg_handler);

    // add lighting client
    bt_mesh_model_lighting_add_lightness_client(&g_model_handle_light_lightness_client, element_index, _lightness_client_msg_handler);
    bt_mesh_model_lighting_add_ctl_client(&g_model_handle_light_ctl_client, element_index, _ctl_client_msg_handler);
    bt_mesh_model_lighting_add_hsl_client(&g_model_handle_light_hsl_client, element_index, _hsl_client_msg_handler);

    //add ota provider
    bt_mesh_ota_distributor_init(element_index, &new_firmware);
    bt_mesh_ota_initiator_init(element_index, 0);

}

void mesh_dump_model_handle(void)
{
    LOG_I(mesh_app, "mesh_dump_model_handle\n");
    LOG_I(mesh_app, "g_model_handle_config_server %x\n", g_model_handle_config_server);
    LOG_I(mesh_app, "g_model_handle_health_server %x\n", g_model_handle_health_server);
    LOG_I(mesh_app, "g_model_handle_vendor %x\n", g_model_handle_vendor);
    LOG_I(mesh_app, "g_model_handle_config_client %x\n", g_model_handle_config_client);
    LOG_I(mesh_app, "g_model_handle_health_client %x\n", g_model_handle_health_client);
    LOG_I(mesh_app, "g_model_handle_onoff_client %x\n", g_model_handle_onoff_client);
    LOG_I(mesh_app, "g_model_handle_level_client %x\n", g_model_handle_level_client);
}

QueueHandle_t mesh_task_queue_handle;
typedef struct {
    uint8_t event_id;
    TimerHandle_t timer_id;
} mesh_app_task_event_t;

void mesh_app_post_timeout_event(TimerHandle_t timer_id)
{
    mesh_app_task_event_t event;
    event.event_id = 1;//event id 1 is for timer.
    event.timer_id = timer_id;
    
    if (mesh_task_queue_handle == NULL) {
        LOG_I(mesh_app, "queue is not ready.");
        return;
    }

    if (pdPASS != xQueueSend(mesh_task_queue_handle, &event, 0)) {
        LOG_E(mesh_app, "q event lost");
    }
}

void mesh_app_process_event(mesh_app_task_event_t *event)
{
    if(event->event_id == 1) {
        TimerHandle_t timer_id = event->timer_id;
        if (timer_id == MESH_LATENCYTEST_TIMER_ID){
            _latencytest_check_send();
        } else if (timer_id == MESH_PACKETLOSS_TIMER_ID) {
            _packetloss_check_send();
        } else if (timer_id == MESH_THROUGHPUT_TIMER_ID) {
            _throughput_check_send();
        } else if (timer_id == MESH_RX_TIMER_ID || timer_id == MESH_THROUGHPUT_RX_TIMER_ID || timer_id == MESH_PACKETLOSS_RX_TIMER_ID) {
            _rx_timer_post_handler(timer_id);
        } else if (timer_id == MESH_SEND_NEXT_TIMER_ID) {
            _latencytest_check_send();
        } else if (timer_id == MESH_LATENCY_TX_TIMER_ID) {
            if (!gLatencyRspCmp)
            {
                gAutoCount++;
                gTimes++;
                gTid++;
            }
            _latencytest_check_send_ex();
        }
    }
}

void mesh_app_task_main(void *arg)
{
    mesh_app_task_event_t event;
    mesh_task_queue_handle = xQueueCreate(10, sizeof(mesh_app_task_event_t));

    while (1) {
        if (pdPASS == xQueueReceive(mesh_task_queue_handle, &event, portMAX_DELAY)) {
            mesh_app_process_event(&event);
        }
    }
}

static void mesh_app_task_create()
{
    if (pdPASS != xTaskCreate(mesh_app_task_main, MESH_APP_TASK_NAME, MESH_APP_TASK_STACKSIZE / sizeof(portSTACK_TYPE), NULL, MESH_APP_TASK_PRIO, NULL)) {
        LOG_E(mesh_app, "mesh app test task cannot be created.");
    }
}

static void meshinit(void)
{
    LOG_I(mesh_app, "Mesh initialising...\n");

    bt_mesh_init_params_t *initparams = (bt_mesh_init_params_t *)bt_mesh_app_util_alloc(sizeof(bt_mesh_init_params_t));
#ifdef PROVISIONER_ENABLE
    initparams->role = BT_MESH_ROLE_PROVISIONER;
    // no need to init provisionee params
    initparams->provisionee = NULL;
#else
    initparams->role = BT_MESH_ROLE_PROVISIONEE;
    // init provisionee parameter
    initparams->provisionee = (bt_mesh_prov_provisionee_params_t *)bt_mesh_app_util_alloc(sizeof(bt_mesh_prov_provisionee_params_t));
    initparams->provisionee->cap.number_of_elements = bt_mesh_model_get_element_count();
    initparams->provisionee->cap.algorithms = BT_MESH_PROV_CAPABILITY_ALGORITHM_FIPS_P256_ELLIPTIC_CURVE; // bit 0: P-256, bit 1~15: RFU
    initparams->provisionee->cap.public_key_type = BT_MESH_PROV_CAPABILITY_OOB_PUBLIC_KEY_TYPE_INBAND;
    initparams->provisionee->cap.static_oob_type = BT_MESH_PROV_CAPABILITY_OOB_STATIC_TYPE_SUPPORTED;
    initparams->provisionee->cap.output_oob_size = 0x00;
    initparams->provisionee->cap.output_oob_action = 0x0000;
    initparams->provisionee->cap.input_oob_size = 0x00;
    initparams->provisionee->cap.input_oob_action = 0x0000;
#endif

    // init config parameter
    initparams->config = (bt_mesh_config_init_params_t *)bt_mesh_app_util_alloc(sizeof(bt_mesh_config_init_params_t));
    memset(initparams->config, 0, sizeof(bt_mesh_config_init_params_t));
    memcpy(initparams->config->device_uuid, deviceUuid, BT_MESH_UUID_SIZE);
    initparams->config->oob_info = 0;
    initparams->config->uri = NULL;
    initparams->config->default_ttl = 4;

    initparams->friend = NULL;

    bt_mesh_init(initparams);
#ifndef PROVISIONER_ENABLE
    bt_mesh_app_util_free((uint8_t *)(initparams->provisionee));
#endif
    bt_mesh_app_util_free((uint8_t *)(initparams->config));
    bt_mesh_app_util_free((uint8_t *)initparams);
}

static void mesh_led_init()
{
#ifdef MTK_LED_ENABLE
    bsp_led_init(BSP_LED0_PIN);
#endif
}

static void mesh_ota_init(void)
{
    /*OTA_init(_mesh_ota_cb);

    if (1) { // may detect ADC and decide whether OTA should be enabled
        OTA_change_permission(OTA_ENABLED);
    }
    LOG_I(mesh_app,"OTA service init done.\n");*/
}

static void switch_led_on_off( uint8_t ucLed , bool bOnOff )
{
#ifdef MTK_LED_ENABLE
    if(bOnOff) {
        bsp_led_on(BSP_LED0_PIN);
    } else {
        bsp_led_off(BSP_LED0_PIN);
    }
#endif
}

extern void mesh_gateway_init(void);
#define MESH_WIFI_SERVICE_TIME (20)    //32, 22, 18 
bt_status_t bt_app_event_callback_demo(bt_msg_type_t msg, bt_status_t status, void *buff)
{
    switch (msg) {
        case BT_POWER_ON_CNF: {
            LOG_I(mesh_app, "BT_POWER_ON_CNF\n");
            // init wifi service time, default value 22
            bt_mesh_app_util_change_wifi_service_time(MESH_WIFI_SERVICE_TIME);
            bt_mesh_app_util_check_device_uuid(deviceUuid);
            
            mesh_led_init();
            mesh_ota_init();
            mesh_timer_init();
            mesh_app_task_create();
            
            mesh_gateway_init();
            
            // create mesh device
            mesh_create_device();
            meshinit();
            break;
        }
        case BT_GAP_LE_ADVERTISING_REPORT_IND: {
            /*bt_gap_le_advertising_report_ind_t *report = (bt_gap_le_advertising_report_ind_t *)buff;

            if (report->event_type == BT_GAP_LE_ADV_REPORT_EVT_TYPE_ADV_IND) {
                // finding mesh service
                connectabt_mesh_device_t *device = NULL;
                uint8_t *ptr;
                uint8_t len;
                ptr = report->data;
                len = report->data_length;
                while (len) {
                    if (ptr[1] == BT_GAP_LE_AD_TYPE_SERVICE_DATA) {
                        uint16_t service_uuid = (ptr[3] << 8) | ptr[2];
                        if (MESH_PROVISION_SERVICE_UUID == service_uuid) {
                            device = (connectabt_mesh_device_t *)bt_mesh_os_layer_ds_queue_entry_alloc(sizeof(connectabt_mesh_device_t));
                            device->type = BT_MESH_GATT_SERVICE_PROVISION;
                            device->data_len = ptr[0] - 4;
                            //LOG_I(mesh_app, "Mesh provisioning service found\n");
                            break;
                        } else if (MESH_PROXY_SERVICE_UUID == service_uuid) {
                            device = (connectabt_mesh_device_t *)bt_mesh_os_layer_ds_queue_entry_alloc(sizeof(connectabt_mesh_device_t));
                            device->type = BT_MESH_GATT_SERVICE_PROXY;
                            device->data_len = ptr[0] - 4;
                            //LOG_I(mesh_app, "Mesh proxy service found\n");
                            break;
                        }
                    }

                    ptr += (ptr[0] + 1);
                    len -= (ptr[0] + 1);
                }
                if (device != NULL && device->data_len != 0) {
                    connectabt_mesh_device_t *entry = (connectabt_mesh_device_t *)bt_mesh_os_layer_ds_queue_first(gMeshDeviceQueue);
                    while (entry != NULL) {
                        if (memcmp(entry->data, ptr + 4, device->data_len) == 0) {
                            // update rssi only
                            entry->rssi = report->rssi;
                            // already in queue, bt_mesh_app_util_free it
                            bt_mesh_os_layer_ds_queue_entry_free(device);
                            return;
                        }
                        entry = (connectabt_mesh_device_t *)bt_mesh_os_layer_ds_queue_next(gMeshDeviceQueue, entry);
                    }
                    device->data = bt_mesh_app_util_alloc(device->data_len);
                    memcpy(device->data, ptr + 4, device->data_len);
                    memcpy(&device->addr, &report->address, sizeof(bt_addr_t));

                    bt_mesh_os_layer_ds_queue_push(gMeshDeviceQueue, device);
                }
            }*/
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
            SWITCH_DEBUG_DUMP_PDU("UUID", BT_MESH_UUID_SIZE, deviceUuid);
#ifdef PROVISIONER_ENABLE
            LOG_I(mesh_app, "with provisioner enabled.\n");
#endif
            LOG_I(mesh_app, "==============================\n\n");
            bt_mesh_enable();
            switch_load_prov_addr();
            bt_mesh_app_util_start_timer(mesh_switch_led_timer, 200);//todo
            break;
        }
        case BT_MESH_EVT_PROV_SCAN_UD_RESULT: {
            bt_mesh_evt_prov_scan_ud_t *scanud = (bt_mesh_evt_prov_scan_ud_t *)evt_data;
            LOG_I(mesh_app, "BT_MESH_EVT_PROV_SCAN_UD_RESULT\n");
            LOG_I(mesh_app, "oobinfo: 0x%04x, ", scanud->device.oob_info);
            SWITCH_DEBUG_DUMP_PDU("scan UUID", BT_MESH_UUID_SIZE, scanud->device.uuid);

            uint8_t i;
            bool is_rest = false;
            
            nvdm_status_t status;
            uint32_t size = MAX_NVDM_BLOCK_SIZE;
            uint8_t count = 0;
            
            status = nvdm_read_data_item("MESH_APP", "node_list", temp_nvdm_buffer, &size);
            if (NVDM_STATUS_OK != status) {
                LOG_I(mesh_app, "Failed to read node info, status %x\n", status);
                if (!is_rest) {
                    bt_mesh_provision_scan_unprovisioned_device(false);
                }
                return BT_MESH_ERROR_FAIL;
            }
            
            count = size / sizeof(switch_mesh_provisioned_node_data_t);
            for (i = 0; i < count; i++) {
                switch_mesh_provisioned_node_data_t node_entry;
                memcpy(&node_entry,
                       temp_nvdm_buffer + i * sizeof(switch_mesh_provisioned_node_data_t),
                       sizeof(switch_mesh_provisioned_node_data_t));

                if (0 == memcmp(node_entry.uuid, scanud->device.uuid, BT_MESH_UUID_SIZE)) {					
                    LOG_I(mesh_app, "[%d] 0x%04x, ", i, node_entry.address);
                    SWITCH_DEBUG_DUMP_PDU("node UUID", BT_MESH_UUID_SIZE, node_entry.uuid);
                    // delete this node
                    if (count > 1) {
                        if (i != count - 1) {
                            memcpy(temp_nvdm_buffer + i * sizeof(switch_mesh_provisioned_node_data_t),
                                   temp_nvdm_buffer + (i + 1) * sizeof(switch_mesh_provisioned_node_data_t),
                                   (count - i - 1) *sizeof(switch_mesh_provisioned_node_data_t));
                        }
                        //if delete the last one, no need to memcpy
                        status = nvdm_write_data_item("MESH_APP", "node_list", NVDM_DATA_ITEM_TYPE_RAW_DATA, temp_nvdm_buffer, size - sizeof(switch_mesh_provisioned_node_data_t));
                    } else {
                        //only one, delete the item.
                        status = nvdm_delete_data_item("MESH_APP", "node_list");
                    }
                    if (NVDM_STATUS_OK != status) {
                        LOG_I(mesh_app,"[%d] force to delete failed, count 0x%x\n", status, count);
                    } else {
                        LOG_I(mesh_app,"force to delete 0x%04x done.\n", node_entry.address);
                    }
                    bt_mesh_config_delete_device_key(node_entry.address);
                    break;
                } else if (node_entry.state == SWITCH_NODE_STATE_RESETING) {
                    is_rest = true;
                }
            }

            if (!is_rest) {
                bt_mesh_provision_scan_unprovisioned_device(false);
            }
            break;
        }
        case BT_MESH_EVT_PROV_CAPABILITIES: {
            bt_mesh_evt_prov_capabilities_t *p = (bt_mesh_evt_prov_capabilities_t *)evt_data;
            LOG_I(mesh_app, "BT_MESH_EVT_PROV_CAPABILITIES\n");
            LOG_I(mesh_app, "\tNumOfElements: 0x%02x\n", p->cap.number_of_elements);
            LOG_I(mesh_app, "\tAlgorithms: 0x%04x\n", p->cap.algorithms);
            LOG_I(mesh_app, "\tPublicKeyType: 0x%02x\n", p->cap.public_key_type);
            LOG_I(mesh_app, "\tStaticOobType: 0x%02x\n", p->cap.static_oob_type);
            LOG_I(mesh_app, "\tOutputOobSize: 0x%02x\n", p->cap.output_oob_size);
            LOG_I(mesh_app, "\tOutputOobAction: 0x%04x\n", p->cap.output_oob_action);
            LOG_I(mesh_app, "\tInputOobSize: 0x%02x\n", p->cap.input_oob_size);
            LOG_I(mesh_app, "\tInputOobAction: 0x%04x\n", p->cap.input_oob_action);
            if (g_start_params->data.address + p->cap.number_of_elements > 0x7FFF) {
                LOG_I(mesh_app, "no valid addr for this node, stop provisioning\n");
                bt_mesh_provision_stop_provisioning(BT_MESH_PROV_FAILED_ERROR_CODE_CANNOT_ASSIGN_ADDR);
            } else {
                if (g_start_params->start.authentication_method == BT_MESH_PROV_START_AUTHEN_METHOD_STATIC_OOB &&
                    p->cap.static_oob_type == BT_MESH_PROV_CAPABILITY_OOB_STATIC_TYPE_SUPPORTED) {
                    g_start_params->start.authentication_method = BT_MESH_PROV_START_AUTHEN_METHOD_STATIC_OOB;
                } else {
                    g_start_params->start.authentication_method = BT_MESH_PROV_START_AUTHEN_METHOD_NO_OOB;
                }
                bt_mesh_status_t ret = bt_mesh_provision_start_provisioning(g_start_params);
                if (BT_MESH_SUCCESS != ret) {
                    LOG_I(mesh_app, "start provisioning failed %d\n", ret);
                }
            }
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

        case BT_MESH_EVT_PROV_SHOW_OOB_PUBLIC_KEY: {
            bt_mesh_evt_prov_show_pk_t *p = (bt_mesh_evt_prov_show_pk_t *)evt_data;
            SWITCH_DEBUG_DUMP_PDU("PublicKey", 64, p->pk);
            bt_mesh_app_util_free(p->pk);
            break;
        }
        case BT_MESH_EVT_PROV_SHOW_OOB_AUTH_VALUE: {
            LOG_I(mesh_app, "BT_MESH_EVT_PROV_SHOW_OOB_AUTH_VALUE\n");
            break;
        }
        case BT_MESH_EVT_PROV_DONE: {
            gProvisioningTick = xTaskGetTickCount() * portTICK_PERIOD_MS - gProvisioningTick;
            bt_mesh_evt_prov_done_t *p = (bt_mesh_evt_prov_done_t *)evt_data;
            LOG_I(mesh_app_must, "BT_MESH_EVT_PROV_DONE %s, %d ms\n", status ? "SUCCESS" : "FAILED", gProvisioningTick);
            if(gProvisioningTick > 10000) {
                mesh_change_ble_wifi_schedule();
            }
#ifdef PROVISIONER_ENABLE
            LOG_I(mesh_app, "\tUnicastAddr: 0x%lx\n", p->address);
            LOG_I(mesh_app, "\t");
            SWITCH_DEBUG_DUMP_PDU("DeviceKey", BT_MESH_DEVKEY_SIZE, p->device_key);

            if (status) {
                memcpy(g_device_key, p->device_key, BT_MESH_DEVKEY_SIZE);
                gSwitchState = SWITCH_SETTING_STATE_PROVISIONED;
                if (!g_provision_by_gatt) {
                    LOG_I(mesh_app, "start to configure provisioned device...\n");
                    gConfigureRetry = MESH_CONFIGURE_RETRY_MAX;//reset retry count
                    msg_config_get_composition(g_prov_current_addr);
                } else {
                    bt_mesh_bearer_gatt_disconnect();
                }
                switch_store_prov_addr(p->address, p->device_key, g_prov_current_uuid);
            } else {
                if (g_provision_by_gatt) {
                    bt_mesh_bearer_gatt_disconnect();
                }
                gSwitchState = SWITCH_SETTING_STATE_IDLE;
            }
#endif
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
            break;
        }
        case BT_MESH_EVT_BEARER_GATT_STATUS: {
            bt_mesh_evt_bearer_gatt_status_t *p = (bt_mesh_evt_bearer_gatt_status_t *)evt_data;
            LOG_I(mesh_app, "BT_MESH_EVT_BEARER_GATT_STATUS\n");
            LOG_I(mesh_app, "\tHandle:%ld, status:%x\n", p->handle, p->status);
            if (p->status == BT_MESH_BEARER_GATT_STATUS_CONNECTED) {
                LOG_I(mesh_app, "BT_MESH_BEARER_GATT_STATUS_CONNECTED\n");
                if (g_provision_by_gatt && ((gSwitchState == SWITCH_SETTING_STATE_IDLE) || (gSwitchState >= SWITCH_SETTING_STATE_PROVISIONED))) {
                    uint8_t uuid[BT_MESH_UUID_SIZE] = {0};
                    bt_mesh_prov_invite_t params = {0};
                    bt_mesh_provision_invite_provisioning(uuid, &params);
                    gSwitchState = SWITCH_SETTING_STATE_PROVISIONING;
                } else {
                    LOG_I(mesh_app, "already in process, state %d\n", gSwitchState);
                }
            } else if (p->status == BT_MESH_BEARER_GATT_STATUS_DISCONNECTED) {
                LOG_I(mesh_app, "BT_MESH_BEARER_GATT_STATUS_DISCONNECTED\n");
                if (g_provision_by_gatt && gSwitchState == SWITCH_SETTING_STATE_PROVISIONED) {
                    LOG_I(mesh_app, "start to configure provisioned device...\n");
                    msg_config_get_composition(g_prov_current_addr);
                }
                g_provision_by_gatt = false;
            } else {
                bt_mesh_bearer_gatt_disconnect();
            }
            break;
        }
    }
    
    return BT_MESH_SUCCESS;
}

/* command line */
extern void bt_mesh_transport_set_transmit_count(uint8_t count);
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
    }else if (!strcmp(argv[1], "addr")) {
        if (argc < 3) {
            LOG_I(mesh_app, "USAGE: config addr <addr>\n");
            return BT_STATUS_FAIL;
        }
        uint16_t addr;
        bt_mesh_app_util_str2u16HexNum(argv[2], &addr);
        bt_mesh_model_set_element_addr(addr);
    } else if (!strcmp(argv[1], "ttl")) {
        if (argc < 3) {
            LOG_I(mesh_app, "USAGE: config ttl <ttl>\n");
            return BT_STATUS_FAIL;
        }
        uint8_t ttl = 0;
        bt_mesh_app_util_str2u8HexNum(argv[2], &ttl);
        bt_mesh_config_set_default_ttl(ttl);
    } else if (!strcmp(argv[1], "bind")) {
        uint8_t ret;
        /* binding health client/server models */
        ret = bt_mesh_model_bind_model_app(g_model_handle_health_server, g_appkey_index);
        LOG_I(mesh_app, "bind g_model_handle_health_server ret: %d\n", ret);
        ret = bt_mesh_model_bind_model_app(g_model_handle_health_client, g_appkey_index);
        LOG_I(mesh_app, "bind g_model_handle_health_client ret: %d\n", ret);

        /* binding generic onoff client model */
        ret = bt_mesh_model_bind_model_app(g_model_handle_onoff_client, g_appkey_index);
        LOG_I(mesh_app, "bind g_model_handle_onoff_client ret: %d\n", ret);

        /* binding generic level client model */
        ret = bt_mesh_model_bind_model_app(g_model_handle_level_client, g_appkey_index);
        LOG_I(mesh_app, "bind g_model_handle_level_client ret: %d\n", ret);

        /* binding vendor model */
        ret = bt_mesh_model_bind_model_app(g_model_handle_vendor, g_appkey_index);
        LOG_I(mesh_app, "bind g_model_handle_vendor ret: %d\n", ret);


        /* binding lighting client models */
        ret = bt_mesh_model_bind_model_app(g_model_handle_light_lightness_client, g_appkey_index);
        LOG_I(mesh_app, "bind g_model_handle_light_lightness_client ret: %d\n", ret);
        ret = bt_mesh_model_bind_model_app(g_model_handle_light_hsl_client, g_appkey_index);
        LOG_I(mesh_app, "bind g_model_handle_light_hsl_client ret: %d\n", ret);
        ret = bt_mesh_model_bind_model_app(g_model_handle_light_ctl_client, g_appkey_index);
        LOG_I(mesh_app, "bind g_model_handle_light_ctl_client ret: %d\n", ret);

        bt_mesh_ota_initiator_bind_models_app(g_appkey_index);
        bt_mesh_ota_distributor_bind_models_app(g_appkey_index);
        
    } else if (!strcmp(argv[1], "iv")) {
        if (argc < 4) {
            LOG_I(mesh_app, "USAGE: config iv <ivindex> <state>\n");
            return BT_STATUS_FAIL;
        }
        uint32_t ivindex = 0;
        bt_mesh_app_util_str2u32HexNum(argv[2], &ivindex);
        bt_mesh_config_set_iv_index(ivindex, atoi(argv[3]));
    } else if (!strcmp(argv[1], "seq")) {
        if (argc < 3) {
            LOG_I(mesh_app, "USAGE: config seq <senquence_num>\n");
            return BT_STATUS_FAIL;
        }
        uint32_t seq_num = 0;
        bt_mesh_app_util_str2u32HexNum(argv[2], &seq_num);
        if (seq_num > 0xFFFFFF) {
            return BT_STATUS_FAIL;
        }
        bt_mesh_config_set_sequence_number(seq_num);
    }  else if (!strcmp(argv[1], "ota_group")) {
        if (argc < 3) {
            LOG_I(mesh_app, "USAGE: config ota_group <1 or 0>");
            return BT_STATUS_FAIL;
        }
        uint8_t grp = 0;
        bt_mesh_app_util_str2u8HexNum(argv[2], &grp);
        bt_mesh_ota_distributor_set_unicast_missing_dispatch(grp);
    } else if (!strcmp(argv[1], "transcount")) {
        if (argc < 3) {
            LOG_I(mesh_app, "USAGE: config transcount <count>");
            return BT_STATUS_FAIL;
        }

        bt_mesh_transport_set_transmit_count(atoi(argv[2]));
    }

    return BT_STATUS_SUCCESS;
}

bt_status_t cmd_delete(void *no, uint16_t argc, char **argv)
{
    if (argc < 2) {
        LOG_I(mesh_app, "delete all mesh sectors.\n");
        bt_mesh_flash_reset(BT_MESH_FLASH_SECTOR_ALL);
        bt_mesh_config_clear_replay_protection();
        switch_delete_prov_addr(0xffff);
        return BT_STATUS_SUCCESS;
    }

    if (!strcmp(argv[1], "seq")) {
        LOG_I(mesh_app, "delete all mesh sequence number sector.\n");
        bt_mesh_flash_reset(BT_MESH_FLASH_SECTOR_SEQUENCE_NUMBER);
    } else if (!strcmp(argv[1], "data")) {
        LOG_I(mesh_app, "delete all mesh data sector.\n");
        bt_mesh_flash_reset(BT_MESH_FLASH_SECTOR_DATA);
    } else if (!strcmp(argv[1], "replay")) {
        bt_mesh_config_clear_replay_protection();
    } else if (!strcmp(argv[1], "device")) {
        if (argc < 3) {
            LOG_I(mesh_app, "USAGE: delete device <node_address> (0xffff means all nodes)\n");
            return BT_STATUS_FAIL;
        }

        uint16_t addr = 0;
        bt_mesh_app_util_str2u16HexNum(argv[2], &addr);

        if (addr == BT_MESH_ADDR_GROUP_NODES_VALUE) {
            LOG_I(mesh_app, "delete all provisioned device info.\n");
        } else {
            LOG_I(mesh_app, "delete device info of node 0x%04x.\n", addr);
        }
        switch_delete_prov_addr(addr);
        bt_mesh_provision_scan_unprovisioned_device(true);
    } else if(!strcmp(argv[1], "xxxxx")) {        
#ifdef CHIP_AB1613
            LOG_I(mesh_app,"self erase flash valid pattern to enter download mode\n");
            DRV_Flash_SectorErase(0x200000);
#endif
    }

    return BT_STATUS_SUCCESS;
}


#ifdef PROVISIONER_ENABLE
static bt_status_t cmd_prov_scan(void *no, uint16_t argc, char **argv)
{
    if (argc < 2) {
        LOG_I(mesh_app, "USAGE: prov scan <enable>\n");
        return BT_STATUS_FAIL;
    }

    bt_mesh_provision_scan_unprovisioned_device(atoi(argv[1]));
    return BT_STATUS_SUCCESS;
}

static bt_status_t cmd_prov_invite(void *no, uint16_t argc, char **argv)
{
    if (argc < 2) {
        LOG_I(mesh_app, "USAGE: prov invite <uuid> [<attention_duration>]\n");
        return BT_STATUS_FAIL;
    }

    if ((gSwitchState == SWITCH_SETTING_STATE_IDLE) || (gSwitchState == SWITCH_SETTING_STATE_DONE)) {
        uint8_t uuid[BT_MESH_UUID_SIZE] = {0};
        bt_mesh_prov_invite_t params;
        // uuid
        if (strlen(argv[1]) != 2 * BT_MESH_UUID_SIZE) {
            LOG_I(mesh_app, "<uuid> is a 16-byte hex array\n");
            return BT_STATUS_FAIL;
        }
        bt_mesh_app_util_str2u8HexNumArray(argv[1], uuid);

        // invite
        if (argc < 3) {
            params.attention_duration = 0;
        } else {
            bt_mesh_app_util_str2u8HexNum(argv[2], &params.attention_duration);
        }

        bt_mesh_provision_invite_provisioning(uuid, &params);
        gSwitchState = SWITCH_SETTING_STATE_PROVISIONING;
    } else {
        LOG_I(mesh_app, "already in process, state %d\n", gSwitchState);
    }
    return BT_STATUS_SUCCESS;
}

static bt_status_t cmd_prov_start(void *no, uint16_t argc, char **argv)
{
    if (argc < 6) {
        LOG_I(mesh_app, "USAGE: prov start <netkey> <netidx> <ivindex> <address> <flags>\n");
        return BT_STATUS_FAIL;
    }

    if (gSwitchState == SWITCH_SETTING_STATE_PROVISIONING) {
        bt_mesh_prov_provisioner_params_t params;
        // start
        params.start.algorithm = BT_MESH_PROV_START_ALGORITHM_FIPS_P256_ELLIPTIC_CURVE;
        params.start.public_key = BT_MESH_PROV_START_PUBLIC_KEY_NO_OOB;
        params.start.authentication_method = BT_MESH_PROV_START_AUTHEN_METHOD_STATIC_OOB;
        params.start.authentication_action = 0;
        params.start.authentication_size = 0;

        // data
        if (strlen(argv[1]) != 2 * BT_MESH_NETKEY_SIZE) {
            LOG_I(mesh_app, "<netkey> is a 16-byte hex array\n");
            return BT_STATUS_FAIL;
        }
        bt_mesh_app_util_str2u8HexNumArray(argv[1], params.data.netkey);

        bt_mesh_app_util_str2u16HexNum(argv[2], &params.data.netkey_index);
        bt_mesh_app_util_str2u32HexNum(argv[3], &params.data.iv_index);
        bt_mesh_app_util_str2u16HexNum(argv[4], &params.data.address);
        bt_mesh_app_util_str2u8HexNum(argv[5], &params.data.flags);

        g_prov_current_netidx = params.data.netkey_index;
        g_prov_current_addr = params.data.address;
        gConfigureRetry = MESH_CONFIGURE_RETRY_MAX;
        bt_mesh_status_t ret = bt_mesh_provision_start_provisioning(&params);
        if (BT_MESH_SUCCESS != ret) {
            LOG_I(mesh_app, "start provisioning failed %d\n", ret);
        }
    } else {
        LOG_I(mesh_app, "already in process, state %d\n", gSwitchState);
    }
    return BT_STATUS_SUCCESS;
}



static bt_status_t cmd_prov_run(void *no, uint16_t argc, char **argv)
{
    if (argc < 7) {
        LOG_I(mesh_app, "USAGE: prov run <uuid> <netkey> <netidx> <ivindex> <address> <flags> [<static_oob>]\n");
        return BT_STATUS_FAIL;
    }

    if ((gSwitchState == SWITCH_SETTING_STATE_IDLE) || (gSwitchState >= SWITCH_SETTING_STATE_PROVISIONED)) {
        uint8_t uuid[BT_MESH_UUID_SIZE] = {0};
        bt_mesh_prov_invite_t params;

        // uuid
        if (strlen(argv[1]) != 2 * BT_MESH_UUID_SIZE) {
            LOG_I(mesh_app, "<uuid> is a 16-byte hex array\n");
            return BT_STATUS_FAIL;
        }
        bt_mesh_app_util_str2u8HexNumArray(argv[1], uuid);

        // invite
        if (argc < 3) {
            params.attention_duration = 0;
        } else {
            bt_mesh_app_util_str2u8HexNum(argv[2], &params.attention_duration);
        }

        // start
        if (g_start_params != NULL) {
            bt_mesh_app_util_free((uint8_t *)g_start_params);
        }
        g_start_params = (bt_mesh_prov_provisioner_params_t *)bt_mesh_app_util_alloc(sizeof(bt_mesh_prov_provisioner_params_t));
        g_start_params->start.algorithm = BT_MESH_PROV_START_ALGORITHM_FIPS_P256_ELLIPTIC_CURVE;
        g_start_params->start.public_key = BT_MESH_PROV_START_PUBLIC_KEY_NO_OOB;
        if (argc == 8) {
            g_start_params->start.authentication_method = atoi(argv[7]) != 0 ? 
                BT_MESH_PROV_START_AUTHEN_METHOD_STATIC_OOB : BT_MESH_PROV_START_AUTHEN_METHOD_NO_OOB;
        } else {
            g_start_params->start.authentication_method = BT_MESH_PROV_START_AUTHEN_METHOD_STATIC_OOB;
        }
        g_start_params->start.authentication_action = 0;
        g_start_params->start.authentication_size = 0;

        // data
        if (strlen(argv[2]) != 2 * BT_MESH_NETKEY_SIZE) {
            LOG_I(mesh_app, "<netkey> is a 16-byte hex array, ex: 00112233445566778899aabbccddeeff\n");
            return BT_STATUS_FAIL;
        }
        bt_mesh_app_util_str2u8HexNumArray(argv[2], g_start_params->data.netkey);
        bt_mesh_app_util_str2u16HexNum(argv[3], &g_start_params->data.netkey_index);
        bt_mesh_app_util_str2u32HexNum(argv[4], &g_start_params->data.iv_index);
        bt_mesh_app_util_str2u16HexNum(argv[5], &g_start_params->data.address);
        bt_mesh_app_util_str2u8HexNum(argv[6], &g_start_params->data.flags);
        g_prov_current_netidx = g_start_params->data.netkey_index;
        g_prov_current_addr = g_start_params->data.address;
        memcpy(&g_prov_current_uuid, uuid, BT_MESH_UUID_SIZE);
        gConfigureRetry = MESH_CONFIGURE_RETRY_MAX;
        gProvisioningTick = xTaskGetTickCount() * portTICK_PERIOD_MS;
        bt_mesh_provision_invite_provisioning(uuid, &params);
        gSwitchState = SWITCH_SETTING_STATE_PROVISIONING;
    } else {
        LOG_I(mesh_app, "already in process, state %d\n", gSwitchState);
    }
    return BT_STATUS_SUCCESS;
}

static bt_status_t cmd_prov_stop(void *no, uint16_t argc, char **argv)
{
    bt_mesh_provision_stop_provisioning(BT_MESH_PROV_FAILED_ERROR_CODE_OUT_OF_RESOURCES);
    return BT_STATUS_SUCCESS;
}

static bt_status_t cmd_prov_init(void *no, uint16_t argc, char **argv)
{
    if (argc < 6) {
        LOG_I(mesh_app, "USAGE: prov init <netkey> <netidx> <ivindex> <address> <flags>\n");
        return BT_STATUS_FAIL;
    }
    // start
    if (g_start_params != NULL) {
        bt_mesh_app_util_free((uint8_t *)g_start_params);
    }
    g_start_params = (bt_mesh_prov_provisioner_params_t *)bt_mesh_app_util_alloc(sizeof(bt_mesh_prov_provisioner_params_t));
    g_start_params->start.algorithm = BT_MESH_PROV_START_ALGORITHM_FIPS_P256_ELLIPTIC_CURVE;
    g_start_params->start.public_key = BT_MESH_PROV_START_PUBLIC_KEY_NO_OOB;
    g_start_params->start.authentication_method = BT_MESH_PROV_START_AUTHEN_METHOD_STATIC_OOB;
    g_start_params->start.authentication_action = 0;
    g_start_params->start.authentication_size = 0;

    // data
    bt_mesh_app_util_str2u8HexNumArray(argv[1], g_start_params->data.netkey);
    bt_mesh_app_util_str2u16HexNum(argv[2], &g_start_params->data.netkey_index);
    bt_mesh_app_util_str2u32HexNum(argv[3], &g_start_params->data.iv_index);
    bt_mesh_app_util_str2u16HexNum(argv[4], &g_start_params->data.address);
    bt_mesh_app_util_str2u8HexNum(argv[5], &g_start_params->data.flags);
    g_prov_current_netidx = g_start_params->data.netkey_index;
    g_prov_current_addr = g_start_params->data.address;

    return BT_STATUS_SUCCESS;
}
#endif

bt_status_t cmd_key(void *no, uint16_t argc, char **argv)
{
    if (argc < 2) {
        return BT_STATUS_FAIL;
    }

    if (!strcmp(argv[1], "add")) {
        if (!strcmp(argv[2], "net")) {
            if (argc < 5) {
                LOG_I(mesh_app, "USAGE: key add net <netkey> <keyidx>\n");
                return BT_STATUS_FAIL;
            }

            uint8_t ret, key[BT_MESH_NETKEY_SIZE];
            uint16_t keyidx;
            if (strlen(argv[3]) != 2 * BT_MESH_NETKEY_SIZE) {
                LOG_I(mesh_app, "<netkey> is a 16-byte hex array, ex: 00112233445566778899aabbccddeeff\n");
                return BT_STATUS_FAIL;
            }
            bt_mesh_app_util_str2u8HexNumArray(argv[3], key);

            bt_mesh_app_util_str2u16HexNum(argv[4], &keyidx);
            ret = bt_mesh_config_add_network_key(key, keyidx);

            if (ret) {
                LOG_I(mesh_app, "ERROR!!!! [%02x]\n", ret);
            }
        } else if (!strcmp(argv[2], "app")) {
            //ex:key add app 22222222222222222222222222222222 0x123 0x456
            //for appkey index = 0x123, netkey index = 0x456
            if (argc < 6) {
                LOG_I(mesh_app, "USAGE: key add app <appkey> <appidx> <netidx>\n");
                return BT_STATUS_FAIL;
            }

            uint8_t ret, key[BT_MESH_APPKEY_SIZE];
            uint16_t appidx, netidx;
            if (strlen(argv[3]) != 2 * BT_MESH_APPKEY_SIZE) {
                LOG_I(mesh_app, "<appkey> is a 16-byte hex array, ex: 00112233445566778899aabbccddeeff\n");
                return BT_STATUS_FAIL;
            }
            bt_mesh_app_util_str2u8HexNumArray(argv[3], key);

            bt_mesh_app_util_str2u16HexNum(argv[4], &appidx);
            bt_mesh_app_util_str2u16HexNum(argv[5], &netidx);

            ret = bt_mesh_config_add_application_key(key, appidx, netidx);
            if (ret) {
                LOG_I(mesh_app, "ERROR!!!! [%02x]\n", ret);
            }
        } else if (!strcmp(argv[2], "dev")) {
            if (argc < 5) {
                LOG_I(mesh_app, "USAGE: key add dev <devkey> <address>\n");
                return BT_STATUS_FAIL;
            }
            uint8_t devkey[BT_MESH_DEVKEY_SIZE] = {0};
            uint16_t dst_addr;

            bt_mesh_app_util_str2u16HexNum(argv[3], &dst_addr);
            bt_mesh_app_util_str2u8HexNumArray(argv[4], devkey);
            LOG_I(mesh_app, "status = %d\n", bt_mesh_config_add_device_key(devkey, dst_addr));
        }
        return BT_STATUS_SUCCESS;
    } else if (!strcmp(argv[1], "update")) {
        if (!strcmp(argv[2], "net")) {
            if (argc < 5) {
                LOG_I(mesh_app, "USAGE: key update net <netkey> <keyidx>\n");
                return BT_STATUS_FAIL;
            }

            uint8_t ret, key[BT_MESH_NETKEY_SIZE];
            uint16_t keyidx;

            if (strlen(argv[3]) != 2 * BT_MESH_NETKEY_SIZE) {
                LOG_I(mesh_app, "<netkey> is a 16-byte hex array, ex: 00112233445566778899aabbccddeeff\n");
                return BT_STATUS_FAIL;
            }
            bt_mesh_app_util_str2u8HexNumArray(argv[3], key);
            bt_mesh_app_util_str2u16HexNum(argv[4], &keyidx);
            ret = bt_mesh_config_update_network_key(key, keyidx);
            LOG_I(mesh_app, "ret = %02x\n", ret);
        } else if (!strcmp(argv[2], "app")) {
            if (argc < 6) {
                LOG_I(mesh_app, "USAGE: key update app <appkey> <appidx> <netidx>\n");
                return BT_STATUS_FAIL;
            }

            uint8_t ret, key[BT_MESH_APPKEY_SIZE];
            uint16_t appidx, netidx;

            if (strlen(argv[3]) != 2 * BT_MESH_APPKEY_SIZE) {
                LOG_I(mesh_app, "<appkey> is a 16-byte hex array, ex: 00112233445566778899aabbccddeeff\n");
                return BT_STATUS_FAIL;
            }
            bt_mesh_app_util_str2u8HexNumArray(argv[3], key);
            bt_mesh_app_util_str2u16HexNum(argv[4], &appidx);
            bt_mesh_app_util_str2u16HexNum(argv[5], &netidx);

            ret = bt_mesh_config_update_application_key(key, appidx, netidx);

            if (ret) {
                LOG_I(mesh_app, "ERROR!!!! [%02x]\n", ret);
            }
        } else if (!strcmp(argv[2], "dev")) {
            if (argc < 4) {
                LOG_I(mesh_app, "USAGE: key update dev <devkey>\n");
                return BT_STATUS_FAIL;
            }

            uint8_t key[BT_MESH_DEVKEY_SIZE];

            if (BT_MESH_DEVKEY_SIZE != bt_mesh_app_util_str2u8HexNumArray(argv[3], key)) {
                return BT_STATUS_FAIL;
            }

            bt_mesh_config_set_device_key(key);
        }
        return BT_STATUS_SUCCESS;
    }
    return BT_STATUS_FAIL;
}


#if 0
static bt_status_t cmd_msg_config(void *no, uint16_t argc, char **argv)
{
    if (argc < 2) {
        return BT_STATUS_FAIL;
    }

    if (!strcmp(argv[1], "getcomp")) {
        if (argc < 3) {
            LOG_I(mesh_app, "USAGE: msg config getcomp <dst_addr>\n");
            return BT_STATUS_FAIL;
        }

        gConfigureRetry = MESH_CONFIGURE_RETRY_MAX;
        gSwitchState = SWITCH_SETTING_STATE_PROVISIONED;
        uint16_t dst_addr = 0;
        bt_mesh_app_util_str2u16HexNum(argv[2], &dst_addr);
        g_prov_current_addr = dst_addr;
        msg_config_get_composition(dst_addr);
    } else if (!strcmp(argv[1], "addappkey")) {
        if (argc < 3) {
            LOG_I(mesh_app, "USAGE: msg config addappkey <dst_addr>\n");
            return BT_STATUS_FAIL;
        }
        gConfigureRetry = MESH_CONFIGURE_RETRY_MAX;
        gSwitchState = SWITCH_SETTING_STATE_GET_COMPOSITION_DONE;
        uint16_t dst_addr = 0;
        bt_mesh_app_util_str2u16HexNum(argv[2], &dst_addr);
        g_prov_current_addr = dst_addr;
        msg_config_add_appkey(g_appkey_index, g_prov_current_netidx,
                              g_application_key, dst_addr);
    } else if (!strcmp(argv[1], "bindmodel")) {
        if (argc < 3) {
            LOG_I(mesh_app, "USAGE: msg config bindmodel <dst_addr>\n");
            return BT_STATUS_FAIL;
        }

        if (binding_model_list != NULL && bt_mesh_os_layer_ds_queue_count(binding_model_list)) {
            gConfigureRetry = MESH_CONFIGURE_RETRY_MAX;
            gSwitchState = SWITCH_SETTING_STATE_ADD_APPKEY_DONE;
            switch_mesh_binding_model_t *entry = bt_mesh_os_layer_ds_queue_first(binding_model_list);
            if (entry != NULL) {
                uint16_t dst_addr = 0;
                bt_mesh_app_util_str2u16HexNum(argv[2], &dst_addr);
                g_prov_current_addr = dst_addr;
                msg_config_bind_model(
                    dst_addr + entry->element_index, g_appkey_index, dst_addr, entry->model_id);
            } else {
                LOG_I(mesh_app, "No more model needs to be boud.\n");
            }
        }
    } else if (!strcmp(argv[1], "nodereset")) {
        if (argc < 3) {
            LOG_I(mesh_app, "USAGE: msg config nodereset <dst_addr>\n");
            return BT_STATUS_FAIL;
        }
        uint16_t dst_addr = 0;
        bt_mesh_app_util_str2u16HexNum(argv[2], &dst_addr);
        msg_config_node_reset(dst_addr);
    } else if (!strcmp(argv[1], "publish")) {
        if (argc < 6) {
            LOG_I(mesh_app, "USAGE: msg config publish <action> <dst_addr> <element_addr> <publish_addr> <model_id> [<period> <count> <interval>]\n");
            LOG_I(mesh_app, "USAGE: msg config publish 2 <dst_addr> <element_addr> <model_id>\n");
            return BT_STATUS_FAIL;
        }

        uint16_t dst_addr, element_addr;
        uint32_t model_id;

        switch (atoi(argv[2])) {
            case 0: { // set
                bt_mesh_model_publication_param_t publish_params = {0};
                uint16_t pub_addr;
                bt_mesh_app_util_str2u16HexNum(argv[3], &dst_addr);
                bt_mesh_app_util_str2u16HexNum(argv[4], &element_addr);
                bt_mesh_app_util_str2u16HexNum(argv[5], &pub_addr);
                bt_mesh_app_util_str2u32HexNum(argv[6], &model_id);
                publish_params.element_address = element_addr;
                publish_params.publish_address.type = bt_mesh_utils_get_addr_type(pub_addr);
                publish_params.publish_address.value = pub_addr;
                publish_params.publish_address.virtual_uuid = NULL;
                publish_params.appkey_index = g_appkey_index;
                publish_params.friendship_credential_flag = false;
                publish_params.publish_ttl = 0;
                publish_params.model_id = model_id;

                if (argc == 10) {
                    bt_mesh_app_util_str2u8HexNum(argv[7], &publish_params.publish_period);
                    bt_mesh_app_util_str2u8HexNum(argv[8], &publish_params.retransmit_count);
                    bt_mesh_app_util_str2u8HexNum(argv[9], &publish_params.retransmit_interval_steps);
                }

                if (publish_params.retransmit_interval_steps > 0x1F) {
                    LOG_I(mesh_app, "retransmit interval steps should be 0 ~ 0x1F\n");
                    return BT_STATUS_FAIL;
                }

                if (publish_params.retransmit_count > 0x7) {
                    LOG_I(mesh_app, "retransmit count should be 0 ~ 0x07\n");
                    return BT_STATUS_FAIL;
                }

                msg_config_set_publication(dst_addr, &publish_params);
                break;
            }
            case 1: { // set virtual
                bt_mesh_model_publication_param_t publish_params = {0};
                uint8_t uuid[BT_MESH_UUID_SIZE] = {0};
                bt_mesh_app_util_str2u16HexNum(argv[3], &dst_addr);
                bt_mesh_app_util_str2u16HexNum(argv[4], &element_addr);
                bt_mesh_app_util_str2u8HexNumArray(argv[5], uuid);
                bt_mesh_app_util_str2u32HexNum(argv[6], &model_id);
                publish_params.element_address = element_addr;
                publish_params.publish_address.type = BT_MESH_ADDRESS_TYPE_VIRTUAL;
                publish_params.publish_address.value = bt_mesh_utils_get_virtual_address(uuid);
                publish_params.publish_address.virtual_uuid = uuid;
                publish_params.appkey_index = g_appkey_index;
                publish_params.friendship_credential_flag = false;
                publish_params.publish_ttl = 0;
                publish_params.model_id = model_id;

                if (argc == 10) {
                    bt_mesh_app_util_str2u8HexNum(argv[7], &publish_params.publish_period);
                    bt_mesh_app_util_str2u8HexNum(argv[8], &publish_params.retransmit_count);
                    bt_mesh_app_util_str2u8HexNum(argv[9], &publish_params.retransmit_interval_steps);
                }

                if (publish_params.retransmit_count > 0x1F) {
                    LOG_I(mesh_app, "retransmit count should be 0 ~ 0x1F\n");
                    return BT_STATUS_FAIL;
                }

                if (publish_params.retransmit_interval_steps > 0x7) {
                    LOG_I(mesh_app, "retransmit interval steps should be 0 ~ 0x07\n");
                    return BT_STATUS_FAIL;
                }

                msg_config_set_publication(dst_addr, &publish_params);
                break;
            }
            case 2: { // get
                bt_mesh_app_util_str2u16HexNum(argv[3], &dst_addr);
                bt_mesh_app_util_str2u16HexNum(argv[4], &element_addr);
                bt_mesh_app_util_str2u32HexNum(argv[5], &model_id);

                msg_config_get_publication(dst_addr, element_addr, model_id);
                break;
            }
        }
    } else if (!strcmp(argv[1], "subscribe")) {
        if (argc < 7) {
            LOG_I(mesh_app, "USAGE: msg config subscribe <action> <dst_addr> <element_addr> <subscribe_addr> <model_id>\n");
        }
        uint16_t dst_addr, element_addr, sub_addr;
        uint32_t model_id;

        bt_mesh_app_util_str2u16HexNum(argv[3], &dst_addr);
        bt_mesh_app_util_str2u16HexNum(argv[4], &element_addr);
        bt_mesh_app_util_str2u16HexNum(argv[5], &sub_addr);
        bt_mesh_app_util_str2u32HexNum(argv[6], &model_id);

        msg_config_handle_subscription(atoi(argv[2]), dst_addr, element_addr, sub_addr, model_id);
    }
    return BT_STATUS_SUCCESS;
}
#endif
static bt_status_t cmd_msg_config_beacon(void *no, uint16_t argc, char **argv)
{
    if (argc < 3) {
        LOG_I(mesh_app, "USAGE: msg config beacon <dst_addr> <enable>\n");
        return BT_STATUS_FAIL;
    }

    uint16_t dst_addr = 0;
    bt_mesh_app_util_str2u16HexNum(argv[1], &dst_addr);
    msg_config_set_beacon(dst_addr, atoi(argv[2]));
    return BT_STATUS_SUCCESS;
}

static bt_status_t cmd_msg_config_friend(void *no, uint16_t argc, char **argv)
{
    if (argc < 3) {
        LOG_I(mesh_app, "USAGE: msg config friend <dst_addr> <enable>\n");
        return BT_STATUS_FAIL;
    }

    uint16_t dst_addr = 0;
    bt_mesh_app_util_str2u16HexNum(argv[1], &dst_addr);
    msg_config_set_friend(dst_addr, atoi(argv[2]));
    return BT_STATUS_SUCCESS;
}

static bt_status_t cmd_msg_config_relay(void *no, uint16_t argc, char **argv)
{
    if (argc < 3) {
        LOG_I(mesh_app, "USAGE: msg config relay <dst_addr> <enable> [<retransmit_count> <retransmit_interval_steps>]\n");
        return BT_STATUS_FAIL;
    }

    uint16_t dst_addr = 0;
    uint8_t count = 0, steps = 0;
    bt_mesh_app_util_str2u16HexNum(argv[1], &dst_addr);
    if (argc == 5) {
        bt_mesh_app_util_str2u8HexNum(argv[3], &count);
        bt_mesh_app_util_str2u8HexNum(argv[4], &steps);        
    }
    msg_config_set_relay(dst_addr, atoi(argv[2]), count, steps);
    return BT_STATUS_SUCCESS;
}

static bt_status_t cmd_msg_config_ni(void *no, uint16_t argc, char **argv)
{
    if (argc < 4) {
        LOG_I(mesh_app, "USAGE: msg config ni <dst_addr> <enable> <keyidx>\n");
        return BT_STATUS_FAIL;
    }

    uint16_t dst_addr = 0, keyidx = 0;
    bt_mesh_app_util_str2u16HexNum(argv[1], &dst_addr);
    bt_mesh_app_util_str2u16HexNum(argv[2], &keyidx);
    msg_config_set_ni(dst_addr, keyidx, atoi(argv[2]));
    return BT_STATUS_SUCCESS;
}

static bt_status_t cmd_msg_config_proxy(void *no, uint16_t argc, char **argv)
{
    if (argc < 3) {
        LOG_I(mesh_app, "USAGE: msg config proxy <dst_addr> <enable>\n");
        return BT_STATUS_FAIL;
    }

    uint16_t dst_addr = 0;
    bt_mesh_app_util_str2u16HexNum(argv[1], &dst_addr);
    msg_config_set_proxy(dst_addr, atoi(argv[2]));
    return BT_STATUS_SUCCESS;
}

static bt_status_t cmd_msg_config_ttl(void *no, uint16_t argc, char **argv)
{
    if (argc < 3) {
        LOG_I(mesh_app, "USAGE: msg config ttl <dst_addr> <ttl>\n");
        return BT_STATUS_FAIL;
    }

    uint16_t dst_addr = 0;
    uint8_t ttl;
    bt_mesh_app_util_str2u16HexNum(argv[1], &dst_addr);
    bt_mesh_app_util_str2u8HexNum(argv[2], &ttl);
    msg_config_set_default_ttl(dst_addr, ttl);
    return BT_STATUS_SUCCESS;
}

static bt_status_t cmd_msg_config_nettransmit(void *no, uint16_t argc, char **argv)
{
    if (argc < 3) {
        LOG_I(mesh_app, "USAGE: msg config nettransmit <dst_addr> <count> <interval_steps>\n");
        return BT_STATUS_FAIL;
    }

    uint16_t dst_addr = 0;
    uint8_t count, interval_steps;
    bt_mesh_app_util_str2u16HexNum(argv[1], &dst_addr);
    bt_mesh_app_util_str2u8HexNum(argv[2], &count);
    bt_mesh_app_util_str2u8HexNum(argv[3], &interval_steps);
    if (count > 7) {
        LOG_I(mesh_app, "<count>: 0~7\n");
        return BT_STATUS_FAIL;
    }

    if (interval_steps > 31) {
        LOG_I(mesh_app, "<count>: 0~31\n");
        return BT_STATUS_FAIL;
    }
    msg_config_set_network_transmit(dst_addr, count, interval_steps);
    return BT_STATUS_SUCCESS;
}

static bt_status_t cmd_msg_config_getcomp(void *no, uint16_t argc, char **argv)
{
    if (argc < 2) {
        LOG_I(mesh_app, "USAGE: msg config getcomp <dst_addr>\n");
        return BT_STATUS_FAIL;
    }

    gConfigureRetry = MESH_CONFIGURE_RETRY_MAX;
    gSwitchState = SWITCH_SETTING_STATE_PROVISIONED;
    uint16_t dst_addr = 0;
    bt_mesh_app_util_str2u16HexNum(argv[1], &dst_addr);
    g_prov_current_addr = dst_addr;
    msg_config_get_composition(dst_addr);
    return BT_STATUS_SUCCESS;
}

/*static bt_status_t cmd_msg_config_addappkey(void *no, uint16_t argc, char **argv)
{
    if (argc < 2) {
        LOG_I(mesh_app, "USAGE: msg config addappkey <dst_addr>\n");
        return BT_STATUS_FAIL;
    }
    gConfigureRetry = MESH_CONFIGURE_RETRY_MAX;
    gSwitchState = SWITCH_SETTING_STATE_GET_COMPOSITION_DONE;
    uint16_t dst_addr = 0;
    bt_mesh_app_util_str2u16HexNum(argv[1], &dst_addr);
    g_prov_current_addr = dst_addr;
    msg_config_add_appkey(g_appkey_index, g_prov_current_netidx,
                          g_application_key, dst_addr);
    return BT_STATUS_SUCCESS;
}

static bt_status_t cmd_msg_config_bindmodel(void *no, uint16_t argc, char **argv)
{
    if (argc < 2) {
        LOG_I(mesh_app, "USAGE: msg config bindmodel <dst_addr>\n");
        return BT_STATUS_FAIL;
    }

    if (binding_model_list != NULL && bt_mesh_os_layer_ds_queue_count(binding_model_list)) {
        gConfigureRetry = MESH_CONFIGURE_RETRY_MAX;
        gSwitchState = SWITCH_SETTING_STATE_ADD_APPKEY_DONE;
        switch_mesh_binding_model_t *entry = bt_mesh_os_layer_ds_queue_first(binding_model_list);
        if (entry != NULL) {
            uint16_t dst_addr = 0;
            bt_mesh_app_util_str2u16HexNum(argv[1], &dst_addr);
            g_prov_current_addr = dst_addr;
            msg_config_bind_model(
                dst_addr + entry->element_index, g_appkey_index, dst_addr, entry->model_id);
        } else {
            LOG_I(mesh_app, "No more model needs to be boud.\n");
        }
    }
    return BT_STATUS_SUCCESS;
}*/

static bt_status_t cmd_msg_config_nodereset(void *no, uint16_t argc, char **argv)
{
    if (argc < 2) {
        LOG_I(mesh_app, "USAGE: msg config nodereset <dst_addr>\n");
        return BT_STATUS_FAIL;
    }
    uint16_t dst_addr = 0;
    bt_mesh_app_util_str2u16HexNum(argv[1], &dst_addr);
    msg_config_node_reset(dst_addr);
    return BT_STATUS_SUCCESS;
}

static bt_status_t cmd_msg_config_publish(void *no, uint16_t argc, char **argv)
{
    if (argc < 2) {
        LOG_I(mesh_app, "USAGE: msg config publish <action> <dst_addr> <element_addr> <publish_addr> <model_id> <publish_ttl> [<period> <count> <interval>]\n");
        return BT_STATUS_FAIL;
    }

    uint16_t dst_addr, element_addr;
    uint32_t model_id;

    switch (atoi(argv[1])) {
        case 0: { // set
            if (argc < 7) {
                LOG_I(mesh_app,"USAGE: msg config publish 0 <dst_addr> <element_addr> <publish_addr> <model_id> <publish_ttl> [<period> <count> <interval>]\n");
                return BT_STATUS_FAIL;
            }
            bt_mesh_model_publication_param_t publish_params = {0};
            uint16_t pub_addr;
            uint8_t publish_ttl;
            bt_mesh_app_util_str2u16HexNum(argv[2], &dst_addr);
            bt_mesh_app_util_str2u16HexNum(argv[3], &element_addr);
            bt_mesh_app_util_str2u16HexNum(argv[4], &pub_addr);
            bt_mesh_app_util_str2u32HexNum(argv[5], &model_id);
            bt_mesh_app_util_str2u8HexNum(argv[6], &publish_ttl);
            publish_params.element_address = element_addr;
            publish_params.publish_address.type = bt_mesh_utils_get_addr_type(pub_addr);
            publish_params.publish_address.value = pub_addr;
            publish_params.publish_address.virtual_uuid = NULL;
            publish_params.appkey_index = g_appkey_index;
            publish_params.friendship_credential_flag = false;
            publish_params.publish_ttl = publish_ttl;
            publish_params.model_id = model_id;

            if (argc == 10) {
                bt_mesh_app_util_str2u8HexNum(argv[7], &publish_params.publish_period);
                bt_mesh_app_util_str2u8HexNum(argv[8], &publish_params.retransmit_count);
                bt_mesh_app_util_str2u8HexNum(argv[9], &publish_params.retransmit_interval_steps);
            }
            
            if (publish_params.publish_ttl > BT_MESH_TTL_MAX) {
                LOG_I(mesh_app, "invalid publish TTL\n");
                return BT_STATUS_FAIL;
            }

            if (publish_params.retransmit_interval_steps > 0x1F) {//5bits
                LOG_I(mesh_app, "retransmit interval steps should be 0 ~ 0x1F\n");
                return BT_STATUS_FAIL;
            }

            if (publish_params.retransmit_count > 0x7) {//3bits
                LOG_I(mesh_app, "retransmit count should be 0 ~ 0x07\n");
                return BT_STATUS_FAIL;
            }

            msg_config_set_publication(dst_addr, &publish_params);
            break;
        }
        case 1: { // set virtual
            if (argc < 7) {
                LOG_I(mesh_app,"USAGE: msg config publish 1 <dst_addr> <element_addr> <publish_addr> <model_id> <publish_ttl> [<period> <count> <interval>]\n");
                return BT_STATUS_FAIL;
            }
            bt_mesh_model_publication_param_t publish_params = {0};
            uint8_t uuid[BT_MESH_UUID_SIZE] = {0};
            uint8_t publish_ttl;
            bt_mesh_app_util_str2u16HexNum(argv[2], &dst_addr);
            bt_mesh_app_util_str2u16HexNum(argv[3], &element_addr);
            bt_mesh_app_util_str2u8HexNumArray(argv[4], uuid);
            bt_mesh_app_util_str2u32HexNum(argv[5], &model_id);
            bt_mesh_app_util_str2u8HexNum(argv[6], &publish_ttl);
            publish_params.element_address = element_addr;
            publish_params.publish_address.type = BT_MESH_ADDRESS_TYPE_VIRTUAL;
            publish_params.publish_address.value = bt_mesh_utils_get_virtual_address(uuid);
            publish_params.publish_address.virtual_uuid = uuid;
            publish_params.appkey_index = g_appkey_index;
            publish_params.friendship_credential_flag = false;
            publish_params.publish_ttl = publish_ttl;
            publish_params.model_id = model_id;

            if (argc == 10) {
                bt_mesh_app_util_str2u8HexNum(argv[7], &publish_params.publish_period);
                bt_mesh_app_util_str2u8HexNum(argv[8], &publish_params.retransmit_count);
                bt_mesh_app_util_str2u8HexNum(argv[9], &publish_params.retransmit_interval_steps);
            }
            
            if (publish_params.publish_ttl > BT_MESH_TTL_MAX) {
                LOG_I(mesh_app, "invalid publish TTL\n");
                return BT_STATUS_FAIL;
            }

            if (publish_params.retransmit_interval_steps > 0x1F) {//5bits
                LOG_I(mesh_app, "retransmit interval steps should be 0 ~ 0x1F\n");
                return BT_STATUS_FAIL;
            }

            if (publish_params.retransmit_count > 0x7) {//3bits
                LOG_I(mesh_app, "retransmit count should be 0 ~ 0x07\n");
                return BT_STATUS_FAIL;
            }

            msg_config_set_publication(dst_addr, &publish_params);
            break;
        }
        case 2: { // get
            if (argc < 5) {
                LOG_I(mesh_app,"USAGE: msg config publish 2 <dst_addr> <element_addr> <model_id>\n");
                return BT_STATUS_FAIL;
            }
            bt_mesh_app_util_str2u16HexNum(argv[2], &dst_addr);
            bt_mesh_app_util_str2u16HexNum(argv[3], &element_addr);
            bt_mesh_app_util_str2u32HexNum(argv[4], &model_id);

            msg_config_get_publication(dst_addr, element_addr, model_id);
            break;
        }
    }
    return BT_STATUS_SUCCESS;
}

static bt_status_t cmd_msg_config_subscribe(void *no, uint16_t argc, char **argv)
{
    if (argc < 6) {
        LOG_I(mesh_app, "USAGE: msg config subscribe <action> <dst_addr> <element_addr> <subscribe_addr> <model_id>\n");
    }
    int addrlen = 0;
    uint8_t uuid[BT_MESH_UUID_SIZE];
    uint16_t dst_addr, element_addr, sub_addr;
    uint32_t model_id;

    bt_mesh_app_util_str2u16HexNum(argv[2], &dst_addr);
    bt_mesh_app_util_str2u16HexNum(argv[3], &element_addr);
    bt_mesh_app_util_str2u32HexNum(argv[5], &model_id);
    addrlen = bt_mesh_app_util_str2u8HexNumArray(argv[4], uuid);
    if (addrlen != BT_MESH_UUID_SIZE) {
        bt_mesh_app_util_str2u16HexNum(argv[4], &sub_addr);
        msg_config_handle_subscription(atoi(argv[1]), dst_addr, element_addr, sub_addr, NULL, model_id);
    } else {
        sub_addr = bt_mesh_utils_get_virtual_address(uuid);
        msg_config_handle_subscription(atoi(argv[1]), dst_addr, element_addr, sub_addr, uuid, model_id);
    }
    return BT_STATUS_SUCCESS;
}

static bt_status_t cmd_msg_config_hb_publish(void *no, uint16_t argc, char **argv)
{
    if (argc < 3) {
        LOG_I(mesh_app,"USAGE: msg config hbpublish <action> <dst_addr> [<publish_dest> <count_log> <period_log> <publish_ttl> <publish_features> <publish_netidx>]\n");
        LOG_I(mesh_app,"<action> : 0 means set; 1 means get.\n");
        return BT_STATUS_FAIL;
    }

    uint16_t dst_addr;
    bt_mesh_app_util_str2u16HexNum(argv[2], &dst_addr);

    switch(atoi(argv[1])) {
        case 0: // set
        {
            if (argc < 4) {
                LOG_I(mesh_app,"USAGE: msg config hbpublish 0 <dst_addr> <publish_dest> [<count_log> <period_log> <publish_ttl> <publish_features> <publish_netidx>]\n");
                return BT_STATUS_FAIL;
            }
            bt_mesh_heartbeat_publication_param_t publication = {0};
            uint32_t tmp = 0;
            bt_mesh_app_util_str2u16HexNum(argv[3], &publication.destination);

            if (argc == 9) {
                tmp = atoi(argv[4]);
                if (tmp != 0xff && tmp > 0x12 ) {
                    LOG_I(mesh_app,"count log 0x12 ~ 0xFE is prohibited.\n");
                    return BT_STATUS_FAIL;
                }
                publication.count_log = (uint8_t)tmp;

                tmp = atoi(argv[5]);
                if (tmp > 0x12 ) {
                    LOG_I(mesh_app,"period log 0x12 ~ 0xFF is prohibited.\n");
                    return BT_STATUS_FAIL;
                }
                publication.period_log = (uint8_t)tmp;

                tmp = atoi(argv[6]);
                if (tmp > BT_MESH_TTL_MAX) {
                    LOG_I(mesh_app,"invalid publish TTL\n");
                    return BT_STATUS_FAIL;
                }
                publication.ttl = (uint8_t)tmp;
                bt_mesh_app_util_str2u16HexNum(argv[7], &publication.features);
                bt_mesh_app_util_str2u16HexNum(argv[8], &publication.netkey_index);
            }
            
            msg_config_set_heartbeat_publication(dst_addr, &publication);
            break;
        }
        case 1: // get
        {
            msg_config_get_heartbeat_publication(dst_addr);
            break;
        }
    }


    return BT_STATUS_SUCCESS;
}

static bt_status_t cmd_msg_config_hb_subscribe(void *no, uint16_t argc, char **argv)
{
    if (argc < 3) {
msg_config_hb_subscribe:
        LOG_I(mesh_app,"USAGE: msg config hbsubscribe <action> <dst_addr> [<subs_src> <subs_dst> <period_log>]\n");
        LOG_I(mesh_app,"<action> : 0 means set; 1 means get.\n");
        return BT_STATUS_FAIL;
    }

    uint16_t dst_addr;
    bt_mesh_app_util_str2u16HexNum(argv[2], &dst_addr);

    switch(atoi(argv[1])) {
        case 0: // set
        {
            if (argc < 6) {
                goto msg_config_hb_subscribe;
            }

            bt_mesh_heartbeat_subscription_param_t subscription;
            uint32_t tmp = 0;
            bt_mesh_app_util_str2u16HexNum(argv[3], &subscription.source);
            bt_mesh_app_util_str2u16HexNum(argv[4], &subscription.destination);
            tmp = atoi(argv[5]);
            if (tmp > 0x12 ) {
                LOG_I(mesh_app,"period log 0x12 ~ 0xFF is prohibited.\n");
                return BT_STATUS_FAIL;
            }
            subscription.period_log = (uint8_t)tmp;

            msg_config_set_heartbeat_subscription(dst_addr, &subscription);
            break;
        }
        case 1: // get
        {
            msg_config_get_heartbeat_subscription(dst_addr);
            break;
        }
    }

    bt_mesh_app_util_str2u16HexNum(argv[2], &dst_addr);
    return BT_STATUS_SUCCESS;
}

const mesh_app_callback_table_t cmd_msg_config_tbl[] = {
    {"beacon", cmd_msg_config_beacon},
    {"ttl", cmd_msg_config_ttl},
    {"nettransmit", cmd_msg_config_nettransmit},
    {"ni", cmd_msg_config_ni},
    {"proxy", cmd_msg_config_proxy},
    {"friend", cmd_msg_config_friend},
    {"relay", cmd_msg_config_relay},
    {"getcomp", cmd_msg_config_getcomp},
    {"nodereset", cmd_msg_config_nodereset},
    {"publish", cmd_msg_config_publish},
    {"subscribe", cmd_msg_config_subscribe},
    {"hbpublish", cmd_msg_config_hb_publish},
    {"hbsubscribe", cmd_msg_config_hb_subscribe},
};

static bt_status_t cmd_msg_config(void *no, uint16_t argc, char **argv)
{
    uint16_t i;
    for (i = 0; i < sizeof(cmd_msg_config_tbl) / sizeof(mesh_app_callback_table_t); i++) {
        if (!strcmp(cmd_msg_config_tbl[i].name, argv[1])) {
            return cmd_msg_config_tbl[i].io_callback(no, argc - 1, argv + 1);
        }
    }
    return BT_STATUS_FAIL;
}

static bt_status_t cmd_msg_onoff(void *no, uint16_t argc, char **argv)
{
    if (argc < 2) {
msg_onoff_exit:
        LOG_I(mesh_app, "USAGE: msg onoff set <dst_addr> <onoff> <reliable> [<ttl>]\n");
        LOG_I(mesh_app, "USAGE: msg onoff get <dst_addr> [<ttl>]\n");
        return BT_STATUS_FAIL;
    }

    if (!strcmp(argv[1], "set")) {
        if (argc < 5) {
            goto msg_onoff_exit;
        }
        uint8_t ttl = bt_mesh_config_get_default_ttl();
        uint16_t dst_addr = 0;
        uint8_t onoff = atoi(argv[3]);
        uint8_t reliable = atoi(argv[4]);
        uint8_t uuid[BT_MESH_UUID_SIZE] = {0};
        int addrlen = 0;
        
        gIsChecking = false;

        if (argc == 6) {
            bt_mesh_app_util_str2u8HexNum(argv[5], &ttl);
        }
        
        addrlen = strlen(argv[2]);

        bt_mesh_app_util_str2u8HexNumArray(argv[2], uuid);
        if (addrlen == (BT_MESH_UUID_SIZE*2)) {
            msg_generic_on_off_set_uuid(uuid, ttl, onoff, gTid++, 0, 0, reliable);
        }
        else if (addrlen <= 6)
        {
            bt_mesh_app_util_str2u16HexNum(argv[2], &dst_addr);

            msg_generic_on_off_set(dst_addr, ttl, onoff, gTid++, 0, 0, reliable);
        }  
        else
        {
            LOG_I(mesh_app,"addrlen = %d\n", addrlen);
            return BT_STATUS_FAIL;
        }  
    } else if (!strcmp(argv[1], "get")) {
        uint8_t ttl = bt_mesh_config_get_default_ttl();
        uint16_t dst_addr;
        if (argc < 3) {
            goto msg_onoff_exit;
        } else {
            bt_mesh_app_util_str2u16HexNum(argv[2], &dst_addr);
            if (argc == 4) {
                bt_mesh_app_util_str2u8HexNum(argv[3], &ttl);
            }
        }
        msg_generic_on_off_get(dst_addr, ttl);
    }

    return BT_STATUS_SUCCESS;
}

static bt_status_t cmd_msg_level(void *no, uint16_t argc, char **argv)
{
    if (argc < 2) {
msg_level_exit:
        LOG_I(mesh_app, "USAGE: msg level set <dst_addr> <level> <reliable> [<ttl>]\n");
        LOG_I(mesh_app, "USAGE: msg level get <dst_addr> [<ttl>]\n");
        return BT_STATUS_FAIL;
    }

    if (!strcmp(argv[1], "set")) {
        if (argc < 5) {
            LOG_I(mesh_app, "USAGE: msg level set <dst_addr> <level> <reliable> [<ttl>]\n");
            return BT_STATUS_FAIL;
        }
        uint8_t ttl = bt_mesh_config_get_default_ttl();
        uint16_t dst_addr = 0;
        int32_t level = atoi(argv[3]);
        uint8_t reliable = atoi(argv[4]);

        if (level > 32767 || level < -32768) {
            LOG_I(mesh_app, "invalid level! USAGE: -32768 <= level <= 32767\n");
            return BT_STATUS_FAIL;
        }
        bt_mesh_app_util_str2u16HexNum(argv[2], &dst_addr);
        if (argc == 6) {
            bt_mesh_app_util_str2u8HexNum(argv[5], &ttl);
        }
        msg_generic_level_set(dst_addr, ttl, (int16_t)level, gTid++, 0, 0, reliable);
    } else if (!strcmp(argv[1], "get")) {
        uint8_t ttl = bt_mesh_config_get_default_ttl();
        uint16_t dst_addr;
        if (argc < 3) {
            goto msg_level_exit;
        } else {
            bt_mesh_app_util_str2u16HexNum(argv[2], &dst_addr);
            if (argc == 4) {
                bt_mesh_app_util_str2u8HexNum(argv[3], &ttl);
            }
        }
        msg_generic_level_get(dst_addr, ttl);
    }

    return BT_STATUS_SUCCESS;
}

/*yaming start*/
static bt_status_t cmd_msg_light_set(void *no, uint16_t argc, char **argv)
{
    if (argc < 2) {
        //LOG_I(mesh_app, "USAGE: msg light <hsl/ctl> <set/get> <dst_addr> <onoff> <reliable> [<ttl>]\n");
        LOG_I(mesh_app, "USAGE: msg light set hsl/ctl/lightness \n");
        return BT_STATUS_FAIL;
    }

    uint8_t ttl = bt_mesh_config_get_default_ttl();

    if (!strcmp(argv[1], "hsl")) {

        if (argc < 6) {
            LOG_I(mesh_app, "USAGE: msg light set hsl <dst_addr> <lightness> <hue> <saturation> [<ttl>]\n");
            return BT_STATUS_FAIL;
        }
        bool ret = false;
        uint16_t dst_addr;
        uint32_t lightness, hue, saturation;

        ret = bt_mesh_app_util_str2u16HexNum(argv[2], &dst_addr);
        ret &= bt_mesh_app_util_str2u32HexNum(argv[3], &lightness);
        if (!ret || lightness > 65535 || lightness < 0) {
            LOG_I(mesh_app, "lightness should be between 0x0000 and 0xffff\n");
            return BT_STATUS_FAIL;
        }
        ret &= bt_mesh_app_util_str2u32HexNum(argv[4], &hue);
        if (!ret || hue > 65535 || hue < 0) {
            LOG_I(mesh_app, "hue should be between 0x0000 and 0xffff\n");
            return BT_STATUS_FAIL;
        }
        ret &= bt_mesh_app_util_str2u32HexNum(argv[5], &saturation);
        if (!ret || saturation > 65535 || saturation < 0) {
            LOG_I(mesh_app, "saturation should be between 0x0000 and 0xffff\n");
            return BT_STATUS_FAIL;
        }
        if (argc == 7) {
            bt_mesh_app_util_str2u8HexNum(argv[6], &ttl);
            if (ttl > BT_MESH_TTL_MAX) {
                LOG_I(mesh_app, "inavlid TTL value\n");
                return BT_STATUS_FAIL;
            }
        }

        if (!ret) {
            return BT_STATUS_FAIL;
        }

        LOG_I(mesh_app, "cmd_msg_light_hsl:addr:0x%04x,lightness:0x%04x, hue:0x%04x, saturation:0x%04x,\n", dst_addr, lightness, hue, saturation);

        msg_Light_Hsl_Set(g_model_handle_light_hsl_client, dst_addr, (uint16_t)lightness, (uint16_t)hue, (uint16_t)saturation, gTid++, 0, 0, true, ttl);
        return BT_STATUS_SUCCESS;
    } else if (!strcmp(argv[1], "ctl")) {

        if (argc < 6) {
            LOG_I(mesh_app, "USAGE: msg light set ctl <dst_addr> <lightness> <temperature> <deltaUV> [<ttl>]\n");
            return BT_STATUS_FAIL;
        }
        bool ret = false;
        uint16_t dst_addr;
        int32_t deltaUV;
        uint32_t lightness, temperature;

        ret = bt_mesh_app_util_str2u16HexNum(argv[2], &dst_addr);
        ret &= bt_mesh_app_util_str2u32HexNum(argv[3], &lightness);
        if (!ret || lightness > 65535 || lightness < 0) {
            LOG_I(mesh_app, "lightness should be between 0x0000 and 0xffff\n");
            return BT_STATUS_FAIL;
        }
        
        ret &= bt_mesh_app_util_str2u32HexNum(argv[4], &temperature);
        if (!ret || temperature < 0 || temperature > 65535) {
            LOG_I(mesh_app,"temperature should be between 0x0000 and 0xffff\n");
            return BT_STATUS_FAIL;
        }
        
        deltaUV = atoi(argv[5]);
        if (!ret || deltaUV < -32768 || deltaUV > 32767) {
            LOG_I(mesh_app, "deltaUV should be between -32768 and 32767\n");
            return BT_STATUS_FAIL;
        }
        if (argc == 7) {
            bt_mesh_app_util_str2u8HexNum(argv[6], &ttl);
            if (ttl > BT_MESH_TTL_MAX) {
                LOG_I(mesh_app, "inavlid TTL value\n");
                return BT_STATUS_FAIL;
            }
        }


        LOG_I(mesh_app, "cmd_msg_light_ctl:addr:0x%04x,lightness:0x%04x, temperature:0x%04x, deltaUV:%d,\n", dst_addr, lightness, temperature, deltaUV);

        msg_Light_Ctl_Set(g_model_handle_light_ctl_client, dst_addr, lightness, temperature, deltaUV, gTid++, 0, 0, true, ttl);
        return BT_STATUS_SUCCESS;
    } else if (!strcmp(argv[1], "lightness")) {

        if (argc < 4) {
            LOG_I(mesh_app, "USAGE: msg light set lightness <0xdst_node_addr> <0xlightness> [<ttl>]\n");
            return BT_STATUS_FAIL;
        }
        bool ret = false;
        uint16_t dst_addr;
        uint32_t lightness;

        ret = bt_mesh_app_util_str2u16HexNum(argv[2], &dst_addr);
        ret &= bt_mesh_app_util_str2u32HexNum(argv[3], &lightness);
        if (!ret || lightness > 65535 || lightness < 0) {
            LOG_I(mesh_app, "lightness should be between 0x0000 and 0xffff\n");
            return BT_STATUS_FAIL;
        }
        if (argc == 5) {
            bt_mesh_app_util_str2u8HexNum(argv[4], &ttl);
            if (ttl > BT_MESH_TTL_MAX) {
                LOG_I(mesh_app, "inavlid TTL value\n");
                return BT_STATUS_FAIL;
            }
        }

        if (!ret) {
            return BT_STATUS_FAIL;
        }
        LOG_I(mesh_app, "cmd_msg_light_lightness:addr:0x%04x,lightness:0x%04x \n", dst_addr, lightness);

        msg_Light_Lightness_Set(g_model_handle_light_lightness_client,
                                dst_addr, lightness, gTid++, 0, 0, true, ttl);
        return BT_STATUS_SUCCESS;
    }

    return BT_STATUS_FAIL;
}
/*yaming end*/

static bt_status_t cmd_msg_light_get(void *no, uint16_t argc, char **argv)
{
    if (argc < 2) {
        LOG_I(mesh_app, "USAGE: msg light get hsl/ctl/lightness <dst_addr> [<ttl>]\n");
        return BT_STATUS_FAIL;
    }

    bool ret = false;
    uint8_t ttl = bt_mesh_config_get_default_ttl();
    uint16_t dst_addr;

    if (!strcmp(argv[1], "hsl")) {

        if (argc < 3) {
            LOG_I(mesh_app, "USAGE: msg light get hsl <dst_addr> [<ttl>]\n");
            return BT_STATUS_FAIL;
        }

        ret = bt_mesh_app_util_str2u16HexNum(argv[2], &dst_addr);
        if (!ret) {
            return BT_STATUS_FAIL;
        }
        LOG_I(mesh_app, "cmd_msg_light_get_hsl:addr:0x%04x\n", dst_addr);

        msg_Light_Hsl_Get(g_model_handle_light_hsl_client, dst_addr, ttl);
        return BT_STATUS_SUCCESS;
    } else if (!strcmp(argv[1], "ctl")) {

        if (argc < 3) {
            LOG_I(mesh_app, "USAGE: msg light get ctl <dst_addr> [<ttl>]\n");
            return BT_STATUS_FAIL;
        }

        ret = bt_mesh_app_util_str2u16HexNum(argv[2], &dst_addr);
        if (!ret) {
            return BT_STATUS_FAIL;
        }
        LOG_I(mesh_app, "cmd_msg_light_get_ctl:addr:0x%04x\n", dst_addr);

        msg_Light_Ctl_Get(g_model_handle_light_ctl_client, dst_addr, ttl);
        return BT_STATUS_SUCCESS;
    } else if (!strcmp(argv[1], "lightness")) {

        if (argc < 3) {
            LOG_I(mesh_app, "USAGE: msg light get lightness <dst_addr> [<ttl>]\n");
            return BT_STATUS_FAIL;
        }

        ret = bt_mesh_app_util_str2u16HexNum(argv[2], &dst_addr);
        if (!ret) {
            return BT_STATUS_FAIL;
        }
        LOG_I(mesh_app, "cmd_msg_light_get_lightness:addr:0x%04x\n", dst_addr);

        msg_Light_Lightness_Get(g_model_handle_light_lightness_client, dst_addr, ttl);
        return BT_STATUS_SUCCESS;
    }

    return BT_STATUS_FAIL;
}


static bt_status_t cmd_msg_vendor(void *no, uint16_t argc, char **argv)
{
    if (argc < 5) {
        LOG_I(mesh_app, "USAGE: msg vendor <dst_addr> <company_id> <opcode> <len>\n");
        return BT_STATUS_FAIL;
    }

    uint16_t dst_addr = 0, datalen = 0;
    uint16_t company_id = 0;
    uint8_t opcode = 0;
    bt_mesh_app_util_str2u16HexNum(argv[1], &dst_addr);
    bt_mesh_app_util_str2u16HexNum(argv[2], &company_id);
    bt_mesh_app_util_str2u8HexNum(argv[3], &opcode);
    bt_mesh_app_util_str2u16HexNum(argv[4], &datalen);

    msg_vendor(company_id, opcode, dst_addr, datalen);

    return BT_STATUS_SUCCESS;
}

static void _rx_timer_post_handler(TimerHandle_t idx)
{
    if (MESH_RX_TIMER_ID == idx)
    {
        LOG_I(mesh_app, "rx timeout ==> resend\n");
        
        gAutoCount++;
        gTimes++;
        gTid++;

        _latencytest_check_send();
    }
    else if (MESH_THROUGHPUT_RX_TIMER_ID == idx)
    {
        LOG_I(mesh_app, "throughput rx timeout ==> resend\n");

        gAutoCount++;
        gTimes++;
        gTid++;

        _throughput_check_send();
    }
    else if (MESH_PACKETLOSS_RX_TIMER_ID == idx)
    {
        LOG_I(mesh_app, "packetloss rx timeout ==> resend\n");

        gTid++;

        _packetloss_check_send();
    }

}
static void _rx_timer_handler(TimerHandle_t idx)
{
    mesh_app_post_timeout_event(idx);
}

static void _rx_set_timer(TimerHandle_t handle, uint32_t ms)
{
    bt_mesh_app_util_start_timer(handle, ms);
}

static void _rx_release_timer(TimerHandle_t handle)
{
    bt_mesh_app_util_stop_timer(handle);
}

static void _latencytest_timer_handler(TimerHandle_t idx)
{
    mesh_app_post_timeout_event(idx);
}

static void _latencytest_check_send(void)
{
    if (gAutoCount > 0) {
        if (msg_op_send(VENDOR_OPCODE_1, gLatencyDst, gAutoTtl))
        {
            gAutoCount--;
            gCrtNumDst = gNumDst;
            gLatencyTickIdx = 0;
            gLatencyTickIdxHops = 0;
            gTxHopsIdx = 0;
            gRxHopsIdx = 0;
            _rx_set_timer(MESH_RX_TIMER_ID, gRxTimeout);
        }
        else
        {
            _latencytest_set_timer(gAutoTimeout);
        }        
    }
}

static void _latencytest_check_send_ex(void)
{
    gLatencyRspCmp = false;
    if (gAutoCount > 0) {
        if (msg_op_send(VENDOR_OPCODE_7, gLatencyDst, gAutoTtl))
        {
            gAutoCount--;
            gCrtNumDst = gNumDst;
            gLatencyTickIdx = 0;
            gLatencyTickIdxHops = 0;
            gTxHopsIdx = 0;
            gRxHopsIdx = 0;
            bt_mesh_app_util_start_timer_ext("mesh latency tx", &MESH_LATENCY_TX_TIMER_ID, false, gTxTimeout, _latencytest_timer_handler);
        }
        else
        {
            bt_mesh_app_util_start_timer_ext("mesh latency tx", &MESH_LATENCY_TX_TIMER_ID, false, gAutoTimeout, _latencytest_timer_handler);
        }
    }
}

static void _latencytest_send_next_timer_handler(TimerHandle_t idx)
{
    mesh_app_post_timeout_event(idx);
}

static void _packetloss_timer_handler(TimerHandle_t idx)
{
    mesh_app_post_timeout_event(idx);
}

static void _packetloss_check_send(void)
{
    if (gAutoCount > 0) {
        if (msg_packetloss_send(gLatencyDst, gAutoTtl))
        {
            gAutoCount--;
            gCrtNumDst = gNumDst;
            _rx_set_timer(MESH_PACKETLOSS_RX_TIMER_ID, gRxTimeout);
        }
        else
        {
            _packetloss_set_timer(gAutoTimeout);
        }
    } else {
        gState = STATE_IDLE;
        LOG_I(mesh_app, "packetloss test done: resend %d\n", gTid);
        _packetloss_release_timer();
        _rx_release_timer(MESH_PACKETLOSS_RX_TIMER_ID);
    }
}

static void _throughput_check_send(void)
{
    if (msg_throughput_send(gLatencyDst, gAutoTtl))
    {
        if (gAutoCount == gTimes)
        {
            bt_mesh_app_get_tick(&gSendCheckTick);
        }
        gAutoCount--;
        gCrtNumDst = gNumDst;
        _rx_set_timer(MESH_THROUGHPUT_RX_TIMER_ID, gRxTimeout);
    }
    else
    {
        _throughput_set_timer(10);
    }
} 

static void _throughput_check_rx(void)
{
    uint32_t tick;
    bt_mesh_app_get_tick(&tick);
    tick -= gSendCheckTick;
    LOG_I(mesh_app, "throughput test done(round %d)\n", gAvgTimes - gCrtCnt);
    LOG_I(mesh_app, "********************************\n");
    if (gIsSmall)
    {
        LOG_I(mesh_app, "small PK throughput:  (%ld)Bytes/s\n", 5004 * 1000 / tick);
        gAvgThroughput += 5004 * 1000 / tick;
    }
    else
    {
        LOG_I(mesh_app, "large PK throughput:  (%ld)Bytes/s\n", 5320 * 1000 / tick);
        gAvgThroughput += 5320 * 1000 / tick;
    }
    LOG_I(mesh_app, "********************************\n");        

    if (gCrtCnt)
    {
        gCrtCnt--;
        gAutoCount = gTimes;
        _throughput_check_send();
        return;
    }

    gState = STATE_IDLE;
    LOG_I(mesh_app, "throughput test done(total times %d)\n", gAvgTimes);
    LOG_I(mesh_app, "********************************\n");
    gAvgThroughput /= gAvgTimes;
    if (gIsSmall)
    {
        LOG_I(mesh_app, "small PK avg. throughput:  (%ld)Bytes/s\n", gAvgThroughput);
    }
    else
    {
        LOG_I(mesh_app, "large PK avg. throughput:  (%ld)Bytes/s\n", gAvgThroughput);
    }
    LOG_I(mesh_app, "throughput test done: resend %d\n", gTid);
    _throughput_release_timer();
    _rx_release_timer(MESH_THROUGHPUT_RX_TIMER_ID);
}

static void _throughput_timer_handler(TimerHandle_t idx)
{
    mesh_app_post_timeout_event(idx);
}

static void _throughput_set_timer(uint32_t ms)
{
    bt_mesh_app_util_start_timer_ext("mesh throughput", &MESH_THROUGHPUT_TIMER_ID, false, ms, _throughput_timer_handler);
}

static void _throughput_release_timer(void)
{
    bt_mesh_app_util_stop_timer(MESH_THROUGHPUT_TIMER_ID);
}

static void _packetloss_set_timer(uint32_t ms)
{
    bt_mesh_app_util_start_timer_ext("mesh packetloss", &MESH_PACKETLOSS_TIMER_ID, false, ms, _packetloss_timer_handler);
}

static void _packetloss_release_timer(void)
{
    bt_mesh_app_util_stop_timer(MESH_PACKETLOSS_TIMER_ID);
}

static void _latencytest_set_timer(uint32_t ms)
{
    LOG_I(mesh_app, "_latencytest_set_timer");
    bt_mesh_app_util_start_timer_ext("mesh latency", &MESH_LATENCYTEST_TIMER_ID, false, ms, _latencytest_timer_handler);
}

static void _latencytest_release_timer(void)
{
    bt_mesh_app_util_stop_timer(MESH_LATENCYTEST_TIMER_ID);
}

static bool msg_send(uint16_t company_id, uint8_t opcode, uint16_t dst_addr, uint16_t datalen, uint8_t ttl, uint16_t idx)
{
    bt_mesh_tx_params_t param;
    
    param.dst.value = dst_addr;
    param.dst.type = bt_mesh_utils_get_addr_type(param.dst.value);
    param.src = bt_mesh_model_get_element_address(0);
    param.ttl = bt_mesh_config_get_default_ttl();  // default TTL value
    param.data_len = 6 + datalen; // total length of this packet

    if (param.data_len > 380) {
        param.data_len = 380;
        datalen = 374;
    }

    // set packet buffer, please refer to Mesh Profile Specification 3.7.3
    uint16_t i = 0;
    uint8_t *payload = malloc(param.data_len);
    if (!payload)
        return false;
    payload[0] = opcode;
    payload[1] = company_id & 0x00FF;
    payload[2] = (company_id & 0xFF00) >> 8;
    payload[3] = ttl;
    payload[4] = idx & 0xff;
    payload[5] = (idx>>8) & 0xff;
    for(i = 0; i < datalen ; i++) {
        payload[6+i] = i;
    }
    param.data = payload;
    param.security.appidx = g_appkey_index; // which application key used to send
    param.security.netidx = BT_MESH_GLOBAL_PRIMARY_NETWORK_KEY_INDEX;
    param.security.device_key = NULL;
    bt_mesh_app_get_tick(&gLatencyTick);
    bt_mesh_status_t ret = bt_mesh_send_packet(&param);

    LOG_I(mesh_app, "[T]Latency={dst=0x%04x,initTTL=0x%02x,idx=0x%x},result:%d",
           dst_addr, ttl, idx, ret);
    free(payload);
    return true;
}

static bool msg_throughput_send(uint16_t dst_addr, uint8_t ttl)
{
    LOG_I(mesh_app, "msg_throughput_send");

    if (gIsSmall)
    {
        return msg_send(VENDOR_COMPANY_ID, VENDOR_OPCODE_5, dst_addr, 0, ttl, gTimes-gAutoCount);
    }
    else
    {
        return msg_send(VENDOR_COMPANY_ID, VENDOR_OPCODE_6, dst_addr, 380, ttl, gTimes-gAutoCount);
    }
}


static bool msg_packetloss_send(uint16_t dst_addr, uint8_t ttl)
{
    LOG_I(mesh_app, "msg_packetloss_send");

    if (gIsSmall)
    {
        return msg_send(VENDOR_COMPANY_ID, VENDOR_OPCODE_3, dst_addr, 0, ttl, gTimes-gAutoCount);
    }
    else
    {
        return msg_send(VENDOR_COMPANY_ID, VENDOR_OPCODE_4, dst_addr, 380, ttl, gTimes-gAutoCount);
    }
}

static bool msg_op_send(uint8_t opcode, uint16_t dst_addr, uint8_t ttl)
{
    LOG_I(mesh_app, "opcode 0x%x send", opcode);

    if (gIsSmall)
    {
        return msg_send(VENDOR_COMPANY_ID, opcode, dst_addr, 0, ttl, gTimes-gAutoCount);
    }
    else
    {
        return msg_send(VENDOR_COMPANY_ID, opcode, dst_addr, 380, ttl, gTimes-gAutoCount);
    }
}

static bt_status_t cmd_throughput_test(void *no, uint16_t argc, char **argv)
{
    if (argc < 5) {
        LOG_I(mesh_app, "USAGE: throughput <dst_addr> <avg_times> <is_small_packet> <rx_timeout_ms> [<num_device>]");
        return BT_STATUS_FAIL;
    }

    bt_mesh_app_util_str2u16HexNum(argv[1], &gLatencyDst);
    bt_mesh_app_util_str2u32HexNum(argv[2], &gAvgTimes);
    bt_mesh_app_util_str2u32HexNum(argv[3], &gIsSmall);
    bt_mesh_app_util_str2u32HexNum(argv[4], &gRxTimeout);
    
    gCrtCnt = gAvgTimes;
    gCrtCnt--;
    if (gIsSmall)
    {
        gTimes = gAutoCount = 834; //(6 * 834 = 5004 > 5k)
    }else
    {
        gTimes = gAutoCount = 14; //(380 * 14 = 5320 > 5k)
    }

    gTid = 0;
    gAvgThroughput = 0;
    LOG_I(mesh_app, "start throughput test for %05d times!!, %d ms", gAutoCount, gRxTimeout);
    if (_isGroupDst())
    {
        if (argc < 6)
        {
            LOG_I(mesh_app, "USAGE: throughput <dst_addr> <is_small_packet> <rx_timeout_ms> <avg_times> <num_device>");
            return BT_STATUS_FAIL;
        }
        else
        {
            bt_mesh_app_util_str2u32HexNum(argv[5], &gNumDst);
        }
        LOG_I(mesh_app, "num_device %d!!", gNumDst);
    }
    else
    {
        gNumDst = 1;
    }
    gCrtNumDst = gNumDst;
    gState = STATE_BUSY;
    _throughput_check_send();
    return BT_STATUS_SUCCESS;
}

static bt_status_t cmd_stop(void *no, uint16_t argc, char **argv)
{
    _latencytest_release_timer();
    _rx_release_timer(MESH_RX_TIMER_ID);
    _packetloss_release_timer();
    _rx_release_timer(MESH_PACKETLOSS_RX_TIMER_ID);
    _throughput_release_timer();
    _rx_release_timer(MESH_THROUGHPUT_RX_TIMER_ID);
    bt_mesh_app_util_stop_timer(MESH_LATENCY_TX_TIMER_ID);
    gState = STATE_IDLE;
    LOG_I(mesh_app, "cmd stop!");
    return BT_STATUS_SUCCESS;
}

static bt_status_t cmd_packet_loss_test(void *no, uint16_t argc, char **argv)
{
    if (argc < 5) {
        LOG_I(mesh_app, "USAGE: packet_loss <dst_addr> <num_of_packet> <is_small_packet> <rx_timeout_ms> [<num_device>]");
        return BT_STATUS_FAIL;
    }

    bt_mesh_app_util_str2u16HexNum(argv[1], &gLatencyDst);
    if (_isGroupDst())
    {
        if (argc < 6)
        {
            LOG_I(mesh_app, "USAGE: packet_loss <dst_addr> <num_of_packet> <is_small_packet> <rx_timeout_ms> <num_device>");
            return BT_STATUS_FAIL;
        }
        else
        {
            bt_mesh_app_util_str2u32HexNum(argv[5], &gNumDst);
        }
        LOG_I(mesh_app, "num_device %d!!", gNumDst);
    }
    else
    {
        gNumDst = 1;
    }
    bt_mesh_app_util_str2u32HexNum(argv[2], &gAutoCount);
    bt_mesh_app_util_str2u32HexNum(argv[3], &gIsSmall);
    bt_mesh_app_util_str2u32HexNum(argv[4], &gRxTimeout);
    
    gTimes = gAutoCount;
    
    LOG_I(mesh_app, "start packet loss test for %05d times, %d ms!!", gAutoCount, gRxTimeout);
    
    gCrtNumDst = gNumDst;
    gTid = 0;
    gState = STATE_BUSY;
    _packetloss_check_send();
    return BT_STATUS_SUCCESS;
}

static void _latencytest_init(void)
{
    gLatencyTickIdx = 0;
    gLatencyTickAvg = 0;
    gLatencyTickAvgHops = 0;
    gLatencyTickIdxHops = 0;
    gTxHopsAvg = 0;
    gRxHopsAvg = 0;
    gTxHopsIdx = 0;
    gRxHopsIdx = 0;
    gNumDst = 1;
    gCrtNumDst = 1;
    gTid = 0;
}

static bt_status_t cmd_latencytest_start(void *no, uint16_t argc, char **argv)
{
    _latencytest_init();

    if (argc < 5) {
        LOG_I(mesh_app, "USAGE: latencytest start <dst_addr> <num_of_packet> <is_small_packet> <rx_timeout_ms> [<num_device>][<ttl>]");
        return BT_STATUS_FAIL;
    }

    bt_mesh_app_util_str2u16HexNum(argv[1], &gLatencyDst);
    if (_isGroupDst())
    {
        if (argc < 6)
        {
            LOG_I(mesh_app, "USAGE: latencytest start <dst_addr> <num_of_packet> <is_small_packet> <rx_timeout_ms> <num_device>");
            return BT_STATUS_FAIL;
        }
        else
        {
            bt_mesh_app_util_str2u32HexNum(argv[5], &gNumDst);
        }
        LOG_I(mesh_app, "num_device %d!!", gNumDst);
    }
    else
    {
        gNumDst = 1;
    }
    bt_mesh_app_util_str2u32HexNum(argv[2], &gAutoCount);
    bt_mesh_app_util_str2u32HexNum(argv[3], &gIsSmall);
    bt_mesh_app_util_str2u32HexNum(argv[4], &gRxTimeout);
    gAutoTimeout = 1000;

    if (argc == 7) {
        bt_mesh_app_util_str2u8HexNum(argv[6], &gAutoTtl);
    } else {
        gAutoTtl = bt_mesh_config_get_default_ttl();
    }
    gCrtNumDst = gNumDst;
    gTimes = gAutoCount;
    gState = STATE_BUSY;
    LOG_I(mesh_app, "start latencytest for %d times, ttl = %d !!", gAutoCount, gAutoTtl);
    _latencytest_check_send();

    return BT_STATUS_SUCCESS;
}

static bt_status_t cmd_latencytest_start_ex(void *no, uint16_t argc, char **argv)
{
    _latencytest_init();
    
    if (argc < 5) {
        LOG_I(mesh_app, "USAGE: latencytest start_ex <dst_addr> <num_of_packet> <is_small_packet> <tx_interval_ms> [<num_device>] [<ttl>]");
        return BT_STATUS_FAIL;
    }

    bt_mesh_app_util_str2u16HexNum(argv[1], &gLatencyDst);
    if (_isGroupDst())
    {
        if (argc < 6)
        {
            LOG_I(mesh_app, "USAGE: latencytest start_ex <dst_addr> <num_of_packet> <is_small_packet> <tx_interval_ms> <num_device>");
            return BT_STATUS_FAIL;
        }
        else
        {
            bt_mesh_app_util_str2u32HexNum(argv[5], &gNumDst);
        }
        LOG_I(mesh_app, "num_device %d!!", gNumDst);
    }
    else
    {
        gNumDst = 1;
    }
    bt_mesh_app_util_str2u32HexNum(argv[2], &gAutoCount);
    bt_mesh_app_util_str2u32HexNum(argv[3], &gIsSmall);
    bt_mesh_app_util_str2u32HexNum(argv[4], &gTxTimeout);
    
    if (argc == 7) {
        bt_mesh_app_util_str2u8HexNum(argv[6], &gAutoTtl);
    } else {
        gAutoTtl = bt_mesh_config_get_default_ttl();
    }

    gCrtNumDst = gNumDst;
    gTimes = gAutoCount;
    gState = STATE_BUSY;
    LOG_I(mesh_app, "start latencytest for %d times, ttl = %d !!", gAutoCount, gAutoTtl);
    _latencytest_check_send_ex();

    return BT_STATUS_SUCCESS;
}

static bt_status_t cmd_latencytest_stop(void *no, uint16_t argc, char **argv)
{
    _latencytest_release_timer();
    _rx_release_timer(MESH_RX_TIMER_ID);
    bt_mesh_app_util_stop_timer(MESH_LATENCY_TX_TIMER_ID);
    gState = STATE_IDLE;
    LOG_I(mesh_app, "stop latencytest!\n");
    return BT_STATUS_SUCCESS;
}

const mesh_app_callback_table_t cmd_latencytest_tbl[] = {
    {"start", cmd_latencytest_start},
    {"start_ex", cmd_latencytest_start_ex},
    {"stop", cmd_latencytest_stop},
};

bt_status_t cmd_latency_test(void *no, uint16_t argc, char **argv)
{
    uint16_t i;
    for (i = 0; i < sizeof(cmd_latencytest_tbl) / sizeof(mesh_app_callback_table_t); i++) {
        if (!strcmp(cmd_latencytest_tbl[i].name, argv[1])) {
            return cmd_latencytest_tbl[i].io_callback(no, argc - 1, argv + 1);
        }
    }
    return BT_STATUS_FAIL;

}

#ifdef PROVISIONER_ENABLE

const mesh_app_callback_table_t cmd_prov_tbl[] = {
    {"scan", cmd_prov_scan},
    {"invite", cmd_prov_invite},
    {"start", cmd_prov_start},
    {"run", cmd_prov_run},
    {"stop", cmd_prov_stop},
    {"init", cmd_prov_init},
};

bt_status_t cmd_prov(void *no, uint16_t argc, char **argv)
{
    uint16_t i;
    for (i = 0; i < sizeof(cmd_prov_tbl) / sizeof(mesh_app_callback_table_t); i++) {
        if (!strcmp(cmd_prov_tbl[i].name, argv[1])) {
            return cmd_prov_tbl[i].io_callback(no, argc - 1, argv + 1);
        }
    }
    return BT_STATUS_FAIL;
}

#endif

const mesh_app_callback_table_t cmd_light_tbl[] = {
    {"set", cmd_msg_light_set},
    {"get", cmd_msg_light_get},
};

static bt_status_t cmd_msg_light(void *no, uint16_t argc, char **argv)
{
    uint16_t i;
    for (i = 0; i < sizeof(cmd_light_tbl) / sizeof(mesh_app_callback_table_t); i++) {
        if (!strcmp(cmd_light_tbl[i].name, argv[1])) {
            return cmd_light_tbl[i].io_callback(no, argc - 1, argv + 1);
        }
    }
    return BT_STATUS_FAIL;
}



const mesh_app_callback_table_t cmd_msg_tbl[] = {
    {"config", cmd_msg_config},
    {"onoff", cmd_msg_onoff},
    {"level", cmd_msg_level},
    {"light", cmd_msg_light},
    {"vendor", cmd_msg_vendor},
};

bt_status_t cmd_msg(void *no, uint16_t argc, char **argv)
{
    uint16_t i;
    for (i = 0; i < sizeof(cmd_msg_tbl) / sizeof(mesh_app_callback_table_t); i++) {
        if (!strcmp(cmd_msg_tbl[i].name, argv[1])) {
            return cmd_msg_tbl[i].io_callback(no, argc - 1, argv + 1);
        }
    }
    return BT_STATUS_FAIL;

}

bt_status_t cmd_gatt_scan(void *no, uint16_t argc, char **argv)
{
    connectabt_mesh_device_t *entry = (connectabt_mesh_device_t *)bt_mesh_os_layer_ds_queue_pop(gMeshDeviceQueue);
    while (entry != NULL) {
        LOG_I(mesh_app, "[%d] %02x:%02x:%02x:%02x:%02x:%02x, rssi[%d], type: %s, ", entry->addr.type,
              entry->addr.addr[0], entry->addr.addr[1], entry->addr.addr[2],
              entry->addr.addr[3], entry->addr.addr[4], entry->addr.addr[5],
              entry->rssi, (entry->type == BT_MESH_GATT_SERVICE_PROXY) ? "proxy" : "prov");
        bt_mesh_app_util_dump_pdu("data", entry->data_len, entry->data);
        entry = (connectabt_mesh_device_t *)bt_mesh_os_layer_ds_queue_pop(gMeshDeviceQueue);
    }
    return BT_STATUS_SUCCESS;
}

bt_status_t cmd_gatt_connect(void *no, uint16_t argc, char **argv)
{
    if (argc == 4) {
        // connect <addr_type> <bd_address> <service_type>, <service_type>: 0 proxy, 1 provision
        bt_addr_t target_addr;

        target_addr.type = atoi(argv[1]);
        bt_mesh_app_util_str2u8HexAddrArray(argv[2], target_addr.addr);

        LOG_I(mesh_app, "connecting to %02x:%02x:%02x:%02x:%02x:%02x:...\n",
              target_addr.addr[5], target_addr.addr[4], target_addr.addr[3],
              target_addr.addr[2], target_addr.addr[1], target_addr.addr[0]);
        if (atoi(argv[3])) {
            g_provision_by_gatt = true;
            gProvisioningTick = xTaskGetTickCount() * portTICK_PERIOD_MS;
            bt_mesh_bearer_gatt_connect(&target_addr, BT_MESH_GATT_SERVICE_PROVISION);
        } else {
            bt_mesh_bearer_gatt_connect(&target_addr, BT_MESH_GATT_SERVICE_PROXY);
        }
        return BT_STATUS_SUCCESS;
    }

    return BT_STATUS_FAIL;
}

bt_status_t cmd_gatt_disconnect(void *no, uint16_t argc, char **argv)
{
    bt_mesh_bearer_gatt_disconnect();

    return BT_STATUS_SUCCESS;
}

const mesh_app_callback_table_t cmd_gatt_tbl[] = {
    {"scan", cmd_gatt_scan},
    {"connect", cmd_gatt_connect},
    {"disconnect", cmd_gatt_disconnect},
};
bt_status_t cmd_gatt(void *no, uint16_t argc, char **argv)
{
    uint16_t i;
    for (i = 0; i < sizeof(cmd_gatt_tbl) / sizeof(mesh_app_callback_table_t); i++) {
        if (!strcmp(cmd_gatt_tbl[i].name, argv[1])) {
            return cmd_gatt_tbl[i].io_callback(no, argc - 1, argv + 1);
        }
    }
    return BT_STATUS_FAIL;

}
extern void mesh_app_get_remote_node_info(uint16_t dst_addr, uint16_t lpn_addr);
extern void mesh_app_set_remote_node_info(uint16_t dst_addr);
extern void mesh_ota_dump(void);

bt_status_t cmd_dump_ext(void *no, uint16_t argc, char **argv)
{
    if (argc >= 2) {
        if (!strcmp(argv[1], "remote_node_info")) {
            uint16_t remote_addr;
            uint16_t lpn_addr;
            bt_mesh_app_util_str2u16HexNum(argv[2], &remote_addr);
            bt_mesh_app_util_str2u16HexNum(argv[3], &lpn_addr);
            mesh_app_get_remote_node_info(remote_addr, lpn_addr);
        } else if (!strcmp(argv[1], "set_node_info")) {
            uint16_t remote_addr;
            bt_mesh_app_util_str2u16HexNum(argv[2], &remote_addr);
            mesh_app_set_remote_node_info(remote_addr);
        } else if (!strcmp(argv[1], "ota")) {
            mesh_ota_dump();
            extern uint8_t *_cb_firmware_get_block_data(uint8_t);
            uint8_t * data = _cb_firmware_get_block_data(0);
            bt_mesh_app_util_dump_pdu("ota bin", 20, data);
        }
	}
    return cmd_dump(no, argc, argv);
}

extern bool g_first_fw_addr;
bt_status_t cmd_dis_init(void *no, uint16_t argc, char **argv)
{
    if (argc < 2) {
        LOG_I(mesh_app, "USAGE: dist init <'bin1' | 'bin2'>");
        return BT_STATUS_FAIL;
    }

    if(!strcmp(argv[1], "bin1")){
        g_first_fw_addr = true;
    } else if(!strcmp(argv[1], "bin2")){
        g_first_fw_addr = false;
    } else {
        LOG_I(mesh_app, "dist init <'bin1' | 'bin2'>\n");
        return BT_STATUS_FAIL;
    }
    return BT_STATUS_SUCCESS;
}

bt_status_t cmd_dis_start(void *no, uint16_t argc, char **argv)
{
    #define MAX_DEV 50
    uint32_t fw_id;
    uint16_t dist, i;
    uint16_t nodes[MAX_DEV], group;
    bool manual;

    LOG_I(mesh_app,"ota cmd %d", argc);
    if (argc < 6 || argc > 5+MAX_DEV) {
        LOG_I(mesh_app, "dist start <'auto' | 'manual'><firmware id> <distributor> <group> <node_1> <node_2> ... <node_n> (n < %d)\n", MAX_DEV);
        return BT_STATUS_FAIL;
    }

    if(!strcmp(argv[1], "auto")) manual = false;
    else if(!strcmp(argv[1], "manual")) manual = true;
    else
    {
        LOG_I(mesh_app, "dist start <'auto' | 'manual'><firmware id> <distributor> <group> <node_1> <node_2> ... <node_n> (n < %d)\n", MAX_DEV);
        return BT_STATUS_FAIL;
    }
    bt_mesh_app_util_str2u32HexNum(argv[2], &fw_id);
    bt_mesh_app_util_str2u16HexNum(argv[3], &dist);
    bt_mesh_app_util_str2u16HexNum(argv[4], &group);

    for (i = 0; i < argc - 5; i++) {
        bt_mesh_app_util_str2u16HexNum(argv[5 + i], &nodes[i]);
    }

    bt_mesh_ota_initiator_start_distribution(manual, fw_id, dist, group, nodes, argc - 5);

    return BT_STATUS_SUCCESS;
}

bt_status_t cmd_dis_stop(void *no, uint16_t argc, char **argv)
{
    uint16_t dst_addr;
    uint32_t new_fw_id;

    if (argc < 2) {
        LOG_I(mesh_app, "USAGE: dist stop <distributor_addr> <firmware_id>");
        return BT_STATUS_FAIL;
    }

    if (!bt_mesh_app_util_str2u16HexNum(argv[1], &dst_addr)) {
        return BT_STATUS_FAIL;
    }

    if(argc > 2){            
        if (!bt_mesh_app_util_str2u32HexNum(argv[2], &new_fw_id)) {
            return BT_STATUS_FAIL;
        }
    }

    bt_mesh_ota_initiator_stop_distribution(new_fw_id, dst_addr);
    return BT_STATUS_SUCCESS;
}

bt_status_t cmd_dis_detail(void *no, uint16_t argc, char **argv)
{
    bt_mesh_ota_initiator_dump();

    return BT_STATUS_SUCCESS;
}

bt_status_t cmd_dis_apply(void *no, uint16_t argc, char **argv)
{
    bt_mesh_ota_initiator_apply_distribution();

    return BT_STATUS_SUCCESS;
}

bt_status_t cmd_dis_ttl(void *no, uint16_t argc, char **argv)
{
    uint8_t ttl;

    if (argc < 2) {
        LOG_I(mesh_app, "USAGE: dist ttl <ttl value>");
        return BT_STATUS_FAIL;
    }

    if (!bt_mesh_app_util_str2u8HexNum(argv[1], &ttl)) {
        return BT_STATUS_FAIL;
    }

    bt_mesh_ota_distributor_set_chunk_ttl(ttl);
    return BT_STATUS_SUCCESS;
}

const mesh_app_callback_table_t cmd_dis_tbl[] = {
    {"init", cmd_dis_init},
    {"start", cmd_dis_start},
    {"stop", cmd_dis_stop},
    {"dump", cmd_dis_detail},
    {"apply", cmd_dis_apply},
    {"ttl", cmd_dis_ttl},
};

bt_status_t cmd_dist(void *no, uint16_t argc, char **argv)
{
    uint16_t i;
    for (i = 0; i < sizeof(cmd_dis_tbl) / sizeof(mesh_app_callback_table_t); i++) {
        if (!strcmp(cmd_dis_tbl[i].name, argv[1])) {
            return cmd_dis_tbl[i].io_callback(no, argc - 1, argv + 1);
        }
    }
    return BT_STATUS_FAIL;

}

const mesh_app_callback_table_t cmds[] = {
    {"power", cmd_power},
    {"dump", cmd_dump_ext},
    {"config", cmd_config},
    {"delete", cmd_delete},
#ifdef PROVISIONER_ENABLE
    {"prov", cmd_prov},
#endif
    {"key", cmd_key},
    {"msg", cmd_msg},
    {"latencytest", cmd_latency_test},
    {"packet_loss", cmd_packet_loss_test},
    {"throughput", cmd_throughput_test},
    {"cmd_stop", cmd_stop},
    {"gatt", cmd_gatt},
    {"dist", cmd_dist},
    {"bearer", cmd_bearer},
};

/******************************************************************************/
/* PUBLIC                                                                     */
/******************************************************************************/
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
    } else if (!strncmp(param[0], "assert", 6)) {
        assert(0);
    } else if (!strncmp(param[0], "dump_n9", 7)) {
        LOG_I(mesh_app, "start dump n9");
        bt_driver_trigger_controller_codedump();
        assert(0);
    } else {
        LOG_I(mesh_app, "%s cmd not found\n", __FUNCTION__);
    }

    return BT_STATUS_FAIL;
}

void bt_mesh_ota_initator_event(bt_mesh_ota_initiator_event_t *evt)
{
    char *str_event[] = {"starting", "started", "ongoing", "stopped", "queued", "dfu ready"}; 
    LOG_I(mesh_app, "OTA %d %s [%d](*%d*)(%d sec)\n", 
        (int)evt->serial_number, str_event[evt->event_id], (int)evt->firmware_id, (int)evt->error_code, (int)evt->time_escaped);
}


