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

#ifndef __BWCS_H__
#define __BWCS_H__

#include <stdint.h>
#include "bwcs_api.h"

#define BWCS_TASK_NAME "bwcs"
#define BWCS_STACK_SIZE 1024*4
#define BWCS_TASK_PRIORITY 4

typedef enum
{
    BWCS_EVENT_TYPE_WIFI,
    BWCS_EVENT_TYPE_BT,
} bwcs_event_type_t;

typedef enum
{
    BWCS_WIFI_EVENT_OFF,
    BWCS_WIFI_EVENT_ON,
    BWCS_WIFI_EVENT_CONNECTING,
    BWCS_WIFI_EVENT_CONNECTED,
    BWCS_WIFI_EVENT_CH_UPDATE,
} bwcs_wifi_event_t;

typedef enum
{
    BWCS_BT_EVENT_OFF,
    BWCS_BT_EVENT_ON,
    BWCS_BT_EVENT_CH_UPDATE,    
} bwcs_bt_event_t;

typedef struct
{
    bwcs_event_type_t event_type;
    uint8_t event;
    uint8_t payload[3];
} bwcs_queue_t;

typedef struct
{
    bwcs_wifi_event_t wifi_current_status;
    uint8_t wifi_channel;    
} bwcs_wifi_status_t;

typedef struct
{
    bwcs_bt_event_t bt_current_status;
    uint8_t bt_channel[3];    
} bwcs_bt_status_t;

typedef struct
{
    pta_mode_t pta_mode;
    pta_cm_mode_t pta_cm_mode;
    pta_antenna_mode_t antenna_mode;
    bwcs_wifi_status_t wifi_status;
    bwcs_bt_status_t bt_status;
} bwcs_status_t;

typedef struct bwcs_event_handler_struct {
    bwcs_event_t events;
    bwcs_event_handler_t func;
    struct bwcs_event_handler_struct *next;
} bwcs_event_handler_struct;

void bwcs_set_notification_callback(uint8_t enabled, bwcs_event_t events, bwcs_event_handler_t callback);

void bwcs_handler(void);

#endif
