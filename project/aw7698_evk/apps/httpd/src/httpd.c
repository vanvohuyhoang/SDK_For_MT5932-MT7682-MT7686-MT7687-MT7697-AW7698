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

#ifdef MTK_WIFI_ENABLE
#include "connsys_profile.h"
#include "wifi_api.h"
#endif

#include "httpd.h"
#include "httpclient.h"
static xQueueHandle mbox;
#define BUF_SIZE        (1024 * 4)
#define HTTP_GET_URL    "http://127.0.0.1/"

#ifdef MTK_WIFI_ENABLE
int32_t wifi_station_port_secure_event_handler(wifi_event_t event, uint8_t *payload, uint32_t length);
int32_t wifi_scan_complete_handler(wifi_event_t event, uint8_t *payload, uint32_t length);
#endif

log_create_module(httpd_example, PRINT_LEVEL_INFO);

/**
  * @brief      http server feedback handle
  * @param      None
  * @return     0, if http server run.\n
  *             1, if other status occurred.\n
  */
static int httpd_test_fb_handle(void)
{
    httpd_fb fb;

    LOG_I(httpd_example, "httpd_test_fb_handle()");

    for (;;) {
        if (xQueueReceive(mbox, (void *)&fb, portMAX_DELAY) == pdPASS) {
            LOG_I(httpd_example, "httpd status = %d", fb.status);

            switch (fb.status) {
                case HTTPD_STATUS_RUN:
                    return 0;  // HTTP server is running.
                case HTTPD_STATUS_STOPPING:
                case HTTPD_STATUS_STOP:
                    break; // HTTP server is stopping or has stopped.
                case HTTPD_STATUS_UNINIT:
                    return 1;  // Uninitialized.
            }
        }
    }
}

/**
  * @brief      Polling http server status
  * @param      None
  * @return     0, if http server stopped.\n
  *             1, if other status occurred.\n
  */
static int httpd_test_poll_status(void)
{
    HTTPD_STATUS status = HTTPD_STATUS_UNINIT;

    LOG_I(httpd_example, "httpd_test_poll_status()");

    for (;;) {
        vTaskDelay(1000); // Delay by 1000ms.

        status = httpd_get_status();

        switch (status) {
            case HTTPD_STATUS_STOP:
                return 0;  // HTTP server has stopped.
            case HTTPD_STATUS_STOPPING:
            case HTTPD_STATUS_RUN:
                break; // HTTP server is running.
            case HTTPD_STATUS_UNINIT:
                return 1;  // Uninitialized.
        }
    }
}

/**
  * @brief      Access webpage by HTTP request
  * @param      None
  * @return     0, if success.\n
  *             other, if fail.\n
  */
static int httpd_test_access_webpage(void)
{
    char *url = HTTP_GET_URL;
    httpclient_t client = {0};
    httpclient_data_t client_data = {0};
    char *buf;
    HTTPCLIENT_RESULT result;

    LOG_I(httpd_example, "httpd_test_access_webpage()");

    buf = pvPortMalloc(BUF_SIZE);
    if (buf == NULL) {        
        LOG_I(httpd_example, "memory malloc fail.");
        return 1;
    }
    
    client_data.response_buf = buf;
    client_data.response_buf_len = BUF_SIZE;
    client.auth_user = "admin";
    client.auth_password = "admin";

    // httpclient "get"
    result = httpclient_get(&client, url, &client_data);
    LOG_I(httpd_example, "data received: %s", client_data.response_buf);
    
    vPortFree(buf);

    if(result >= 0)
        return 0;
    else {        
        LOG_I(httpd_example, "example project test fail, reason:%d.", result);
        return 1;
    }
}

/**
  * @brief      Http server init, start and stop flow.
  * @param      None
  * @return     None
  */
void httpd_test(void)
{
    /*
     *  The webpages and settings can be customized.
     *
     *  The webpages on the server is stored in middleware/third_party/httpd/webhtml folder.
     *  The web server settings are defined in middleware/third_party/httpd/inc/config.h.
     */
    HTTPD_RESULT result;
    httpd_para parameter;

    LOG_I(httpd_example, "httpd_test()");

    // Firstly, initialize the HTTP server.
    result = httpd_init();

    switch (result) {
        case HTTPD_RESULT_SUCCESS:   /**<  Initialization was successful. */
            break;
        case HTTPD_RESULT_WAITING:   /**<  Needs to wait for the feedback. */
        case HTTPD_RESULT_UNINIT:   /**<  HTTP server is uninitialized. */
        case HTTPD_RESULT_QUEUE_FULL: /**<  The queue is full. */
            LOG_I(httpd_example, "init failed.");
            return ;
    }

    /*
     * Secondly, start the HTTP server.
     * After starting the HTTP server, the status can asynchronously queried, 
     * either through the message to listen to the status or through the polling.
     */
    mbox = xQueueCreate(4, sizeof(httpd_fb));
    if (mbox == NULL) {
        LOG_I(httpd_example, "xQueueCreate failed.");
        return;
    }

    memset(&parameter, 0, sizeof(httpd_para));
    parameter.fb_queue = mbox;
    result = httpd_start(&parameter);

    switch (result) {
        case HTTPD_RESULT_SUCCESS: // Successful.
            break;
        case HTTPD_RESULT_UNINIT: // Uninitialized.
        case HTTPD_RESULT_QUEUE_FULL: // Mbox queue is full.
            LOG_I(httpd_example, "httpd start failed.");
            return;
        case HTTPD_RESULT_WAITING: {// Need to wait for the feedback of the status notification or to poll the status.
            if (httpd_test_fb_handle()) {
                LOG_I(httpd_example, "httpd start failed.");
                return;
            }
            break;
        }
    }

    // Right now, HTTP server has started. The user can browse the webpage.
    if(httpd_test_access_webpage())
        return;
    
    // Finally, the following flow can be used to stop HTTP server.
    result = httpd_stop();

    switch (result) {
        case HTTPD_RESULT_SUCCESS: // Successful.
            break;
        case HTTPD_RESULT_UNINIT: // Uninitialized.
        case HTTPD_RESULT_QUEUE_FULL: // Mbox queue is full.
            LOG_I(httpd_example, "httpd stop failed.");
            return;
        case HTTPD_RESULT_WAITING: {// Need to wait for the feedback of the status notification or to poll the status.
            if (httpd_test_poll_status()) {
                LOG_I(httpd_example, "httpd stop failed.");
                return;
            }
            break;
        }

    }

    // Print success log
    LOG_I(httpd_example, "example project test success.");
    return;
}


