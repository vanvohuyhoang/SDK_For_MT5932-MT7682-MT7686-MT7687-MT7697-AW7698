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

#ifdef MTK_BOOTREASON_CHECK_ENABLE
/* Includes ------------------------------------------------------------------*/
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include "bootreason_check.h"
#include "hal_rtc.h"
#include "hal_nvic.h"
#include "hal_nvic_internal.h"
#include "memory_attribute.h"
#include "hal_pmu.h"
#include "exception_handler.h"


/* Private define ------------------------------------------------------------*/
/* Private typedef -----------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
static const char* bootreason_string[] = {  "UNKNOWN",
                                            "NORMAL",
                                            "ASSERT",
                                            "EXCEPTION",
                                            "WATCHDOG"};
static bootreason_reason_t bootreason_lastreason = BOOTREASON_UNKNOWN;
#if (EXCEPTION_MEMDUMP_MODE == EXCEPTION_MEMDUMP_MINIDUMP)
static char bootreason_assert_info[64] = {0};
#endif /* EXCEPTION_MEMDUMP_MODE */


/* Public functions ----------------------------------------------------------*/
void bootreason_set_flag_exception_reset(void)
{
    char flag = BOOTREASON_EXCEPTION_RESET_FLAG;

    hal_rtc_set_data(BOOTREASON_EXCEPTION_RESET_OFFSET,
                     (const char *)&flag,
                     1);
}

void bootreason_set_flag_assert_reset(void)
{
    char flag = BOOTREASON_ASSERT_RESET_FLAG;

    hal_rtc_set_data(BOOTREASON_ASSERT_RESET_OFFSET,
                     (const char *)&flag,
                     1);
}

void bootreason_set_flag_wdt_sw_reset(void)
{
    char flag = BOOTREASON_WDT_SW_RESET_FLAG;

    hal_rtc_set_data(BOOTREASON_WDT_SW_RESET_OFFSET,
                     (const char *)&flag,
                     1);
}

static bootreason_reason_t bootreason_get_boot_reason(void)
{
    uint8_t reason = 0x0;

    /* Check if assert happens */
    hal_rtc_get_data(BOOTREASON_ASSERT_RESET_OFFSET,
                     (char *)&reason,
                     1);
    if(reason == BOOTREASON_ASSERT_RESET_FLAG)
    {
        return BOOTREASON_ASSERT;
    }

    /* Check if other exception happens */
    hal_rtc_get_data(BOOTREASON_EXCEPTION_RESET_OFFSET,
                     (char *)&reason,
                     1);
    if(reason == BOOTREASON_EXCEPTION_RESET_FLAG)
    {
        return BOOTREASON_EXCEPTION;
    }

    /* Check if WDT Software reset happens */
    hal_rtc_get_data(BOOTREASON_WDT_SW_RESET_OFFSET,
                     (char *)&reason,
                     1);
    if(reason == BOOTREASON_WDT_SW_RESET_FLAG)
    {
        return BOOTREASON_NORMAL;
    }

    /* Check the last power off reason */
    reason = pmu_get_power_off_reason();
    if(reason == BOOTREASON_WDT_TIMEOUT_RESET_FLAG)
    {
        return BOOTREASON_WATCHDOG;
    }
    else if((reason == BOOTREASON_POWER_ON_FLAG) ||
            (reason == BOOTREASON_RESETPIN_RESET_FLAG) ||
            (reason == BOOTREASON_FIRST_POWER_ON_FLAG))
    {
        return BOOTREASON_NORMAL;
    }

    return BOOTREASON_UNKNOWN;
}

static void bootreason_flags_clear(void)
{
    char flag = 0x0;

    hal_rtc_set_data(BOOTREASON_ASSERT_RESET_OFFSET,
                     (const char *)&flag,
                     1);
    hal_rtc_set_data(BOOTREASON_EXCEPTION_RESET_OFFSET,
                     (const char *)&flag,
                     1);
    hal_rtc_set_data(BOOTREASON_WDT_SW_RESET_OFFSET,
                     (const char *)&flag,
                     1);
}

void bootreason_init(void)
{
    bootreason_lastreason = bootreason_get_boot_reason();
    bootreason_flags_clear();

    printf("[Boot Reason]:%s", bootreason_string[bootreason_lastreason]);
}

bootreason_status_t bootreason_get_reason(bootreason_reason_t *reason)
{
    *reason = bootreason_lastreason;
    return BOOTREASON_STATUS_OK;
}

bootreason_status_t bootreason_get_info(bootreason_info_t *info)
{
    bootreason_status_t ret = BOOTREASON_STATUS_OK;
#if (EXCEPTION_MEMDUMP_MODE == EXCEPTION_MEMDUMP_MINIDUMP)
    uint32_t dump_count = 0;
    uint32_t dump_addr, dump_len = 0;
    char *assert_file;
    uint32_t assert_line;
    uint32_t offset1, offset2, space;
#endif /* EXCEPTION_MEMDUMP_MODE */

    info->panic_file.data   = NULL;
    info->panic_file.len    = 0;
    info->registers.data    = NULL;
    info->registers.len     = 0;
    info->stack.data        = NULL;
    info->stack.len         = 0;
    info->custom.data       = NULL;
    info->custom.len        = 0;

    switch(bootreason_lastreason)
    {
        case BOOTREASON_UNKNOWN:
            break;

        case BOOTREASON_NORMAL:
            break;

        case BOOTREASON_ASSERT:
#if (EXCEPTION_MEMDUMP_MODE == EXCEPTION_MEMDUMP_MINIDUMP)
            if(EXCEPTION_STATUS_OK != exception_minidump_region_query_count(&dump_count))
            {
                ret = BOOTREASON_STATUS_ERROR;
                break;
            }

            if(EXCEPTION_STATUS_OK != exception_minidump_region_query_info(dump_count,
                                                                           &dump_addr,
                                                                           &dump_len))
            {
                ret = BOOTREASON_STATUS_ERROR;
                break;
            }

            if(EXCEPTION_STATUS_OK != exception_minidump_get_assert_info(dump_addr,
                                                                         &assert_file,
                                                                         &assert_line))
            {
                ret = BOOTREASON_STATUS_ERROR;
                break;
            }
            else
            {
                snprintf(bootreason_assert_info, 12, "%u,", (unsigned int)assert_line);
                offset1 = strlen(bootreason_assert_info);
                space = sizeof(bootreason_assert_info) - 1 - offset1;
                if(space < strlen(assert_file))
                {
                    /* There is no enough space for file string*/
                    offset2 = strlen(assert_file) - space;
                    strncpy(bootreason_assert_info + offset1, assert_file + offset2, space);
                    bootreason_assert_info[sizeof(bootreason_assert_info) - 1] = 0;
                }
                else
                {
                    /* There is enough space for file string*/
                    strncpy(bootreason_assert_info + offset1, assert_file, strlen(assert_file));
                    bootreason_assert_info[sizeof(bootreason_assert_info) - 1] = 0;
                }
                info->panic_file.data = (uint8_t *)bootreason_assert_info;
                info->panic_file.len  = strlen(bootreason_assert_info);
            }

            if(EXCEPTION_STATUS_OK != exception_minidump_get_context_info(dump_addr,
                                                                          &info->registers.data,
                                                                          &info->registers.len))
            {
                ret = BOOTREASON_STATUS_ERROR;
                break;
            }

            if(EXCEPTION_STATUS_OK != exception_minidump_get_stack_info(dump_addr,
                                                                        &info->stack.data,
                                                                        &info->stack.len))
            {
                ret = BOOTREASON_STATUS_ERROR;
                break;
            }

            info->custom.data = (uint8_t *)dump_addr;
            info->custom.len  = dump_len;
#endif /* EXCEPTION_MEMDUMP_MODE */
            break;

        case BOOTREASON_EXCEPTION:
#if (EXCEPTION_MEMDUMP_MODE == EXCEPTION_MEMDUMP_MINIDUMP)
            if(EXCEPTION_STATUS_OK != exception_minidump_region_query_count(&dump_count))
            {
                ret = BOOTREASON_STATUS_ERROR;
                break;
            }

            if(EXCEPTION_STATUS_OK != exception_minidump_region_query_info(dump_count,
                                                                           &dump_addr,
                                                                           &dump_len))
            {
                ret = BOOTREASON_STATUS_ERROR;
                break;
            }

            if(EXCEPTION_STATUS_OK != exception_minidump_get_context_info(dump_addr,
                                                                          &info->registers.data,
                                                                          &info->registers.len))
            {
                ret = BOOTREASON_STATUS_ERROR;
                break;
            }

            if(EXCEPTION_STATUS_OK != exception_minidump_get_stack_info(dump_addr,
                                                                        &info->stack.data,
                                                                        &info->stack.len))
            {
                ret = BOOTREASON_STATUS_ERROR;
                break;
            }

            info->custom.data = (uint8_t *)dump_addr;
            info->custom.len  = dump_len;
#endif /* EXCEPTION_MEMDUMP_MODE */
            break;

        case BOOTREASON_WATCHDOG:
            break;

        default:
            ret = BOOTREASON_STATUS_ERROR;
            break;
    }

    info->reason = bootreason_lastreason;

    return ret;
}

#endif /* MTK_BOOTREASON_CHECK_ENABLE */
