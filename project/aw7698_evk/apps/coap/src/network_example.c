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
#include <stdio.h>
#include <string.h>

/* Kernel includes. */
#include "FreeRTOS.h"
#include "task.h"
#include "sys_init.h"
#include "wifi_api.h"
#include "lwip/netif.h"
#include "lwip/tcpip.h"
#include "lwip/sockets.h"
#include "ethernetif.h"
#include "lwip/sockets.h"
#include "netif/etharp.h"
#include "portmacro.h"
#include "wifi_lwip_helper.h"
#include "task_def.h"
#include "syslog.h"

extern int coap_client();
extern int coap_server();

/**
  * Create the log control block for lwip socket example.
  * User needs to define their own log control blocks as project needs.
  * Please refer to the log dev guide under /doc folder for more details.
  */
log_create_module(coap_example, PRINT_LEVEL_INFO);

/**
  * @brief    server test entry
  * @param[in] void *not_used:Not used
  * @return    None
  */
static void server_test_thread(void *not_used)
{
    coap_server();
    //Keep the task alive
    while (1) {
        vTaskDelay(1000 / portTICK_RATE_MS); // release CPU
    }
}

/**
  * @brief     client test entry
  * @param[in] void *not_used:Not used
  * @return    None
  */
static void client_test_thread(void *not_used)
{
    vTaskDelay(1000 / portTICK_RATE_MS);
    coap_client();
    //Keep the task alive
    while (1) {
        vTaskDelay(1000 / portTICK_RATE_MS); // release CPU
    }
}

/**
  * @brief     Create a task for tcp/udp test
  * @param[in] void *args:Not used
  * @return    None
  */
void user_entry(void *args)
{
    lwip_net_ready();
    LOG_I(coap_example, "Begin to create test_thread");
    //xTaskHandle xHandle;

    if (pdPASS != xTaskCreate(server_test_thread,
                              COAP_SERVER_EXAMPLE_TASK_NAME,
                              COAP_EXAMPLE_TASK_STACKSIZE / sizeof(portSTACK_TYPE),
                              NULL,
                              COAP_EXAMPLE_TASK_PRIO,
                              NULL)) {
        LOG_I(coap_example, "Cannot create server_test_thread");
    }

    if (pdPASS != xTaskCreate(client_test_thread,
                              COAP_CLINET_EXAMPLE_TASK_NAME,
                              COAP_EXAMPLE_TASK_STACKSIZE / sizeof(portSTACK_TYPE),
                              NULL,
                              COAP_EXAMPLE_TASK_PRIO,
                              NULL)) {
        LOG_I(coap_example, "Cannot create client_test_thread");
    }

    LOG_I(coap_example, "Finish to create test_thread");
    while (1) {
        vTaskDelay(1000 / portTICK_RATE_MS); // release CPU
    }
}