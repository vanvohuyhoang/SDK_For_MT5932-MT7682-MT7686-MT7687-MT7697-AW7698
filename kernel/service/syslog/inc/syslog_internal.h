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

#ifndef __SYSLOG_INTERNAL_H__
#define __SYSLOG_INTERNAL_H__

#ifndef MTK_SYSLOG_VERSION_2
#if defined(PRODUCT_VERSION)

/* maximum length of syslog configuration */
#define SYSLOG_FILTER_LEN (1024)

#else

#define SYSLOG_FILTER_LEN (256) /* default if PRODUCT_VERSION is not defined */

#endif

/**
 * Convert the the internal representation to readable format.
 */
const char *log_switch_to_str(log_switch_t log_switch);
const char *print_level_to_str(print_level_t print_level);

/**
 * Convert the readable format to internal representation.
 */
int str_to_log_switch(const char *log_switch_str);
int str_to_print_level(const char *print_level_str);
#endif

#ifdef MTK_SYSLOG_VERSION_2
void print_module_msgid_log(void *handle, print_level_t level, const char *message, uint32_t arg_cnt, ...);
uint32_t dump_module_tlv_buffer(void *handle, print_level_t level, log_type_t type, const void **p_data, uint32_t *p_length);
void filter_config_print_switch(void *handle, log_switch_t log_switch);
void filter_config_print_level(void *handle, print_level_t log_level);
#endif
void log_print_msgid(void *handle, print_level_t level, const char *message, uint32_t arg_cnt, va_list list);
void print_module_log(void *handle, const char *func, int line, print_level_t level, const char *message, ...);
void vprint_module_log(void *handle, const char *func, int line, print_level_t level, const char *message, va_list list);

void dump_module_buffer(void *handle, const char *func, int line, print_level_t level, const void *data, int length, const char *message, ...);
void vdump_module_buffer(void *handle, const char *func, int line, print_level_t level, const void *data, int length, const char *message, va_list list);

typedef void (*f_print_t)(void *handle, const char *func, int line, print_level_t level, const char *message, ...);
typedef void (*f_dump_buffer_t)(void *handle, const char *func, int line, print_level_t level, const void *data, int length, const char *message, ...);
#ifdef MTK_SYSLOG_VERSION_2
typedef void (*f_msg_id_t)(void *handle, print_level_t level, const char *message, uint32_t arg_cnt, ...);
typedef uint32_t (*f_tlv_dump_t)(void *handle, print_level_t level, log_type_t type, const void **p_data, uint32_t *p_length);
#endif

#ifndef MTK_SYSLOG_VERSION_2
int log_write(char *buf, int len);
void log_putchar(char byte);
#endif

#ifdef MTK_SYSLOG_VERSION_2
typedef struct {
    const char *module_name;
    uint32_t log_switch;
    uint32_t print_level;
    f_print_t print_handle;
    f_dump_buffer_t dump_handle;
    f_tlv_dump_t tlv_dump_handle;
    f_msg_id_t msg_id_handle;
} log_control_block_t;
#else
typedef struct {
    const char *module_name;
    log_switch_t log_switch;
    print_level_t print_level;
    f_print_t print_handle;
    f_dump_buffer_t dump_handle;
} log_control_block_t;
#endif

#ifdef MTK_SYSLOG_VERSION_2
extern void log_save_filter(void);
extern void log_offline_dump_idle_hook(void);
bool log_get_cpu_filter_number(uint32_t *p_cpu_number);
bool log_get_cpu_filter_config(uint32_t cpu_id, log_switch_t *p_log_switch, print_level_t *p_print_level);
bool log_get_module_filter_number(uint32_t cpu_id, uint32_t *p_module_number);
bool log_get_module_filter_config(uint32_t cpu_id, uint32_t module_id, const char **p_module_name, log_switch_t *p_log_switch, print_level_t *p_print_level);
bool log_set_cpu_filter_config(uint32_t cpu_id, log_switch_t log_switch, print_level_t print_level);
bool log_set_module_filter_config(uint32_t cpu_id, char *module_name, log_switch_t log_switch, print_level_t print_level);
void log_trigger_save_filter(void);
bool log_query_save_filter(void);
void log_global_turn_off(void);
#endif

#if defined(MTK_SAVE_LOG_TO_FLASH_ENABLE)
bool log_path_switch(bool log_to_flash, bool save_to_nvdm, bool always_dump);
bool log_trigger_write_to_flash(void);
#endif

#ifdef MTK_SYSLOG_VERSION_2
#define LOG_DEBUG_PREFIX(module) "[M:" #module " C:debug F: L: ]: "
#define LOG_INFO_PREFIX(module) "[M:" #module " C:info F: L: ]: "
#define LOG_WARNING_PREFIX(module) "[M:" #module " C:warning F: L: ]: "
#define LOG_ERROR_PREFIX(module) "[M:" #module " C:error F: L: ]: "
#else
#define LOG_DEBUG_PREFIX(module)
#define LOG_INFO_PREFIX(module)
#define LOG_WARNING_PREFIX(module)
#define LOG_ERROR_PREFIX(module)
#endif

#ifndef ATTR_LOG_STRING_LIB
#define ATTR_LOG_STRING_LIB const char
#endif

#endif

