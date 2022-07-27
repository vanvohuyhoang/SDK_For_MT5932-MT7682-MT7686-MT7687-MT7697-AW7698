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

#include "bwcs.h"
#include "bwcs_api.h"
#include "wifi_os_api.h"
#include "hal_platform.h"
#include "hal_gpio.h"
#include "os.h"
#include "bwcs_bt_interface.h"

bwcs_status_t g_bwcs_status = {0};
QueueHandle_t g_bwcs_queue = NULL;
os_task_t bwcs_task = NULL;


int32_t bwcs_init(bwcs_init_t bwcs_init_param)
{
    g_bwcs_status.pta_mode = bwcs_init_param.pta_mode;
    g_bwcs_status.pta_cm_mode = bwcs_init_param.pta_cm_mode;
    g_bwcs_status.antenna_mode = bwcs_init_param.antenna_mode;
    g_bwcs_status.wifi_status.wifi_current_status = BWCS_WIFI_EVENT_OFF;
    g_bwcs_status.bt_status.bt_current_status = BWCS_BT_EVENT_OFF;

    bwcs_bt_init();

    g_bwcs_queue = wifi_os_queue_create(5, sizeof(bwcs_queue_t));
    if(g_bwcs_queue == NULL) {
        printf("BWCS queue create fail.\r\n");
        return -1;
    }
    if ( wifi_os_task_create((TaskFunction_t)bwcs_handler,
                            BWCS_TASK_NAME,
                            BWCS_STACK_SIZE,
                            NULL,
                            BWCS_TASK_PRIORITY,
                            &bwcs_task) != 0 )
    {
        printf( "BWCS Task Create Fail!\r\n");
        return -1;
    }
    return 0;
}

int32_t bwcs_deinit(void)
{
    os_memset(&g_bwcs_status,0,sizeof(g_bwcs_status));
    if(g_bwcs_queue != NULL) {
        wifi_os_queue_delete(g_bwcs_queue);
        g_bwcs_queue = NULL;
    }

    if(bwcs_task != NULL) {
        wifi_os_task_delete(bwcs_task);
        bwcs_task = NULL;
    }

    return 0;
    
}

int32_t bwcs_register_event_handler(bwcs_event_t event,bwcs_event_handler_t handler)
{
    bwcs_set_notification_callback(1, event, handler);
    return 0;
}

int32_t bwcs_unregister_event_handler(bwcs_event_t event,bwcs_event_handler_t handler)
{
    bwcs_set_notification_callback(0, event, handler);
    return 0;
}

