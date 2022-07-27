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
 
/* Includes ------------------------------------------------------------------*/
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
/* Kernel includes. */
#include "FreeRTOS.h"
#include "task.h"

/* device.h includes */
#include "aw7698.h"

/* hal includes */
#include "hal.h"

#include "sys_init.h"
#include "task_def.h"
#ifdef MTK_WIFI_ENABLE
#include "wifi_nvdm_config.h"
#include "wifi_lwip_helper.h"
#endif
#if defined(MTK_MINICLI_ENABLE)
#include "cli_def.h"
#endif

#include "bsp_gpio_ept_config.h"
#include "hal_sleep_manager.h"

#ifdef MTK_WIFI_ENABLE
#include "connsys_profile.h"
#include "wifi_api.h"

#ifdef MTK_BWCS_ENABLE
#include "bwcs_api.h"
#endif
#endif

#ifdef MTK_SYSTEM_HANG_TRACER_ENABLE
#include "systemhang_tracer.h"
#endif /* MTK_SYSTEM_HANG_TRACER_ENABLE */

extern void httpd_test(void);
#define WIFI_SSID                ("SQA_TEST_AP")
#define WIFI_PASSWORD            ("77777777")

#ifdef MTK_WIFI_ENABLE
int32_t wifi_station_port_secure_event_handler(wifi_event_t event, uint8_t *payload, uint32_t length);
int32_t wifi_scan_complete_handler(wifi_event_t event, uint8_t *payload, uint32_t length);
#endif


void vApplicationIdleHook(void)
{
#ifdef MTK_SYSTEM_HANG_TRACER_ENABLE
    systemhang_wdt_count = 0;
    hal_wdt_feed(HAL_WDT_FEED_MAGIC);
#endif /* MTK_SYSTEM_HANG_TRACER_ENABLE */
}

#ifdef MTK_WIFI_ENABLE
int32_t wifi_init_done_handler(wifi_event_t event,
                                      uint8_t *payload,
                                      uint32_t length)
{
    LOG_I(common, "WiFi Init Done: port = %d", payload[6]);
    return 1;
}

#ifdef MTK_USER_FAST_TX_ENABLE
#include "type_def.h"

#define DemoPktLen 64
extern UINT_8 DemoPkt[];

extern uint32_t g_FastTx_Channel;
extern PUINT_8 g_PktForSend;
extern UINT_32 g_PktLen;
static void fastTx_init(uint32_t channel, PUINT_8 pPktContent, UINT_32 PktLen)
{
    g_FastTx_Channel = channel;
    g_PktForSend = pPktContent;
    g_PktLen = PktLen;
}
#endif
#endif

/**
  * @brief      Create a task for httpd example
  * @param[in]  void *args: Not used
  * @return     None
  */
static void user_entry(void *args)
{
    lwip_net_ready();

    // Httpd feature test
    httpd_test();

    while (1) {
        vTaskDelay(1000 / portTICK_RATE_MS); // release CPU
    }
}

/**
  * @brief  Main program
  * @param  None
  * @retval None
  */
int main(void)
{
#ifdef MTK_WIFI_ENABLE
#ifdef MTK_USER_FAST_TX_ENABLE
    /* Customize Packet Content and Length */
    fastTx_init(11, DemoPkt, DemoPktLen);
#endif
#endif

    /* Do system initialization, eg: hardware, nvdm, logging and random seed. */
    system_init();

#ifdef MTK_WIFI_ENABLE
    /* User initial the parameters for wifi initial process,  system will determin which wifi operation mode
     * will be started , and adopt which settings for the specific mode while wifi initial process is running*/
    wifi_cfg_t wifi_config = {0};
    if (0 != wifi_config_init(&wifi_config)) {
        LOG_E(common, "wifi config init fail");
        return -1;
    }

    wifi_config_t config = {0};
    wifi_config_ext_t config_ext = {0};

    config.opmode = wifi_config.opmode;

    memcpy(config.sta_config.ssid, wifi_config.sta_ssid, 32);
    config.sta_config.ssid_length = wifi_config.sta_ssid_len;
    config.sta_config.bssid_present = 0;
    memcpy(config.sta_config.password, wifi_config.sta_wpa_psk, 64);
    config.sta_config.password_length = wifi_config.sta_wpa_psk_len;
    config_ext.sta_wep_key_index_present = 1;
    config_ext.sta_wep_key_index = wifi_config.sta_default_key_id;
    config_ext.sta_auto_connect_present = 1;
    config_ext.sta_auto_connect = 1;

    memcpy(config.ap_config.ssid, wifi_config.ap_ssid, 32);
    config.ap_config.ssid_length = wifi_config.ap_ssid_len;
    memcpy(config.ap_config.password, wifi_config.ap_wpa_psk, 64);
    config.ap_config.password_length = wifi_config.ap_wpa_psk_len;
    config.ap_config.auth_mode = (wifi_auth_mode_t)wifi_config.ap_auth_mode;
    config.ap_config.encrypt_type = (wifi_encrypt_type_t)wifi_config.ap_encryp_type;
    config.ap_config.channel = wifi_config.ap_channel;
    config.ap_config.bandwidth = wifi_config.ap_bw;
    config.ap_config.bandwidth_ext = WIFI_BANDWIDTH_EXT_40MHZ_UP;
    config_ext.ap_wep_key_index_present = 1;
    config_ext.ap_wep_key_index = wifi_config.ap_default_key_id;
    config_ext.ap_hidden_ssid_enable_present = 1;
    config_ext.ap_hidden_ssid_enable = wifi_config.ap_hide_ssid;
    config_ext.sta_power_save_mode = (wifi_power_saving_mode_t)wifi_config.sta_power_save_mode;

    // For Network Example
    strcpy((char *)config.sta_config.ssid, WIFI_SSID);
    strcpy((char *)config.sta_config.password, WIFI_PASSWORD);
    config.sta_config.ssid_length = strlen(WIFI_SSID);
    config.sta_config.password_length = strlen(WIFI_PASSWORD);

    /* Initialize wifi stack and register wifi init complete event handler,
     * notes:  the wifi initial process will be implemented and finished while system task scheduler is running,
     *            when it is done , the WIFI_EVENT_IOT_INIT_COMPLETE event will be triggered */
    wifi_init(&config, &config_ext);

    wifi_connection_register_event_handler(WIFI_EVENT_IOT_INIT_COMPLETE, wifi_init_done_handler);

    /* Tcpip stack and net interface initialization,  dhcp client, dhcp server process initialization*/
    lwip_network_init(config.opmode);
    lwip_net_start(config.opmode);
#endif

#if defined(MTK_MINICLI_ENABLE)
    /* Initialize cli task to enable user input cli command from uart port.*/
    cli_def_create();
    cli_task_create();
#endif

#ifdef MTK_WIFI_ENABLE
#ifdef MTK_BWCS_ENABLE
    bwcs_init_t bwcs_init_param = {0};
    bwcs_init_param.pta_mode = PTA_MODE_TDD_3_WIRE;
    bwcs_init_param.pta_cm_mode = PTA_CM_MODE_TDD;
    bwcs_init_param.antenna_mode = PTA_ANT_MODE_SINGLE;
    bwcs_init(bwcs_init_param);
#endif
#endif

    /* Call this function to indicate the system initialize done. */
    SysInitStatus_Set();

#ifdef MTK_SYSTEM_HANG_CHECK_ENABLE
#ifdef HAL_WDT_MODULE_ENABLED
    wdt_init();
#endif
#endif

    /* Create a user task for demo when and how to use wifi config API  to change WiFI settings,
        * Most WiFi APIs must be called in task scheduler, the system will work wrong if called in main(),
        * For which API must be called in task, please refer to wifi_api.h or WiFi API reference.
        * xTaskCreate(user_wifi_app_entry,
        *       UNIFY_USR_DEMO_TASK_NAME,
        *       UNIFY_USR_DEMO_TASK_STACKSIZE / 4,
        *       NULL, UNIFY_USR_DEMO_TASK_PRIO, NULL);
        */
    if (pdPASS != xTaskCreate(user_entry,
                              USER_ENTRY_TASK_NAME,
                              USER_ENTRY_TASK_STACKSIZE / sizeof(portSTACK_TYPE),
                              NULL,
                              USER_ENTRY_TASK_PRIO,
                              NULL)) {
        return -1;
    }

    /* Start the scheduler. */
    vTaskStartScheduler();

    /* If all is well, the scheduler will now be running, and the following line
    will never be reached.  If the following line does execute, then there was
    insufficient FreeRTOS heap memory available for the idle and/or timer tasks
    to be created.  See the memory management section on the FreeRTOS web site
    for more details. */
    for (;;);
}

