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
 
#include "FreeRTOS.h"
#include "timers.h"
#include "fota_timer.h"
#include "fota_util.h"
#include "fota_platform.h"


static TimerHandle_t g_fota_reboot_timer;


static void fota_reboot_timer_hdlr(TimerHandle_t timer_id)
{
    fota_device_reboot();
}


/* Use timer to delay certain time before reboot to let the last BT package be sent to the peer. */
bool fota_start_reboot_timer(uint32_t delay_msec)
{
    if (!g_fota_reboot_timer)
    {
        /* Start a new timer */
        g_fota_reboot_timer = xTimerCreate( "g_fota_reboot_timer",
                                            (delay_msec/portTICK_PERIOD_MS),
                                            pdFALSE,
                                            NULL,
                                            fota_reboot_timer_hdlr);
        if (g_fota_reboot_timer)
        {
        xTimerStart(g_fota_reboot_timer, 0);
        return TRUE;
    }
    }
    
    /* Change the timer period. */    
    if (!g_fota_reboot_timer)
    {
        return FALSE;
    }

    if (xTimerIsTimerActive(g_fota_reboot_timer) != pdFALSE)
    {
        if (pdFAIL == xTimerStop(g_fota_reboot_timer, 0))
        {
            return FALSE;
        }
    }

    if (pdFAIL == xTimerChangePeriod(g_fota_reboot_timer, (delay_msec/portTICK_PERIOD_MS), 0))
    {
        return FALSE;
    }

    return TRUE;
}

