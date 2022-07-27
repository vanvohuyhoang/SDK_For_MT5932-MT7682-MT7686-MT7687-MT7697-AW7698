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

#ifndef __SYSTEM_DAEMON_H__
#define __SYSTEM_DAEMON_H__

#ifdef SYSTEM_DAEMON_TASK_ENABLE

#include <stdint.h>
#include "FreeRTOS.h"

#ifdef __cplusplus
extern "C" {
#endif

/* system daemon event id */
typedef enum {
    SYSTEM_DAEMON_ID_NVDM = 0,
    SYSTEM_DAEMON_ID_LOGGING_TO_FLASH = 1,

    SYSTEM_DAEMON_ID_EVENT_MAX = 0xffffffff
} system_daemon_event_t;

/* system daemon task queue length */
#define SYSTEM_DAEMON_QUEUE_LENGTH 10

/* system daemon task stack size */
#define SYSTEM_DAEMON_STACK_SIZE 2048 /* Byte */

/* system daemon task main function prototype */
extern void system_daemon_task(void *arg);

/**
 * @brief  This function sends a message to the system daemon task's queue.
 * @param[in] event_id is the id, which is statically defined in the enum system_daemon_event_id.
 * @param[in] pItem is a pointer to the item that is to be put on the queue.
 * The size of the items the queue will hold was defined when the queue was created,
 * so this number of bytes is copied from pItem into the queue storage area.
 * @return
 * return pdTRUE, the message is successfully sent to the queue.\n
 * return pdFALSE, the message failed to be sent to the queue.
 */
BaseType_t system_daemon_send_message(system_daemon_event_t event_id, const void *const pitem);




#ifdef __cplusplus
}
#endif

#endif     /* SYSTEM_DAEMON_TASK_ENABLE */
#endif    /*__SYSTEM_DAEMON_H__*/
