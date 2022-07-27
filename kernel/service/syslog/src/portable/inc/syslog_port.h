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

#ifndef __SYSLOG_PORT_H__
#define __SYSLOG_PORT_H__

#ifdef __cplusplus
extern "C" {
#endif

/* debug options for syslog */


#include "syslog_port_common.h"

/* MSGID log string start address define */
#define PORT_SYSLOG_MSG_ADDR_OFFSET 0x06000000

/* syslog length limitation */
#define PORT_SYSLOG_MAX_ONE_LOG_SIZE (256 + 64)
#define PORT_SYSLOG_MAX_ONE_PLAIN_LOG_SIZE (256)

/* syslog global share buffer and variable define */
#define PORT_SYSLOG_SHARE_MEMORY_SIZE 8192  /* MUST be 4-byte align */
#define PORT_SYSLOG_SHARE_VARIABLE_SIZE 512
#ifdef PORT_SYSLOG_SINGLE_CPU_ENV
ATTR_ZIDATA_IN_NONCACHED_SYSRAM_4BYTE_ALIGN static uint8_t g_syslog_share_buffer[PORT_SYSLOG_SHARE_MEMORY_SIZE + PORT_SYSLOG_SHARE_VARIABLE_SIZE];
#define PORT_SYSLOG_SHARE_MEMORY_BEGIN (uint32_t)g_syslog_share_buffer
#else
#include "hal_resource_assignment.h"
#define PORT_SYSLOG_SHARE_MEMORY_BEGIN HW_SYSRAM_PRIVATE_MEMORY_SYSLOG_START
#endif
#define PORT_SYSLOG_SHARE_MEMORY_END   (PORT_SYSLOG_SHARE_MEMORY_BEGIN + PORT_SYSLOG_SHARE_MEMORY_SIZE)
#define share_log_buffer_offset(address) ((address) - PORT_SYSLOG_SHARE_MEMORY_BEGIN)
#define share_log_buffer_address(offset) (PORT_SYSLOG_SHARE_MEMORY_BEGIN + (offset))
#define PORT_SYSLOG_SHARE_VARIABLE_BEGIN (PORT_SYSLOG_SHARE_MEMORY_BEGIN + PORT_SYSLOG_SHARE_MEMORY_SIZE)
#define PORT_SYSLOG_MODULE_FILTER_TOTAL_NUMBER 200
#define PORT_SYSLOG_MODULE_FILTER_STATUS_SIZE (PORT_SYSLOG_CPU_NUMBER * 2 + PORT_SYSLOG_MODULE_FILTER_TOTAL_NUMBER)
#define PORT_SYSLOG_MAX_MODULE_FILTER_STATUS_SIZE (PORT_SYSLOG_MAX_CPU_NUMBER * 2 + PORT_SYSLOG_MODULE_FILTER_TOTAL_NUMBER)

/* log filters collection */
#if defined(__GNUC__)
extern uint8_t _log_filter_start[];
extern uint8_t _log_filter_end[];

#define LOG_FILTER_START _log_filter_start
#define LOG_FILTER_END _log_filter_end
#endif

/* SDK version and build time */
#ifdef MTK_CPU_NUMBER_0
extern char sw_verno_str[];
#else
#include "memory_attribute.h"
#define DUMMY_SDK_VERSION_STRING "DUMMY_SDK_VERSION"
static char sw_verno_str[] = DUMMY_SDK_VERSION_STRING;
ATTR_LOG_VERSION log_bin_sw_verno_str[] = DUMMY_SDK_VERSION_STRING;
#endif
extern char build_date_time_str[];

#ifdef PORT_SYSLOG_SINGLE_CPU_ENV
#define PORT_SYSLOG_SDK_VERSION_BEGIN sw_verno_str
#define PORT_SYSLOG_SDK_VERSION_LENGTH strlen(sw_verno_str)
#define PORT_SYSLOG_BUILD_TIME_BEGIN build_date_time_str
#define PORT_SYSLOG_BUILD_TIME_LENGTH strlen(build_date_time_str)
static void port_syslog_build_time_sdk_version_copy(uint32_t cpu_id)
{}
#else
#define PORT_SYSLOG_SDK_VERSION_BEGIN HW_SYSRAM_PRIVATE_MEMORY_SYSLOG_VERSION_VAR_START
#define PORT_SYSLOG_SDK_VERSION_LENGTH 32
#define PORT_SYSLOG_BUILD_TIME_BEGIN HW_SYSRAM_PRIVATE_MEMORY_SYSLOG_BUILD_TIME_VAR_START
#define PORT_SYSLOG_BUILD_TIME_LENGTH 32
static void port_syslog_build_time_sdk_version_copy(uint32_t cpu_id)
{
    strcpy((char *)(PORT_SYSLOG_SDK_VERSION_BEGIN + cpu_id * PORT_SYSLOG_SDK_VERSION_LENGTH), sw_verno_str);
    strcpy((char *)(PORT_SYSLOG_BUILD_TIME_BEGIN + cpu_id * PORT_SYSLOG_BUILD_TIME_LENGTH), build_date_time_str);
}
#endif

/* HW semaphore and NVIC */
#include "hal_nvic_internal.h"

static uint32_t g_per_cpu_irq_mask;

static void port_syslog_local_cpu_enter_critical(void)
{
    hal_nvic_save_and_set_interrupt_mask(&g_per_cpu_irq_mask);
}

static void port_syslog_local_cpu_exit_critical(void)
{
    hal_nvic_restore_interrupt_mask(g_per_cpu_irq_mask);
}

#ifndef PORT_SYSLOG_SINGLE_CPU_ENV
#include "hal_hw_semaphore.h"
#endif

static void port_syslog_cross_cpu_enter_critical(void)
{
#ifndef PORT_SYSLOG_SINGLE_CPU_ENV
    while (hal_hw_semaphore_take(HW_SEMAPHORE_SYSLOG) != HAL_HW_SEMAPHORE_STATUS_OK);
#endif
}

static void port_syslog_cross_cpu_exit_critical(void)
{
#ifndef PORT_SYSLOG_SINGLE_CPU_ENV
    while (hal_hw_semaphore_give(HW_SEMAPHORE_SYSLOG) != HAL_HW_SEMAPHORE_STATUS_OK);
#endif
}

/* GPT timer */
#include "hal_gpt.h"

static uint32_t port_syslog_get_current_timestamp(void)
{
    uint32_t count = 0;
    uint64_t count64 = 0;

    hal_gpt_get_free_run_count(HAL_GPT_CLOCK_SOURCE_32K, &count);
    count64 = ((uint64_t)count) * 1000 / 32768;

    return (unsigned int)count64;
}

/* memory malloc and free */
#ifdef MTK_CPU_NUMBER_0
static void *port_syslog_malloc(uint32_t size)
{
    return malloc(size);
}

static void port_syslog_free(void *pdata)
{
    free(pdata);
}
#endif

/* Memory remap across CPUs (other CPU remap to primary CPU) */
static uint32_t port_syslog_memory_remap_to_primary_cpu(uint32_t cpu_id, uint32_t address)
{
#ifdef PORT_SYSLOG_SINGLE_CPU_ENV
    return address;
#else
    if (cpu_id == 1) {
        return hal_memview_dsp0_to_cm4(address);
    } else if (cpu_id == 2) {
        return hal_memview_dsp1_to_cm4(address);
    } else {
        return address;
    }
#endif
}

/* Memory remap across CPUs (primary CPU remap to other CPU) */
#ifdef PORT_SYSLOG_SINGLE_CPU_ENV
#define port_syslog_memory_remap_primary_cpu_to_current_cpu(address) address
#else
#if defined(MTK_CPU_NUMBER_1)
#define port_syslog_memory_remap_primary_cpu_to_current_cpu(address) hal_memview_cm4_to_dsp0(address)
#elif defined(MTK_CPU_NUMBER_2)
#define port_syslog_memory_remap_primary_cpu_to_current_cpu(address) hal_memview_cm4_to_dsp1(address)
#else
#define port_syslog_memory_remap_primary_cpu_to_current_cpu(address) address
#endif
#endif

/* syslog setting read/write to non-volatile memory */
#ifdef MTK_CPU_NUMBER_0
#ifdef MTK_NVDM_ENABLE
#include "nvdm.h"
uint32_t port_syslog_read_setting(char *name, uint8_t *p_setting, uint32_t size)
{
    uint32_t max_size;
    nvdm_status_t status;

    max_size = size;
    status = nvdm_read_data_item("syslog", name, p_setting, &max_size);
    if (status != NVDM_STATUS_OK) {
        return false;
    }

    return true;
}
bool port_syslog_save_setting(char *name, uint8_t *p_setting, uint32_t size)
{
    nvdm_status_t status;

    status = nvdm_write_data_item("syslog", name, NVDM_DATA_ITEM_TYPE_RAW_DATA, p_setting, size);
    if (status != NVDM_STATUS_OK) {
        return false;
    }

    return true;
}
bool port_syslog_save_is_available(void)
{
    return true;
}
#else
uint32_t port_syslog_read_setting(char *name, uint8_t *p_setting, uint32_t size)
{
    return false;
}
bool port_syslog_save_setting(char *name, uint8_t *p_setting, uint32_t size)
{
    return false;
}
bool port_syslog_save_is_available(void)
{
    return false;
}
#endif
#endif

#ifdef __cplusplus
}
#endif

#endif

