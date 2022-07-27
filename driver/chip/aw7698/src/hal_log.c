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
 
#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include "hal_platform.h"
#include "hal_log.h"
#include "hal.h"

#if defined(MTK_HAL_NO_LOG_ENABLE) || defined(MTK_DEBUG_LEVEL_NONE)
void log_hal_error_internal(const char *func, int line, const char *message, ...)
{
}
void log_hal_warning_internal(const char *func, int line, const char *message, ...)
{
}
void log_hal_info_internal(const char *func, int line, const char *message, ...)
{
}

void log_hal_dump_internal(const char *func, int line, const char *message, const void *data, int length, ...)
{

}

#elif defined(MTK_HAL_PLAIN_LOG_ENABLE)
void log_hal_error_internal(const char *func, int line, const char *message, ...)
{
    va_list ap;
#ifdef HAL_NVIC_MODULE_ENABLED
    uint32_t mask;
    hal_nvic_save_and_set_interrupt_mask(&mask);
#endif
    va_start(ap, message);
    vprintf(message, ap);
    va_end(ap);
#ifdef HAL_NVIC_MODULE_ENABLED
    hal_nvic_restore_interrupt_mask(mask);
#endif
}
void log_hal_warning_internal(const char *func, int line, const char *message, ...)
{
    va_list ap;
#ifdef HAL_NVIC_MODULE_ENABLED
    uint32_t mask;
    hal_nvic_save_and_set_interrupt_mask(&mask);
#endif
    va_start(ap, message);
    vprintf(message, ap);
    va_end(ap);
#ifdef HAL_NVIC_MODULE_ENABLED
    hal_nvic_restore_interrupt_mask(mask);
#endif
}
void log_hal_info_internal(const char *func, int line, const char *message, ...)
{
    va_list ap;
#ifdef HAL_NVIC_MODULE_ENABLED
    uint32_t mask;
    hal_nvic_save_and_set_interrupt_mask(&mask);
#endif
    va_start(ap, message);
    vprintf(message, ap);
    va_end(ap);
#ifdef HAL_NVIC_MODULE_ENABLED
    hal_nvic_restore_interrupt_mask(mask);
#endif
}

void log_hal_dump_internal(const char *func, int line, const char *message, const void *data, int length, ...)
{
}

#else  /* !defined(MTK_HAL_PLAIN_LOG_ENABLE) */

log_hal_create(PRINT_LEVEL_INFO);



void log_hal_error_internal(const char *func, int line, const char *message, ...)
{
    va_list ap;
    va_start(ap, message);
    vprint_module_log(&log_control_block_hal, func, line, PRINT_LEVEL_ERROR, message, ap);
    va_end(ap);
}

void log_hal_warning_internal(const char *func, int line, const char *message, ...)
{
    va_list ap;
    va_start(ap, message);
    vprint_module_log(&log_control_block_hal, func, line, PRINT_LEVEL_WARNING, message, ap);
    va_end(ap);
}

void log_hal_info_internal(const char *func, int line, const char *message, ...)
{
    va_list ap;
    va_start(ap, message);
    vprint_module_log(&log_control_block_hal, func, line, PRINT_LEVEL_INFO, message, ap);
    va_end(ap);
}

void log_hal_dump_internal(const char *func, int line, const char *message, const void *data, int length, ...)
{
    va_list ap;
    va_start(ap, length);
    vdump_module_buffer(&log_control_block_hal, func, line, PRINT_LEVEL_INFO, data, length, message, ap);
    va_end(ap);
}

#endif /* !defined(MTK_HAL_NO_LOG_ENABLE) */

