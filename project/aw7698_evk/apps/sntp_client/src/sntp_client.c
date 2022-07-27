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
#include "os.h"

#include "sntp.h"
#include "syslog.h"
#include "hal_rtc.h"


/**
* @brief         SNTP task process function.
* @param[in]     Not using
* @return        None
*/
static void verify_proc(void *args)
{
    hal_rtc_time_t r_time;
    hal_rtc_status_t ret = HAL_RTC_STATUS_OK;

    for (int i = 0 ; i < 90; i++) {
        LOG_I(sntp_client_main, "Waiting for SNTP success [%d]", i);
        ret = hal_rtc_get_time(&r_time);
        if (ret == HAL_RTC_STATUS_OK && (r_time.rtc_year != 0 || r_time.rtc_mon != 1 || r_time.rtc_day != 1)) {
            LOG_I(sntp_client_main, "SNTP success [%d]", i);
            LOG_I(sntp_client_main, "cur_time[%d,%d,%d,%d]", r_time.rtc_year, r_time.rtc_mon, r_time.rtc_day, r_time.rtc_week);
            LOG_I(sntp_client_main, "[%d]cur_time[%d:%d:%d]", ret, r_time.rtc_hour, r_time.rtc_min, r_time.rtc_sec);
            sntp_stop();
            break;
        }
        vTaskDelay(1000 / portTICK_RATE_MS);
    }

    LOG_I(sntp_client_main, "test_proc TaskDelete");
    LOG_I(sntp_client_main, "example project test success.");
    vTaskDelete(NULL);
}


/**
* @brief        SNTP client example entry function
* @return       None
*/
void sntp_client()
{
    /** Set this to 1 to allow config of SNTP server(s) by DNS name */
#if (!SNTP_SERVER_DNS)
    struct ip4_addr test_addr;
#endif
    hal_rtc_time_t r_time = {6,6,6,1,1,6,0};
    hal_rtc_status_t ret = HAL_RTC_STATUS_OK;

    //Set RTC to a incorrect time.
    ret = hal_rtc_set_time(&r_time);
    LOG_I(sntp_client_main, "[%d]cur_time[%d:%d:%d]", ret, r_time.rtc_hour, r_time.rtc_min, r_time.rtc_sec);

    //Create a task to check SNTP status.
    portBASE_TYPE type = xTaskCreate(verify_proc, APP_SNTP_TASK_NAME, APP_SNTP_TASK_STACKSIZE / sizeof(portSTACK_TYPE), NULL, APP_SNTP_TASK_PRIO, NULL);
    LOG_I(sntp_client_main, "xTaskCreate test_proc -- %d", type);

    //SNTP example start.
    LOG_I(sntp_client_main, "Begin to init SNTP");
    
    /** Set this to 1 to allow config of SNTP server(s) by DNS name */
#if SNTP_SERVER_DNS
    sntp_setservername(0, "1.cn.pool.ntp.org");
    sntp_setservername(1, "1.hk.pool.ntp.org");
#else
    IP4_ADDR(&test_addr, 213, 161, 194, 93);
    sntp_setserver(0, (const ip_addr_t *)(&test_addr));
    IP4_ADDR(&test_addr, 129, 6, 15, 29);
    sntp_setserver(1, (const ip_addr_t *)(&test_addr));
#endif
    sntp_init();

    LOG_I(sntp_client_main, "SNTP init done");
}

