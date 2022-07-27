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

#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdbool.h>
#include <assert.h>
#include "syslog.h"
#include "syslog_port.h"
#include "syslog_port_device.h"

extern port_syslog_device_ops_t *g_syslog_device_ops_array[];

#if !defined(MTK_DEBUG_LEVEL_PRINTF)

#define ALIGNMENT_TO_4_BYTE_BOUNDARY(number, align_byte) \
    do {    \
        if (number % 4) {   \
            align_byte = 4 - number % 4;    \
        }   \
        else {  \
            align_byte = 0; \
        }   \
    } while(0)

#define ARRAY_SIZE(a) (sizeof(a) / sizeof((a)[0]))

#define SYSLOG_INIT_NONE  0x00
#define SYSLOG_EARLY_INIT 0x01
#define SYSLOG_FULL_INIT  0x02

/*
 * Global share variable shared by all CPUs, make sure every member keep align access.
 * [cpu_module_filter_status]:
 *      cpu1_id                   1 byte
 *      cpu1_module_number        1 byte
 *      cpu2_id                   1 byte
 *      cpu2_module_number        1 byte
 *        ..                        ..
 *      cpuN_id                   1 byte
 *      cpuN_module_number        1 byte
 *      valid_cpu1_module1_log_switch   1 byte (high 4 bit)
 *      valid_cpu1_module1_print_level  1 byte (low 4 bit)
 *        ..                              ..
 *      valid_cpu1_moduleM_log_switch   1 byte (high 4 bit)
 *      valid_cpu1_moduleM_print_level  1 byte (low 4 bit)
 *      valid_cpu2_module1_log_switch   1 byte (high 4 bit)
 *      valid_cpu2_module1_print_level  1 byte (low 4 bit)
 *        ..                              ..
 *      valid_cpu2_moduleM_log_switch   1 byte (high 4 bit)
 *      valid_cpu2_moduleM_print_level  1 byte (low 4 bit)
 * [cpu_module_filter]:
 *      per-cpu needs to do address transfer to CM4 view for access by CM4 side.
 */
typedef struct {
    uint32_t sw_wptr;
    uint32_t drop_count;
    uint32_t nest_count;
    uint8_t port_type;
    uint8_t port_index;
    uint8_t init_phase;
    uint8_t reserved;
    uint8_t cpu_log_switch[PORT_SYSLOG_MAX_CPU_NUMBER];
    uint8_t cpu_log_print_level[PORT_SYSLOG_MAX_CPU_NUMBER];
    uint8_t cpu_module_filter_status[PORT_SYSLOG_MAX_MODULE_FILTER_STATUS_SIZE]; /* log filter setting read from NVDM to let per-cpu do initialization with it's log filter array. */
    void *cpu_module_filter[PORT_SYSLOG_MAX_CPU_NUMBER]; /* Init to per-cpu's log filter array when per-cpu call log_set_filter() in it's init phase. */
} syslog_share_variable_t;

static volatile syslog_share_variable_t *g_syslog_share_variable = (volatile syslog_share_variable_t *)(PORT_SYSLOG_SHARE_VARIABLE_BEGIN);


#define MAX_URGENT_LOG_LENGTH                   256
#define MAX_ACTIVE_LOG_CMD_LENGTH               64

#define TLV_SYNC_HEADER                         0xAB

#define TLV_EXCEPTION_HEADER_SIZE               4
#define TLV_LOG_NO_TS_HEADER_SIZE               8
#define TLV_LOG_HEADER_SIZE                     12

#define TEXT_LOG_TYPE_PRINTF                    0x01
#define TEXT_LOG_TYPE_NORMAL                    0x02
#define TEXT_LOG_TYPE_NORMAL_WITH_DUMP          0x03

#define CMD_SUBTYPE_LOG_VERSION_GET             0x00
#define CMD_SUBTYPE_PROCESSOR_STATUS_GET        0x01
#define CMD_SUBTYPE_PROCESSOR_CONFIGURATION_GET 0x02
#define CMD_SUBTYPE_PROCESSOR_CONFIGURATION_SET 0x03
#define CMD_SUBTYPE_MODULE_CONFIGURATION_GET    0x04
#define CMD_SUBTYPE_MODULE_CONFIGURATION_SET    0x05
#define CMD_SUBTYPE_REGISTER_READ               0x06
#define CMD_SUBTYPE_REGISTER_WRITE              0x07
#define CMD_SUBTYPE_PROCESSOR_INFORMATION_GET   0x08
#define CMD_SUBTYPE_FORCE_ASSERT                0x09
#define CMD_SUBTYPE_BOOT_UP_INDICATOR           0x80

#define CMD_RESULT_SUCCESS                      0x00
#define CMD_RESULT_COMMON_FAIL                  0x01
#define CMD_RESULT_PC_COMMAND_ABNORMAL          0x02
#define CMD_RESULT_TARGET_NOT_AVAILABLE         0x03

#define CMD_CPU_STATUS_STOPED                   0x0F
#define CMD_CPU_STATUS_STARTED                  0xF0

#define CMD_END_FLAG_CLEAR                      0x0F
#define CMD_END_FLAG_SET                        0xF0

#define CMD_SAVE_TO_NVDM_CLEAR                  0x0F
#define CMD_SAVE_TO_NVDM_SET                    0xF0

#define CMD_INDEX_RESET_FLAG_SET                0x00
#define CMD_INDEX_RESET_FLAG_CLEAR              0xFF

#define CMD_LOG_VERSION                         0x0001

#define CMD_CPU_NAME_MAX_LENGTRH                16
#define CMD_SDK_VERSION_MAX_LENGTRH             48
#define CMD_BUILD_TIME_MAX_LENGTRH              48

#define TLV_EXCEPTION_STRING_LOG_FORMAT_HEADER(tlv_header, payload_length, total_len) \
    do { \
        tlv_header[0] = (payload_length << 16) | (LOG_TYPE_EXCEPTION_STRING_LOG << 8) | TLV_SYNC_HEADER; \
        total_len = payload_length + TLV_EXCEPTION_HEADER_SIZE; \
    } while(0)

#define TLV_EXCEPTION_BINARY_LOG_FORMAT_HEADER(tlv_header, payload_length, total_len) \
    do { \
        tlv_header[0] = (payload_length << 16) | (LOG_TYPE_EXCEPTION_BINARY_LOG << 8) | TLV_SYNC_HEADER; \
        total_len = payload_length + TLV_EXCEPTION_HEADER_SIZE; \
    } while(0)

#define TLV_NORMAL_LOG_FORMAT_HEADER(tlv_header, text_type, payload_length, align_length, total_len) \
    do { \
        tlv_header[0] = ((payload_length + align_length + TLV_LOG_HEADER_SIZE - 4) << 16) | (LOG_TYPE_TEXTURE_LOG << 8) | TLV_SYNC_HEADER; \
        tlv_header[1] = 0; \
        tlv_header[2] = port_syslog_get_cpu_id() | (align_length << 8) | (text_type << 16); \
        total_len = payload_length + align_length + TLV_LOG_HEADER_SIZE; \
    } while(0)

#define TLV_DUMP_LOG_FORMAT_HEADER(tlv_header, type, payload_length, align_length, total_len) \
    do { \
        tlv_header[0] = ((payload_length + align_length + TLV_LOG_HEADER_SIZE - 4) << 16) | (type << 8) | TLV_SYNC_HEADER; \
        tlv_header[1] = 0; \
        tlv_header[2] = port_syslog_get_cpu_id() | (align_length << 8); \
        total_len = payload_length + align_length + TLV_LOG_HEADER_SIZE; \
    } while(0)

#define TLV_MSGID_LOG_FORMAT_HEADER(tlv_header, msg_id, payload_length, total_len) \
    do { \
        tlv_header[0] = ((payload_length + TLV_LOG_HEADER_SIZE - 4) << 16) | (LOG_TYPE_MSG_ID_LOG << 8) | TLV_SYNC_HEADER; \
        tlv_header[1] = 0; \
        tlv_header[2] = port_syslog_get_cpu_id() | (msg_id << 8); \
        total_len = payload_length + TLV_LOG_HEADER_SIZE; \
    } while(0)

#define __TLV_PC_RESPONCE_COMMAND_FORMAT_HEADER(tlv_header, subtype, result, end_flag, payload_len, align_len, total_len) \
    do { \
        tlv_header[0] = ((payload_len + align_len + TLV_LOG_NO_TS_HEADER_SIZE - 4) << 16) | (LOG_TYPE_INTERNAL_COMMAND << 8) | TLV_SYNC_HEADER; \
        tlv_header[1] = (align_len << 24) | (end_flag << 16) | (result << 8) | subtype; \
        total_len = TLV_LOG_NO_TS_HEADER_SIZE + payload_len + align_len; \
    } while(0)

#define TLV_PC_RESPONCE_COMMAND_NO_PAYLOAD_FORMAT_HEADER(tlv_header, subtype, total_len) \
    __TLV_PC_RESPONCE_COMMAND_FORMAT_HEADER(tlv_header, subtype, CMD_RESULT_SUCCESS, CMD_END_FLAG_SET, 0, 0, total_len)

#define TLV_PC_RESPONCE_COMMAND_PAYLOAD_WITH_END_FORMAT_HEADER(tlv_header, subtype, payload_length, align_length, total_len) \
    __TLV_PC_RESPONCE_COMMAND_FORMAT_HEADER(tlv_header, subtype, CMD_RESULT_SUCCESS, CMD_END_FLAG_SET, payload_length, align_length, total_len)

#define TLV_PC_RESPONCE_COMMAND_PAYLOAD_NO_END_FORMAT_HEADER(tlv_header, subtype, payload_length, align_length, total_len) \
    __TLV_PC_RESPONCE_COMMAND_FORMAT_HEADER(tlv_header, subtype, CMD_RESULT_SUCCESS, CMD_END_FLAG_CLEAR, payload_length, align_length, total_len)

#define TLV_PC_FAIL_FORMAT_HEADER(tlv_header, subtype, total_len) \
    do { \
        tlv_header[0] = ((TLV_LOG_NO_TS_HEADER_SIZE - 4) << 16) | (LOG_TYPE_INTERNAL_COMMAND << 8) | TLV_SYNC_HEADER; \
        tlv_header[1] = (0 << 24) | (CMD_END_FLAG_SET << 16) | (CMD_RESULT_PC_COMMAND_ABNORMAL << 8) | subtype; \
        total_len = TLV_LOG_NO_TS_HEADER_SIZE; \
    } while(0)

#define TLV_INSERT_TIMESTAMP(tlv_header, timestamp) \
    do { \
        tlv_header[1] = timestamp; \
    } while(0)

NO_INLINE static int32_t pc_package_align_and_assembly(uint8_t sub_type, uint32_t payload_len, bool end_flag, uint8_t *respond_buffer)
{
    uint8_t *p_buffer;
    uint8_t i, align_len;
    uint32_t *p_header;
    uint32_t total_size;

    if (payload_len % 4) {
        align_len = 4 - (payload_len % 4);
    } else {
        align_len = 0;
    }

    p_header = (uint32_t *)respond_buffer;
    if (end_flag == true) {
        TLV_PC_RESPONCE_COMMAND_PAYLOAD_WITH_END_FORMAT_HEADER(p_header, sub_type, payload_len, align_len, total_size);
    } else {
        TLV_PC_RESPONCE_COMMAND_PAYLOAD_NO_END_FORMAT_HEADER(p_header, sub_type, payload_len, align_len, total_size);
    }

    /* Fill align region with 0x00 */
    p_buffer = &respond_buffer[TLV_LOG_NO_TS_HEADER_SIZE + payload_len];
    for (i = 0; i < align_len; i++) {
        *p_buffer++ = 0x00;
    }

    return total_size;
}

typedef struct {
    uint8_t cpu_id;
    uint8_t module_number;
} module_filter_info_t;

#define SYSLOG_FILTER_VALID_MARK 0x80

NO_INLINE static volatile uint8_t *find_start_of_cpu_log_filters(uint32_t cpu_id)
{
    uint32_t offset, i;
    module_filter_info_t *p_filter_info;

    offset = 2 * PORT_SYSLOG_CPU_NUMBER;
    p_filter_info = (module_filter_info_t *)(g_syslog_share_variable->cpu_module_filter_status);
    for (i = 0; i < PORT_SYSLOG_CPU_NUMBER; i++) {
        if ((p_filter_info[i].cpu_id & SYSLOG_FILTER_VALID_MARK) == 0) {
            break;
        }
        if ((p_filter_info[i].cpu_id & (~SYSLOG_FILTER_VALID_MARK)) == cpu_id) {
            break;
        }
        offset += p_filter_info[i].module_number;
    }

    return &(g_syslog_share_variable->cpu_module_filter_status[offset]);
}

#define DROP_PROMPT_LOG ">>> log drop count = "
#define DROP_PROMPT_LOG_FIX_SIZE        (sizeof(DROP_PROMPT_LOG) - 1) /* substract \0 */
#define MAX_DROP_PROMPT_LOG_SIZE_ORIG   (DROP_PROMPT_LOG_FIX_SIZE + 10 + 2 + 3) /* drop count, \r\n, align bytes */
#define MAX_DROP_PROMPT_LOG_SIZE        (((MAX_DROP_PROMPT_LOG_SIZE_ORIG / 4) * 4) + 4)
#define MAX_DROP_PROMPT_LOG_TOTAL_SIZE  (MAX_DROP_PROMPT_LOG_SIZE + TLV_LOG_HEADER_SIZE)

static uint32_t uint_to_str(uint32_t number, uint8_t *str_array)
{
    uint32_t i, value, bits;
    uint8_t byte_array[16];

    bits = 0;
    value = number;
    do {
        byte_array[bits++] = value % 10;
        value /= 10;
    } while (value);

    for (i = 0; i < bits; i++) {
        str_array[i] = byte_array[bits - i - 1] + '0';
    }

    return bits;
}

NO_INLINE static uint32_t buffer_calculate_free_space(uint32_t hw_rptr, uint32_t hw_wptr, uint32_t urgent_log_size)
{
    uint32_t free_space;

    if (hw_wptr >= hw_rptr) {
        if (g_syslog_share_variable->sw_wptr >= hw_wptr) {
            free_space = (PORT_SYSLOG_SHARE_MEMORY_SIZE - g_syslog_share_variable->sw_wptr) + hw_rptr;
        } else {
            free_space = hw_rptr - g_syslog_share_variable->sw_wptr;
        }
    } else {
        free_space = hw_rptr - g_syslog_share_variable->sw_wptr;
    }

    /* In order to simplify the code flow and overhead of logging alloc and update,
     * we always keep 4 bytes reserved space to avoid the full case of logging buffer.
     * As it's difficult to distinguish the different case of full case. */
    if (free_space < 4) {
        free_space = 0;
    } else {
        free_space -= 4;
    }

    /* should keep urgent space for urgent log */
    if (free_space >= urgent_log_size) {
        free_space -= urgent_log_size;
    } else {
        free_space = 0;
    }

    return free_space;
}

NO_INLINE static uint8_t *buffer_copy_data(uint8_t *src_buf, uint8_t *dst_buf, uint32_t log_size)
{
    uint32_t i;
    uint8_t *p_buf;

    if ((PORT_SYSLOG_SHARE_MEMORY_END - (uint32_t)dst_buf) <= log_size) {
        memcpy(dst_buf, src_buf, PORT_SYSLOG_SHARE_MEMORY_END - (uint32_t)dst_buf);
        i = PORT_SYSLOG_SHARE_MEMORY_END - (uint32_t)dst_buf;
        memcpy((uint8_t *)PORT_SYSLOG_SHARE_MEMORY_BEGIN, &src_buf[i], log_size - i);
        p_buf = (uint8_t *)(PORT_SYSLOG_SHARE_MEMORY_BEGIN + log_size - i);
    } else {
        memcpy(dst_buf, src_buf, log_size);
        p_buf = dst_buf + log_size;
    }

    return p_buf;
}

static uint8_t *drop_prompt_log_insert(uint32_t timestamp, uint8_t *p_buf, uint32_t drop_log_count)
{
    uint8_t str_array[16];
    uint32_t tlv_header[3];
    uint32_t i, bits, drop_log_size;

    /* Calculate the size of drop log */
    bits = uint_to_str(drop_log_count, str_array);
    drop_log_size = DROP_PROMPT_LOG_FIX_SIZE + bits;

    /* Insert the drop log */
    TLV_NORMAL_LOG_FORMAT_HEADER(tlv_header, TEXT_LOG_TYPE_PRINTF, MAX_DROP_PROMPT_LOG_SIZE, 0, i);
    TLV_INSERT_TIMESTAMP(tlv_header, timestamp);
    p_buf = buffer_copy_data((uint8_t *)tlv_header, p_buf, TLV_LOG_HEADER_SIZE);
    p_buf = buffer_copy_data((uint8_t *)DROP_PROMPT_LOG, p_buf, DROP_PROMPT_LOG_FIX_SIZE);
    p_buf = buffer_copy_data(str_array, p_buf, bits);
    for (i = 0; i < (MAX_DROP_PROMPT_LOG_SIZE - drop_log_size); i++) {
        str_array[i] = 0;
    }
    p_buf = buffer_copy_data(str_array, p_buf, (MAX_DROP_PROMPT_LOG_SIZE - drop_log_size));

    return p_buf;
}

NO_INLINE uint8_t *log_buffer_pre_alloc(uint32_t log_size, bool is_urgent_log, uint32_t *timestamp)
{
    bool drop_log_insert;
    uint8_t *log_buf_ptr;
    uint32_t free_space, drop_log_count, curr_timestamp;
    uint32_t p_log_fill, p_log_hw_rptr, p_log_hw_wptr;

    port_syslog_local_cpu_enter_critical();
    port_syslog_cross_cpu_enter_critical();

    p_log_hw_rptr = port_syslog_device_get_hw_rptr(g_syslog_share_variable->port_type, g_syslog_share_variable->port_index);
    p_log_hw_wptr = port_syslog_device_get_hw_wptr(g_syslog_share_variable->port_type, g_syslog_share_variable->port_index);

    if (is_urgent_log == false) {
        free_space = buffer_calculate_free_space(p_log_hw_rptr, p_log_hw_wptr, MAX_URGENT_LOG_LENGTH + MAX_ACTIVE_LOG_CMD_LENGTH);
    } else {
        free_space = buffer_calculate_free_space(p_log_hw_rptr, p_log_hw_wptr, 0);
    }

    if (free_space < (log_size + MAX_DROP_PROMPT_LOG_TOTAL_SIZE)) {
        g_syslog_share_variable->drop_count++;
        port_syslog_cross_cpu_exit_critical();
        port_syslog_local_cpu_exit_critical();
        return NULL;
    }
    if (g_syslog_share_variable->drop_count) {
        drop_log_insert = true;
        drop_log_count = g_syslog_share_variable->drop_count;
        g_syslog_share_variable->drop_count = 0;
        log_size += MAX_DROP_PROMPT_LOG_TOTAL_SIZE;
    } else {
        drop_log_insert = false;
    }

    p_log_fill = g_syslog_share_variable->sw_wptr;
    if ((g_syslog_share_variable->sw_wptr + log_size) < PORT_SYSLOG_SHARE_MEMORY_SIZE) {
        g_syslog_share_variable->sw_wptr += log_size;
    } else {
        g_syslog_share_variable->sw_wptr = log_size - (PORT_SYSLOG_SHARE_MEMORY_SIZE - g_syslog_share_variable->sw_wptr);
    }
    g_syslog_share_variable->nest_count++;


    /* Get the timestamp here to make the log sequence align with timestamp sequence */
    curr_timestamp = port_syslog_get_current_timestamp();
    if (timestamp) {
        *timestamp = curr_timestamp;
    }

    /* Don't exit local CPU critical to avoid the possible log hole */
    port_syslog_cross_cpu_exit_critical();

    log_buf_ptr = (uint8_t *)share_log_buffer_address(p_log_fill);

    /* Check whether the drop log is needed to be inserted */
    if (drop_log_insert == true) {
        log_buf_ptr = drop_prompt_log_insert(curr_timestamp, log_buf_ptr, drop_log_count);
    }

    return log_buf_ptr;
}

NO_INLINE void log_buffer_wptr_update(void)
{
    uint32_t p_log_hw_rptr, p_log_hw_wptr;
    uint32_t move_bytes;

    port_syslog_cross_cpu_enter_critical();

    g_syslog_share_variable->nest_count--;
    if (g_syslog_share_variable->nest_count == 0) {
        /* update WPTR */
        p_log_hw_rptr = port_syslog_device_get_hw_rptr(g_syslog_share_variable->port_type, g_syslog_share_variable->port_index);
        p_log_hw_wptr = port_syslog_device_get_hw_wptr(g_syslog_share_variable->port_type, g_syslog_share_variable->port_index);
        if (p_log_hw_wptr >= p_log_hw_rptr) {
            if (g_syslog_share_variable->sw_wptr >= p_log_hw_wptr) {
                move_bytes = g_syslog_share_variable->sw_wptr - p_log_hw_wptr;
            } else {
                move_bytes = g_syslog_share_variable->sw_wptr + (PORT_SYSLOG_SHARE_MEMORY_SIZE - p_log_hw_wptr);
            }
        } else {
            move_bytes = g_syslog_share_variable->sw_wptr - p_log_hw_wptr;
        }
        port_syslog_device_set_hw_wptr(g_syslog_share_variable->port_type, g_syslog_share_variable->port_index, move_bytes);
    }

    port_syslog_cross_cpu_exit_critical();
    port_syslog_local_cpu_exit_critical();
}

#ifdef MTK_CPU_NUMBER_0

#include "exception_handler.h"

static volatile uint32_t g_curr_filter_save_type;
static volatile bool g_filter_save_is_need = false;

static void filter_trigger_save(uint32_t save_type)
{
    if (save_type == CMD_SUBTYPE_PROCESSOR_CONFIGURATION_SET) {
        g_curr_filter_save_type = 0x0F;
        g_filter_save_is_need = true;
    } else if (save_type == CMD_SUBTYPE_MODULE_CONFIGURATION_SET) {
        g_curr_filter_save_type = 0xF0;
        g_filter_save_is_need = true;
    }
}

void log_trigger_save_filter(void)
{
    g_curr_filter_save_type = 0xFF;
    g_filter_save_is_need = true;
}

bool log_query_save_filter(void)
{
    return !g_filter_save_is_need;
}

static void filter_config_load(void)
{
    uint32_t i, size;
    char *syslog_filter_buf;

    syslog_filter_buf = (char *)port_syslog_malloc(PORT_SYSLOG_MODULE_FILTER_STATUS_SIZE);
    assert(syslog_filter_buf != NULL);

    /* this part need each CPU to parse itself */
    size = PORT_SYSLOG_MODULE_FILTER_STATUS_SIZE;
    if (port_syslog_read_setting("module_filter", (uint8_t *)syslog_filter_buf, size) == true) {
        memcpy((uint8_t *)g_syslog_share_variable->cpu_module_filter_status, syslog_filter_buf, size);
    }

    size = 2 * PORT_SYSLOG_CPU_NUMBER;
    if (port_syslog_read_setting("cpu_filter", (uint8_t *)syslog_filter_buf, size) == true) {
        for (i = 0; i < PORT_SYSLOG_CPU_NUMBER; i++) {
            g_syslog_share_variable->cpu_log_switch[i] = syslog_filter_buf[i * 2];
            g_syslog_share_variable->cpu_log_print_level[i] = syslog_filter_buf[i * 2 + 1];
        }
    } else {
        for (i = 0; i < PORT_SYSLOG_CPU_NUMBER; i++) {
            g_syslog_share_variable->cpu_log_switch[i] = DEBUG_LOG_ON;
            g_syslog_share_variable->cpu_log_print_level[i] = PRINT_LEVEL_INFO;
        }
    }

    port_syslog_free(syslog_filter_buf);
}

static void filter_cpu_config_save(void)
{
    uint32_t i;
    char syslog_filter_buf[2 * PORT_SYSLOG_CPU_NUMBER];

    for (i = 0; i < PORT_SYSLOG_CPU_NUMBER; i++) {
        syslog_filter_buf[i * 2] = g_syslog_share_variable->cpu_log_switch[i];
        syslog_filter_buf[i * 2 + 1] = g_syslog_share_variable->cpu_log_print_level[i];
    }

    port_syslog_save_setting("cpu_filter", (uint8_t *)syslog_filter_buf, sizeof(syslog_filter_buf));
}

static void filter_module_config_save(void)
{
    port_syslog_save_setting("module_filter", (uint8_t *)g_syslog_share_variable->cpu_module_filter_status, PORT_SYSLOG_MODULE_FILTER_STATUS_SIZE);
}

static bool filter_parse_pc_cpu_setting(uint32_t payload_len)
{
    uint8_t buffer[16];
    uint8_t cpu_id, cpu_log_switch, cpu_print_level;

    while (payload_len > 0) {
        if (port_syslog_device_receive_data(g_syslog_share_variable->port_type, g_syslog_share_variable->port_index, buffer, 3) != 3) {
            return false;
        }
        cpu_id = buffer[0];
        cpu_log_switch = buffer[1];
        cpu_print_level = buffer[2];
        if ((cpu_id >= PORT_SYSLOG_CPU_NUMBER) ||
                (cpu_log_switch > DEBUG_LOG_OFF) ||
                (cpu_print_level > PRINT_LEVEL_ERROR)) {
            return false;
        }
        g_syslog_share_variable->cpu_log_switch[cpu_id] = cpu_log_switch;
        g_syslog_share_variable->cpu_log_print_level[cpu_id] = cpu_print_level;

        payload_len -= 3;
    }

    return true;
}

static bool filter_parse_pc_module_setting(uint32_t payload_len)
{
    uint8_t buffer[16];
    log_control_block_t *filters;
    module_filter_info_t *p_filter_info;
    volatile uint8_t *p_filters_status;
    uint8_t cpu_id, module_id, module_switch, module_print_level, module_number;

    while (payload_len > 0) {
        if (port_syslog_device_receive_data(g_syslog_share_variable->port_type, g_syslog_share_variable->port_index, buffer, 4) != 4) {
            return false;
        }

        cpu_id = buffer[0];
        if (cpu_id >= PORT_SYSLOG_CPU_NUMBER) {
            return false;
        }
        p_filter_info = (module_filter_info_t *)g_syslog_share_variable->cpu_module_filter_status;
        if ((p_filter_info[cpu_id].cpu_id & SYSLOG_FILTER_VALID_MARK) == 0) {
            return false;
        }

        module_id = buffer[1];
        module_switch = buffer[2];
        module_print_level = buffer[3];
        module_number = p_filter_info[cpu_id].module_number;
        if ((module_id >= module_number) ||
                (module_switch > DEBUG_LOG_OFF) ||
                (module_print_level > PRINT_LEVEL_ERROR)) {
            return false;
        }

        /* This pointer has been remap by log_set_filter() */
        filters = (log_control_block_t *)g_syslog_share_variable->cpu_module_filter[cpu_id];
        if (filters == NULL) {
            return false;
        }
        p_filters_status = find_start_of_cpu_log_filters(cpu_id);
        p_filters_status[module_id] = (module_switch << 4) | module_print_level;

        payload_len -= 4;
    }

    return true;
}

static uint32_t filter_assembly_pc_cpu_setting(uint8_t cpu_id, uint8_t *respond_buffer)
{
    *respond_buffer++ = cpu_id;
    *respond_buffer++ = g_syslog_share_variable->cpu_log_switch[cpu_id];
    *respond_buffer++ = g_syslog_share_variable->cpu_log_print_level[cpu_id];

    return 3;
}

static uint32_t filter_assembly_pc_module_setting(uint8_t cpu_id, bool need_reset, uint8_t *respond_buffer, bool *end_flag)
{
    uint16_t respond_length;
    uint32_t name_len;
    volatile uint8_t *p_filters_status;
    log_control_block_t *filters;
    module_filter_info_t *p_filter_info;
    static uint8_t curr_module_id = 0;

    respond_length = 0;
    p_filters_status = find_start_of_cpu_log_filters(cpu_id);
    /* This filter has been remap already in log_set_filter() */
    filters = (log_control_block_t *)(g_syslog_share_variable->cpu_module_filter[cpu_id]);
    if (filters == NULL) {
        return 0;
    }
    p_filter_info = (module_filter_info_t *)g_syslog_share_variable->cpu_module_filter_status;
    if ((p_filter_info[cpu_id].cpu_id & SYSLOG_FILTER_VALID_MARK) == 0) {
        return 0;
    }

    if (need_reset == true) {
        curr_module_id = 0;
    }

    while (curr_module_id < p_filter_info[cpu_id].module_number) {
        name_len = strlen(filters[curr_module_id].module_name);
        respond_length += name_len + 5;
        if (respond_length > (MAX_URGENT_LOG_LENGTH - 3 - TLV_LOG_NO_TS_HEADER_SIZE)) {
            respond_length -= name_len + 5;
            *end_flag = false;
            return respond_length;
        }
        *respond_buffer++ = cpu_id;  /* cpu id */
        *respond_buffer++ = curr_module_id;  /* module id */
        *respond_buffer++ = name_len; /* module name length */
        strcpy((char *)respond_buffer, filters[curr_module_id].module_name); /* module name */
        respond_buffer += name_len;
        *respond_buffer++ = p_filters_status[curr_module_id] >> 4; /* module log switch */
        *respond_buffer++ = p_filters_status[curr_module_id] & 0x0F; /* module log print level */
        curr_module_id++;
    }

    curr_module_id = 0;
    *end_flag = true;

    return respond_length;
}

void log_save_filter(void)
{
    uint8_t *p_buf, respond_buffer[16];
    uint32_t *p_header, total_size;

    if (g_filter_save_is_need == false) {
        return;
    }

    /* Save log filter setting to NVDM */
    p_header = (uint32_t *)respond_buffer;
    if (g_curr_filter_save_type & 0x0F) {
        filter_cpu_config_save();
        TLV_PC_RESPONCE_COMMAND_NO_PAYLOAD_FORMAT_HEADER(p_header, CMD_SUBTYPE_PROCESSOR_CONFIGURATION_SET, total_size);
    }
    if (g_curr_filter_save_type & 0xF0) {
        filter_module_config_save();
        TLV_PC_RESPONCE_COMMAND_NO_PAYLOAD_FORMAT_HEADER(p_header, CMD_SUBTYPE_MODULE_CONFIGURATION_SET, total_size);
    }

    /*
     * As logging module has no task,
     * so we just put responce command to buffer right now.
     * As we always keep urgent space for this propose,
     * this operation should be always completed successfully.
     * The second command should not be received by target
     * until the prev responce has been sent out.
     * The PC tool needs to promise this with mechanism like UI control and timeout.
     */
    p_buf = log_buffer_pre_alloc(total_size, true, NULL);
    if (p_buf == NULL) {
        assert(0);
    }
    buffer_copy_data(respond_buffer, p_buf, total_size);
    log_buffer_wptr_update();

    /* Reset the save flag. */
    g_filter_save_is_need = false;
}

bool log_get_cpu_filter_number(uint32_t *p_cpu_number)
{
    if (p_cpu_number == NULL) {
        return false;
    }

    *p_cpu_number = PORT_SYSLOG_CPU_NUMBER;

    return true;
}

bool log_get_cpu_filter_config(uint32_t cpu_id, log_switch_t *p_log_switch, print_level_t *p_print_level)
{
    if ((cpu_id >= PORT_SYSLOG_CPU_NUMBER) || (p_log_switch == NULL) || (p_print_level == NULL)) {
        return false;
    }

    *p_log_switch = g_syslog_share_variable->cpu_log_switch[cpu_id];
    *p_print_level = g_syslog_share_variable->cpu_log_print_level[cpu_id];

    return true;
}

bool log_get_module_filter_number(uint32_t cpu_id, uint32_t *p_module_number)
{
    log_control_block_t *filters;
    module_filter_info_t *p_filter_info;

    if ((cpu_id >= PORT_SYSLOG_CPU_NUMBER) || (p_module_number == NULL)) {
        return false;
    }

    /* This filter has been remap already in log_set_filter() */
    filters = (log_control_block_t *)(g_syslog_share_variable->cpu_module_filter[cpu_id]);
    if (filters == NULL) {
        return false;
    }
    p_filter_info = (module_filter_info_t *)g_syslog_share_variable->cpu_module_filter_status;
    if ((p_filter_info[cpu_id].cpu_id & SYSLOG_FILTER_VALID_MARK) == 0) {
        return false;
    }
    *p_module_number = p_filter_info[cpu_id].module_number;

    return true;
}

bool log_get_module_filter_config(uint32_t cpu_id, uint32_t module_id, const char **p_module_name, log_switch_t *p_log_switch, print_level_t *p_print_level)
{
    volatile uint8_t *p_filters_status;
    log_control_block_t *filters;
    module_filter_info_t *p_filter_info;

    if ((cpu_id >= PORT_SYSLOG_CPU_NUMBER) || (p_module_name == NULL) || (p_log_switch == NULL) || (p_print_level == NULL)) {
        return false;
    }

    p_filters_status = find_start_of_cpu_log_filters(cpu_id);
    filters = (log_control_block_t *)(g_syslog_share_variable->cpu_module_filter[cpu_id]);
    if (filters == NULL) {
        return false;
    }
    p_filter_info = (module_filter_info_t *)g_syslog_share_variable->cpu_module_filter_status;
    if ((p_filter_info[cpu_id].cpu_id & SYSLOG_FILTER_VALID_MARK) == 0) {
        return false;
    }

    if (module_id >= p_filter_info[cpu_id].module_number) {
        return false;
    }

    *p_module_name = filters[module_id].module_name;
    *p_log_switch = p_filters_status[module_id] >> 4;
    *p_print_level = p_filters_status[module_id] & 0x0F;

    return true;
}

bool log_set_cpu_filter_config(uint32_t cpu_id, log_switch_t log_switch, print_level_t print_level)
{
    if ((cpu_id >= PORT_SYSLOG_CPU_NUMBER) ||
            (log_switch > DEBUG_LOG_OFF) || (print_level > PRINT_LEVEL_ERROR)) {
        return false;
    }

    g_syslog_share_variable->cpu_log_switch[cpu_id] = log_switch;
    g_syslog_share_variable->cpu_log_print_level[cpu_id] = print_level;

    return true;
}

bool log_set_module_filter_config(uint32_t cpu_id, char *module_name, log_switch_t log_switch, print_level_t print_level)
{
    uint32_t module_id, size_1, size_2;
    volatile uint8_t *p_filters_status;
    log_control_block_t *filters;
    module_filter_info_t *p_filter_info;

    if ((cpu_id >= PORT_SYSLOG_CPU_NUMBER) ||
            (module_name == NULL) ||
            (log_switch > DEBUG_LOG_OFF) || (print_level > PRINT_LEVEL_ERROR)) {
        return false;
    }

    filters = (log_control_block_t *)g_syslog_share_variable->cpu_module_filter[cpu_id];
    if (filters == NULL) {
        return false;
    }
    p_filter_info = (module_filter_info_t *)g_syslog_share_variable->cpu_module_filter_status;
    if ((p_filter_info[cpu_id].cpu_id & SYSLOG_FILTER_VALID_MARK) == 0) {
        return false;
    }

    size_1 = strlen(module_name);
    for (module_id = 0; module_id < p_filter_info[cpu_id].module_number; module_id++) {
        size_2 = strlen(filters[module_id].module_name);
        if (size_1 != size_2) {
            continue;
        }
        if (!strcmp(filters[module_id].module_name, module_name)) {
            break;
        }
    }
    if (module_id >= p_filter_info[cpu_id].module_number) {
        return false;
    }

    p_filters_status = find_start_of_cpu_log_filters(cpu_id);
    p_filters_status[module_id] = (log_switch << 4) | print_level;

    return true;
}

static void filter_trigger_save(uint32_t save_type);

static int32_t pc_get_log_version(uint8_t sub_type, bool need_reset, uint32_t payload_len, uint8_t *respond_buffer)
{
    uint16_t *p_respond_buffer;

    PORT_SYSLOG_UNUSED(need_reset);

    if ((sub_type != CMD_SUBTYPE_LOG_VERSION_GET) || (payload_len != 0)) {
        return 0;
    }

    p_respond_buffer = (uint16_t *) & (respond_buffer[TLV_LOG_NO_TS_HEADER_SIZE]);
    *p_respond_buffer = CMD_LOG_VERSION;

    return pc_package_align_and_assembly(sub_type, 2, true, respond_buffer);
}

static int32_t pc_internal_get_cpu_status(uint8_t sub_type, bool init_flag, uint32_t payload_len, uint8_t *respond_buffer)
{
    uint32_t i, respond_len;
    uint8_t *p_respond_buffer;

    if ((sub_type != CMD_SUBTYPE_PROCESSOR_STATUS_GET) || (payload_len != 0)) {
        return 0;
    }

    p_respond_buffer = (uint8_t *) & (respond_buffer[TLV_LOG_NO_TS_HEADER_SIZE]);
    if (init_flag == true) {
        p_respond_buffer[0] = port_syslog_get_cpu_id();
        p_respond_buffer[1] = CMD_CPU_STATUS_STARTED;
        respond_len = 2;
    } else {
        for (i = 0; i < PORT_SYSLOG_CPU_NUMBER; i++) {
            p_respond_buffer[2 * i] = i;
            if (g_syslog_share_variable->cpu_module_filter[i] != NULL) {
                p_respond_buffer[2 * i + 1] = CMD_CPU_STATUS_STARTED;
            } else {
                p_respond_buffer[2 * i + 1] = CMD_CPU_STATUS_STOPED;
            }
        }
        respond_len = 2 * PORT_SYSLOG_CPU_NUMBER;
    }

    return pc_package_align_and_assembly(sub_type, respond_len, true, respond_buffer);
}

static int32_t pc_get_cpu_status(uint8_t sub_type, bool need_reset, uint32_t payload_len, uint8_t *respond_buffer)
{
    PORT_SYSLOG_UNUSED(need_reset);

    return pc_internal_get_cpu_status(sub_type, false, payload_len, respond_buffer);
}

static int32_t pc_set_cpu_config(uint8_t sub_type, bool need_reset, uint32_t payload_len, uint8_t *respond_buffer)
{
    uint32_t *p_header;
    uint32_t total_size;
    uint8_t save_flag;

    PORT_SYSLOG_UNUSED(need_reset);

    if (payload_len < 1) {
        return 0;
    }
    if ((payload_len - 1) % 3) {
        return 0;
    }

    if (port_syslog_device_receive_data(g_syslog_share_variable->port_type, g_syslog_share_variable->port_index, &save_flag, 1) != 1) {
        return 0;
    }
    payload_len -= 1;

    if (filter_parse_pc_cpu_setting(payload_len) == false) {
        return 0;
    }

    /* Check whether need to save config to NVDM. */
    if (port_syslog_save_is_available() == true) {
        if (save_flag == CMD_SAVE_TO_NVDM_SET) {
            filter_trigger_save(CMD_SUBTYPE_PROCESSOR_CONFIGURATION_SET);
            return -1;
        }
    }

    /* assembly responce package */
    p_header = (uint32_t *)respond_buffer;
    TLV_PC_RESPONCE_COMMAND_NO_PAYLOAD_FORMAT_HEADER(p_header, CMD_SUBTYPE_PROCESSOR_CONFIGURATION_SET, total_size);

    return total_size;
}

static int32_t pc_set_module_config(uint8_t sub_type, bool need_reset, uint32_t payload_len, uint8_t *respond_buffer)
{
    uint32_t *p_header;
    uint32_t total_size;
    uint8_t save_flag;

    PORT_SYSLOG_UNUSED(need_reset);

    if (payload_len < 1) {
        return 0;
    }
    if ((payload_len - 1) % 4) {
        return 0;
    }

    if (port_syslog_device_receive_data(g_syslog_share_variable->port_type, g_syslog_share_variable->port_index, &save_flag, 1) != 1) {
        return 0;
    }
    payload_len -= 1;

    if (filter_parse_pc_module_setting(payload_len) == false) {
        return 0;
    }

    /* Check whether need to save config to NVDM. */
    if (port_syslog_save_is_available() == true) {
        if (save_flag == CMD_SAVE_TO_NVDM_SET) {
            filter_trigger_save(CMD_SUBTYPE_MODULE_CONFIGURATION_SET);
            return -1;
        }
    }

    /* assembly responce package */
    p_header = (uint32_t *)respond_buffer;
    TLV_PC_RESPONCE_COMMAND_NO_PAYLOAD_FORMAT_HEADER(p_header, CMD_SUBTYPE_MODULE_CONFIGURATION_SET, total_size);

    return total_size;
}

static int32_t pc_get_cpu_config(uint8_t sub_type, bool need_reset, uint32_t payload_len, uint8_t *respond_buffer)
{
    uint32_t respond_length;
    uint8_t cpu_id, *p_respond_buffer;

    PORT_SYSLOG_UNUSED(need_reset);

    if (payload_len != 1) {
        return 0;
    }

    if (port_syslog_device_receive_data(g_syslog_share_variable->port_type, g_syslog_share_variable->port_index, &cpu_id, 1) != 1) {
        return 0;
    }
    if (cpu_id >= PORT_SYSLOG_CPU_NUMBER) {
        return 0;
    }

    p_respond_buffer = &(respond_buffer[TLV_LOG_NO_TS_HEADER_SIZE]);
    respond_length = filter_assembly_pc_cpu_setting(cpu_id, p_respond_buffer);

    return pc_package_align_and_assembly(sub_type, respond_length, true, respond_buffer);
}

static int32_t pc_get_module_config(uint8_t sub_type, bool need_reset, uint32_t payload_len, uint8_t *respond_buffer)
{
    bool end_flag;
    uint16_t respond_length;
    uint8_t cpu_id, *p_respond_buffer;

    if (payload_len != 1) {
        return 0;
    }
    if (port_syslog_device_receive_data(g_syslog_share_variable->port_type, g_syslog_share_variable->port_index, &cpu_id, 1) != 1) {
        return 0;
    }
    if (cpu_id >= PORT_SYSLOG_CPU_NUMBER) {
        return 0;
    }

    end_flag = false;
    p_respond_buffer = &(respond_buffer[TLV_LOG_NO_TS_HEADER_SIZE]);
    respond_length = filter_assembly_pc_module_setting(cpu_id, need_reset, p_respond_buffer, &end_flag);
    if (respond_length == 0) {
        return 0;
    }

    return pc_package_align_and_assembly(sub_type, respond_length, end_flag, respond_buffer);
}

static int32_t pc_responce_fail(uint8_t sub_type, uint8_t *respond_buffer)
{
    uint32_t *p_header;
    uint32_t total_size;

    p_header = (uint32_t *)respond_buffer;
    TLV_PC_FAIL_FORMAT_HEADER(p_header, sub_type, total_size);

    return total_size;
}

static int32_t pc_register_write(uint8_t sub_type, bool need_reset, uint32_t payload_len, uint8_t *respond_buffer)
{
    uint32_t *p_header;
    uint32_t total_size, reg_addr, reg_val;

    PORT_SYSLOG_UNUSED(need_reset);

    if ((sub_type != CMD_SUBTYPE_REGISTER_WRITE) || (payload_len != 8)) {
        return 0;
    }

    if (port_syslog_device_receive_data(g_syslog_share_variable->port_type, g_syslog_share_variable->port_index, (uint8_t *)&reg_addr, 4) != 4) {
        return 0;
    }

    if (port_syslog_device_receive_data(g_syslog_share_variable->port_type, g_syslog_share_variable->port_index, (uint8_t *)&reg_val, 4) != 4) {
        return 0;
    }

    *(volatile uint32_t *)(reg_addr) = reg_val;

    p_header = (uint32_t *)respond_buffer;
    TLV_PC_RESPONCE_COMMAND_NO_PAYLOAD_FORMAT_HEADER(p_header, sub_type, total_size);

    return total_size;
}

static int32_t pc_register_read(uint8_t sub_type, bool need_reset, uint32_t payload_len, uint8_t *respond_buffer)
{
    uint32_t reg_addr;
    uint32_t *p_respond_buffer;

    PORT_SYSLOG_UNUSED(need_reset);

    if ((sub_type != CMD_SUBTYPE_REGISTER_READ) || (payload_len != 4)) {
        return 0;
    }

    if (port_syslog_device_receive_data(g_syslog_share_variable->port_type, g_syslog_share_variable->port_index, (uint8_t *)&reg_addr, 4) != 4) {
        return 0;
    }

    p_respond_buffer = (uint32_t *) & (respond_buffer[TLV_LOG_NO_TS_HEADER_SIZE]);
    *p_respond_buffer = *(volatile uint32_t *)(reg_addr);

    return pc_package_align_and_assembly(sub_type, 4, true, respond_buffer);
}

static int32_t pc_get_cpu_information(uint8_t sub_type, bool need_reset, uint32_t payload_len, uint8_t *respond_buffer)
{
    uint8_t cpu_id;
    uint8_t *p_respond_buffer;
    log_control_block_t *filters;

    PORT_SYSLOG_UNUSED(need_reset);

    if ((sub_type != CMD_SUBTYPE_PROCESSOR_INFORMATION_GET) || (payload_len != 1)) {
        return 0;
    }

    if (port_syslog_device_receive_data(g_syslog_share_variable->port_type, g_syslog_share_variable->port_index, &cpu_id, 1) != 1) {
        return 0;
    }
    if (cpu_id >= PORT_SYSLOG_CPU_NUMBER) {
        return 0;
    }

    filters = (log_control_block_t *)(g_syslog_share_variable->cpu_module_filter[cpu_id]);
    if (filters == NULL) {
        return 0;
    }

    p_respond_buffer = (uint8_t *) & (respond_buffer[TLV_LOG_NO_TS_HEADER_SIZE]);
    p_respond_buffer[0] = cpu_id;
    memset(&p_respond_buffer[1], 0x00, CMD_CPU_NAME_MAX_LENGTRH + CMD_SDK_VERSION_MAX_LENGTRH + CMD_BUILD_TIME_MAX_LENGTRH);
    strncpy((char *)&p_respond_buffer[1 + CMD_CPU_NAME_MAX_LENGTRH], (char *)(PORT_SYSLOG_SDK_VERSION_BEGIN + cpu_id * PORT_SYSLOG_SDK_VERSION_LENGTH), PORT_SYSLOG_SDK_VERSION_LENGTH);
    strncpy((char *)&p_respond_buffer[1 + CMD_CPU_NAME_MAX_LENGTRH + CMD_SDK_VERSION_MAX_LENGTRH], (char *)(PORT_SYSLOG_BUILD_TIME_BEGIN + cpu_id * PORT_SYSLOG_BUILD_TIME_LENGTH), PORT_SYSLOG_BUILD_TIME_LENGTH);

    return pc_package_align_and_assembly(sub_type, 1 + CMD_CPU_NAME_MAX_LENGTRH + CMD_SDK_VERSION_MAX_LENGTRH + CMD_BUILD_TIME_MAX_LENGTRH, true, respond_buffer);
}

static int32_t pc_assert_cpu(uint8_t sub_type, bool need_reset, uint32_t payload_len, uint8_t *respond_buffer)
{
    PORT_SYSLOG_UNUSED(need_reset);

    if ((sub_type != CMD_SUBTYPE_FORCE_ASSERT) || (payload_len != 0)) {
        return 0;
    }

    platform_assert("Asserted by PC logging tool", __FILE__, __LINE__);

    return 0;
}

typedef int32_t (*log_command_handler_t)(uint8_t sub_type, bool need_reset, uint32_t payload_len, uint8_t *respond_buffer);

static const log_command_handler_t g_log_command_handlers[] = {
    pc_get_log_version,
    pc_get_cpu_status,
    pc_get_cpu_config,
    pc_set_cpu_config,
    pc_get_module_config,
    pc_set_module_config,
    pc_register_read,
    pc_register_write,
    pc_get_cpu_information,
    pc_assert_cpu,
};

typedef void (*pc_rx_dispatch_func_t)(uint32_t payload_len);

void pc_tool_command_handler(uint32_t length)
{
    bool need_reset;
    uint8_t *p_buf;
    uint8_t sub_type, index;
    uint8_t *pc_command_response_buf;
    int32_t pc_command_response_len;

    if (length <= 1) {
        return;
    }

    if (port_syslog_device_receive_data(g_syslog_share_variable->port_type, g_syslog_share_variable->port_index, &sub_type, 1) != 1) {
        return;
    }
    if (sub_type >= ARRAY_SIZE(g_log_command_handlers)) {
        return;
    }
    if (port_syslog_device_receive_data(g_syslog_share_variable->port_type, g_syslog_share_variable->port_index, &index, 1) != 1) {
        return;
    }
    if (index == CMD_INDEX_RESET_FLAG_CLEAR) {
        need_reset = false;
    } else if (index == CMD_INDEX_RESET_FLAG_SET) {
        need_reset = true;
    } else {
        return;
    }

    pc_command_response_buf = port_syslog_malloc(MAX_URGENT_LOG_LENGTH);
    if (pc_command_response_buf == NULL) {
        assert(0);
    }

    length -= 2;
    pc_command_response_len = g_log_command_handlers[sub_type](sub_type, need_reset, length, pc_command_response_buf);

    /* Temp created syslog task will handler this, so return now. */
    if (pc_command_response_len < 0) {
        port_syslog_free(pc_command_response_buf);
        return;
    }

    /* If command is invalid or abnormal status detect, respond fail package. */
    if (pc_command_response_len == 0) {
        pc_command_response_len = pc_responce_fail(sub_type, pc_command_response_buf);
    }

    /*
     * As logging module has no task,
     * so we just put responce command to buffer right now.
     * As we always keep urgent space for this propose,
     * this operation should be always completed successfully.
     * The second command should not be received by target
     * until the prev responce has been sent out.
     * The PC tool needs to promise this with mechanism like UI control and timeout.
     */
    p_buf = log_buffer_pre_alloc((uint32_t)pc_command_response_len, true, NULL);
    if (p_buf == NULL) {
        assert(0);
    }
    buffer_copy_data(pc_command_response_buf, p_buf, (uint32_t)pc_command_response_len);
    port_syslog_free(pc_command_response_buf);
    log_buffer_wptr_update();
}

static const pc_rx_dispatch_func_t g_pc_rx_dispatch_funcs[] = {
    pc_tool_command_handler,
};

void pc_rx_handler(void)
{
    uint32_t length;
    uint8_t header, type, buffer[32];

    /* find the header */
    while (1) {
        if (port_syslog_device_receive_data(g_syslog_share_variable->port_type, g_syslog_share_variable->port_index, &header, 1) != 1) {
            return;
        }
        if (header == TLV_SYNC_HEADER) {
            break;
        }
    }

    if (port_syslog_device_receive_data(g_syslog_share_variable->port_type, g_syslog_share_variable->port_index, &type, 1) != 1) {
        return;
    }
    if (port_syslog_device_receive_data(g_syslog_share_variable->port_type, g_syslog_share_variable->port_index, buffer, 2) != 2) {
        return;
    }
    length = *(uint16_t *)buffer;
    g_pc_rx_dispatch_funcs[type](length);
}

static volatile bool g_exception_is_happen = false;

static void exception_syslog_callback(void)
{
    /* Reset status of syslog to make other CPU's log to be bypass. */
    g_syslog_share_variable->init_phase = SYSLOG_INIT_NONE;

    port_syslog_device_exception_init(g_syslog_share_variable->port_type, g_syslog_share_variable->port_index);

    g_exception_is_happen = true;
}

/* This API will bypass all log APIs including exception APIs. */
void log_global_turn_off(void)
{
    /* Reset status of syslog to make other CPU's log to be bypass. */
    g_syslog_share_variable->init_phase = SYSLOG_INIT_NONE;

    g_exception_is_happen = false;
}

int log_print_exception_log(const char *message, ...)
{
    va_list list;
    int32_t log_size;
    uint32_t *p_buf;
    char frame_header[PORT_SYSLOG_MAX_ONE_PLAIN_LOG_SIZE];

    if (g_exception_is_happen == false) {
        return 0;
    }

    va_start(list, message);

    p_buf = (uint32_t *)frame_header;
    log_size = vsnprintf(&frame_header[4], sizeof(frame_header) - 4, message, list);
    if (log_size < 0) {
        va_end(list);
        return 0;
    }
    if ((uint32_t)log_size >= (sizeof(frame_header) - 4)) {
        log_size = sizeof(frame_header) - 4 - 1;
    }
    p_buf[0] = (log_size << 16) | (LOG_TYPE_EXCEPTION_STRING_LOG << 8) | TLV_SYNC_HEADER;
    log_size += 4;

    va_end(list);

    port_syslog_device_exception_send(g_syslog_share_variable->port_type, g_syslog_share_variable->port_index, (uint8_t *)frame_header, log_size);

    return log_size;
}

void log_dump_exception_data(const uint8_t *data, uint32_t size)
{
    uint8_t *curr_data;
    uint32_t curr_size, frame_header;

    if (g_exception_is_happen == false) {
        return;
    }

    curr_data = (uint8_t *)data;
    while (size > 0) {
        if (size >= (0x10000 - 4)) {
            curr_size = 0x10000 - 4;
            size -= 0x10000 - 4;
        } else {
            curr_size = size;
            size = 0;
        }

        frame_header = (curr_size << 16) | (LOG_TYPE_EXCEPTION_BINARY_LOG << 8) | TLV_SYNC_HEADER;
        port_syslog_device_exception_send(g_syslog_share_variable->port_type, g_syslog_share_variable->port_index, (uint8_t *)(&frame_header), 4);

        port_syslog_device_exception_send(g_syslog_share_variable->port_type, g_syslog_share_variable->port_index, curr_data, curr_size);
        curr_data += curr_size;
    }
}

static volatile bool g_primary_cpu_syslog_is_initialized = false;

bool log_uart_init(hal_uart_port_t port, hal_uart_baudrate_t baudrate)
{
    exception_config_type exception_config;

    if (g_primary_cpu_syslog_is_initialized == true) {
        return false;
    }

    /* Initialize the global share variables */
    memset((syslog_share_variable_t *)g_syslog_share_variable, 0, sizeof(syslog_share_variable_t));

    g_syslog_share_variable->port_type = LOG_PORT_TYPE_UART;
    g_syslog_share_variable->port_index = port;
    port_syslog_device_early_init(g_syslog_share_variable->port_type, g_syslog_share_variable->port_index, baudrate);

    g_syslog_share_variable->init_phase = SYSLOG_EARLY_INIT;

    exception_config.init_cb = exception_syslog_callback;
    exception_config.dump_cb = NULL;
    exception_register_callbacks(&exception_config);
    g_exception_is_happen = false;

    g_primary_cpu_syslog_is_initialized = true;

    return true;
}

bool log_init(syslog_port_type_t port_type, uint8_t port_index)
{
    if ((g_primary_cpu_syslog_is_initialized == false) ||
            (g_syslog_share_variable->init_phase != SYSLOG_EARLY_INIT)) {
        return false;
    }

    /* load logging device setting */
    port_syslog_device_get_setting((log_port_type_t *)&port_type, &port_index);
    g_syslog_share_variable->port_type = port_type;
    g_syslog_share_variable->port_index = port_index;

    /* logging device post initialization */
    port_syslog_device_post_init(port_type, port_index, PORT_SYSLOG_SHARE_MEMORY_BEGIN, PORT_SYSLOG_SHARE_MEMORY_SIZE, pc_rx_handler);

    /* load logging filters setting */
    filter_config_load();

    g_syslog_share_variable->init_phase = SYSLOG_FULL_INIT;

    return true;
}

#endif

#if !defined(MTK_DEBUG_LEVEL_NONE)

static void pc_initial_sync(void)
{
    uint8_t *p_buf;
    uint8_t buffer[MAX_ACTIVE_LOG_CMD_LENGTH];
    uint32_t total_size;

    p_buf = &(buffer[TLV_LOG_NO_TS_HEADER_SIZE]);
    p_buf[0] = port_syslog_get_cpu_id();
    p_buf[1] = CMD_CPU_STATUS_STARTED;
    total_size = (uint32_t)pc_package_align_and_assembly(CMD_SUBTYPE_BOOT_UP_INDICATOR, 2, true, buffer);

    p_buf = log_buffer_pre_alloc(total_size, true, NULL);
    if (p_buf == NULL) {
        assert(0);
    }
    buffer_copy_data(buffer, p_buf, total_size);
    log_buffer_wptr_update();
}

bool log_set_filter(void)
{
    uint32_t i, cpu_id, offset;
    uint32_t runtime_filter_number;
    module_filter_info_t *p_filter_info;
    volatile uint8_t *p_filters;
    log_control_block_t *entries;

    if (g_syslog_share_variable->init_phase != SYSLOG_FULL_INIT) {
        return false;
    }

    port_syslog_device_init_hook(g_syslog_share_variable->port_type, g_syslog_share_variable->port_index);

    cpu_id = port_syslog_get_cpu_id();

    port_syslog_build_time_sdk_version_copy(cpu_id);

    pc_initial_sync();

    /* Calculate the number of runtime module filter */
    runtime_filter_number = (LOG_FILTER_END - LOG_FILTER_START) / sizeof(log_control_block_t);
    /* As this variable need to be by accessed by master CPU, so maybe need to do remap */
    entries = (log_control_block_t *)LOG_FILTER_START;
    g_syslog_share_variable->cpu_module_filter[cpu_id] = (void *)port_syslog_memory_remap_to_primary_cpu(cpu_id, (uint32_t)entries);

    p_filter_info = (module_filter_info_t *)g_syslog_share_variable->cpu_module_filter_status;

    for (i = 0; i < PORT_SYSLOG_CPU_NUMBER; i++) {
        if (!(p_filter_info[i].cpu_id & SYSLOG_FILTER_VALID_MARK)) {
            break;
        }
        if ((p_filter_info[i].cpu_id & (~SYSLOG_FILTER_VALID_MARK)) == cpu_id) {
            if (p_filter_info[i].module_number != runtime_filter_number) {
                assert(0);
                return false;
            }
            return true;
        }
    }
    offset = i;

    p_filters = find_start_of_cpu_log_filters(cpu_id);
    for (i = 0; i < runtime_filter_number; i++) {
        p_filters[i] = ((uint8_t)(entries[i].log_switch) << 4) | (uint8_t)(entries[i].print_level);
    }
    p_filter_info[offset].cpu_id = cpu_id | SYSLOG_FILTER_VALID_MARK;
    p_filter_info[offset].module_number = runtime_filter_number;

    return true;
}

void filter_config_print_switch(void *handle, log_switch_t log_switch)
{
    uint32_t index, cpu_id;
    volatile uint8_t *p_filters;

    cpu_id = port_syslog_get_cpu_id();
    if (g_syslog_share_variable->cpu_module_filter[cpu_id] == NULL) {
        return;
    }

    index = ((uint32_t)handle - (uint32_t)LOG_FILTER_START) / sizeof(log_control_block_t);
    p_filters = find_start_of_cpu_log_filters(cpu_id);

    p_filters[index] &= 0x0F;
    p_filters[index] |= log_switch << 4;
}

void filter_config_print_level(void *handle, print_level_t log_level)
{
    uint32_t index, cpu_id;
    volatile uint8_t *p_filters;

    cpu_id = port_syslog_get_cpu_id();
    if (g_syslog_share_variable->cpu_module_filter[cpu_id] == NULL) {
        return;
    }

    index = ((uint32_t)handle - (uint32_t)LOG_FILTER_START) / sizeof(log_control_block_t);
    p_filters = find_start_of_cpu_log_filters(cpu_id);

    p_filters[index] &= 0xF0;
    p_filters[index] |= log_level;
}

static bool filter_runtime_check(const void *is_module_control, log_control_block_t *context, print_level_t level)
{
    uint32_t offset;
    volatile uint8_t *p_filters;

    /* If current CPU debug level is turn off, bypass the log. */
    if (g_syslog_share_variable->cpu_log_switch[port_syslog_get_cpu_id()] == DEBUG_LOG_OFF) {
        return false;
    }

    /* Check the address range to detect the seperate build log filter */
    if (((uint32_t)context >= (uint32_t)LOG_FILTER_END) || ((uint32_t)context < (uint32_t)LOG_FILTER_START)) {
        if ((is_module_control != NULL) &&
                ((context->log_switch == DEBUG_LOG_OFF) ||
                 (level < g_syslog_share_variable->cpu_log_print_level[port_syslog_get_cpu_id()]) ||
                 (level < context->print_level))) {
            return false;
        }

        return true;
    }

    /* For LOG_*()/LOG_MSGID_*()/LOG_TLVDUMP_*(), ignore it if
     * 1. If module's debug level is turn off or
     * 2. If current log level is lower than current CPU's debug level.
     * 3. If current log level is lower than module's debug level.
     */
    offset = ((uint32_t)context - (uint32_t)(g_syslog_share_variable->cpu_module_filter[port_syslog_get_cpu_id()])) / sizeof(log_control_block_t);
    p_filters = find_start_of_cpu_log_filters(port_syslog_get_cpu_id());
    if ((is_module_control != NULL) &&
            (((p_filters[offset] >> 4) == DEBUG_LOG_OFF) ||
             (level < g_syslog_share_variable->cpu_log_print_level[port_syslog_get_cpu_id()]) ||
             (level < (p_filters[offset] & 0x0F)))) {
        return false;
    }

    return true;
}

void log_sleep_restore_callback(void)
{
    g_syslog_share_variable->sw_wptr = 0;
}

static const char *print_level_table[] = {"debug", "info", "warning", "error"};

#define change_level_to_string(level) \
  ((level) - PRINT_LEVEL_DEBUG <= PRINT_LEVEL_ERROR) ? print_level_table[level] : "debug"

log_create_module(common, PRINT_LEVEL_INFO);

NO_INLINE static bool check_log_control(const void *is_module_control, log_control_block_t *context, print_level_t level)
{
    /* check whether syslog is initialized. */
    if (g_syslog_share_variable->init_phase == SYSLOG_INIT_NONE) {
        return false;
    }

    /* check whether CPU control when not full initialize. */
    if (g_syslog_share_variable->init_phase == SYSLOG_EARLY_INIT) {
        if (port_syslog_get_cpu_id() != 0) {
            return false;
        }
    }

    /* check whether debug level control when full initialize */
    if (g_syslog_share_variable->init_phase == SYSLOG_FULL_INIT) {
        port_syslog_device_logging_hook(g_syslog_share_variable->port_type, g_syslog_share_variable->port_index);
        if (filter_runtime_check(is_module_control, context, level) == false) {
            return false;
        }
    }

    return true;
}

#ifdef MTK_SYSLOG_SUB_FEATURE_STRING_LOG_SUPPORT
NO_INLINE static void string_log_handler(void *handle,
                               const char *func,
                               int line,
                               print_level_t level,
                               const char *message,
                               va_list list,
                               const void *data,
                               int data_len)
{
    uint8_t *p_log_fill;
    char *p_frame_header;
    char frame_header[PORT_SYSLOG_MAX_ONE_LOG_SIZE];
    uint32_t *p_buf, timestamp, dummy_bytes;
    int32_t i, step_size, log_size, align_size, max_size, payload_size, total_size;
    log_control_block_t *context = (log_control_block_t *)handle;

    /* check log control */
    if (check_log_control(func, context, level) == false) {
        return;
    }

    /* Keep space for TLV header at the beginning of log */
    max_size = sizeof(frame_header) - TLV_LOG_HEADER_SIZE;

    if (data) {
        max_size -= 1; /* keep one byte more for \0 */
    }

    /* Format the log header and calculate the size */
    p_frame_header = (char *)&frame_header[TLV_LOG_HEADER_SIZE];
    if (func) {
        log_size = snprintf(p_frame_header, max_size,
                            "[M:%s C:%s F:%s L:%d]: ",
                            context->module_name,
                            change_level_to_string(level),
                            func,
                            line);
        if (log_size < 0) {
            return;
        }
        if (log_size >= max_size) {
            log_size = max_size - 1;
        }
    } else {
        log_size = 0;
    }

    /* Format the log string/arguments and calculate the size */
    max_size -= log_size;
    p_frame_header += log_size;
    step_size = vsnprintf(p_frame_header, max_size, message, list);
    if (step_size < 0) {
        return;
    }
    if (step_size >= max_size) {
        step_size = max_size - 1;
    }
    log_size += step_size;

    /* Calculate align bytes and total bytes */
    align_size = 0;
    payload_size = data_len + log_size;
    p_frame_header += step_size;
    if (data) {
        /* need insert \0 between string and binary data when call LOG_HEXDUMP_*() */
        *p_frame_header = 0;
        payload_size += 1;
    }
    if (g_syslog_share_variable->init_phase == SYSLOG_FULL_INIT) {
        ALIGNMENT_TO_4_BYTE_BOUNDARY(payload_size, align_size);
        if (!data) {
            for (i = 0; i < align_size; i++) {
                *p_frame_header++ = 0;
            }
        }
    }

    /* Fill TLV header at the beginning of log */
    p_buf = (uint32_t *)frame_header;
    if (func) {
        if (data) {
            TLV_NORMAL_LOG_FORMAT_HEADER(p_buf, TEXT_LOG_TYPE_NORMAL_WITH_DUMP, payload_size, align_size, total_size);
        } else {
            TLV_NORMAL_LOG_FORMAT_HEADER(p_buf, TEXT_LOG_TYPE_NORMAL, payload_size, align_size, total_size);
        }
    } else {
        TLV_NORMAL_LOG_FORMAT_HEADER(p_buf, TEXT_LOG_TYPE_PRINTF, payload_size, align_size, total_size);
    }

#ifdef MTK_CPU_NUMBER_0
    /* Use UART polling mode to send log dirrectly when DMA mode is not ready */
    if (g_syslog_share_variable->init_phase == SYSLOG_EARLY_INIT) {
        /* Send string part of log */
        log_size += TLV_LOG_HEADER_SIZE;
        if (data) {
            log_size += 1;
        }
        port_syslog_device_early_send(g_syslog_share_variable->port_type, g_syslog_share_variable->port_index, (uint8_t *)frame_header, log_size);
        if (data) {
            /* Send binary part of log */
            port_syslog_device_early_send(g_syslog_share_variable->port_type, g_syslog_share_variable->port_index, (uint8_t *)data, data_len);
        }
        return;
    }
#endif

    /* Alloc the buffer space for the log */
    p_log_fill = log_buffer_pre_alloc(total_size, false, &timestamp);
    if (p_log_fill == NULL) {
        return;
    }

    /* Fill the timestamp to log buffer */
    p_buf = (uint32_t *)frame_header;
    TLV_INSERT_TIMESTAMP(p_buf, timestamp);

    /* Copy the string part to the buffer */
    log_size += TLV_LOG_HEADER_SIZE;
    if (data) {
        log_size += 1;
    } else {
        log_size += align_size;
    }
    p_log_fill = buffer_copy_data((uint8_t *)frame_header, p_log_fill, log_size);

    if (data) {
        /* Copy binary part to the buffer */
        p_log_fill = buffer_copy_data((uint8_t *)data, p_log_fill, data_len);

        /* Copy align bytes to the buffer */
        dummy_bytes = 0;
        buffer_copy_data((uint8_t *)&dummy_bytes, p_log_fill, align_size);
    }

    /* Try to update HW WPTR if it's possible. */
    log_buffer_wptr_update();
}

void vprint_module_log(void *handle,
                       const char *func,
                       int line,
                       print_level_t level,
                       const char *message,
                       va_list list)
{
    string_log_handler(handle, func, line, level, message, list, NULL, 0);
}

/* For LOG_*() */
void print_module_log(void *handle,
                      const char *func,
                      int line,
                      print_level_t level,
                      const char *message, ...)
{
    va_list ap;

    va_start(ap, message);
    vprint_module_log(handle, func, line, level, message, ap);
    va_end(ap);
}

/* For printf() */
int __wrap_printf( const char *format, ... )
{
    va_list ap;

    va_start(ap, format);
    vprint_module_log(&log_control_block_common, NULL, 0, 0, format, ap);
    va_end(ap);

    return 0;
}

void vdump_module_buffer(void *handle,
                         const char *func,
                         int line,
                         print_level_t level,
                         const void *data,
                         int length,
                         const char *message,
                         va_list list)
{
    string_log_handler(handle, func, line, level, message, list, data, length);
}

/* For LOG_HEXDUMP_*() */
void dump_module_buffer(void *handle,
                        const char *func,
                        int line,
                        print_level_t level,
                        const void *data,
                        int length,
                        const char *message, ...)
{
    va_list ap;

    va_start(ap, message);
    vdump_module_buffer(handle, func, line, level, data, length, message, ap);
    va_end(ap);
}
#else
void print_module_log(void *handle,
                      const char *func,
                      int line,
                      print_level_t level,
                      const char *message, ...)
{
    PORT_SYSLOG_UNUSED(handle);
    PORT_SYSLOG_UNUSED(func);
    PORT_SYSLOG_UNUSED(line);
    PORT_SYSLOG_UNUSED(level);
    PORT_SYSLOG_UNUSED(message);
}

int __wrap_printf( const char *format, ... )
{
    PORT_SYSLOG_UNUSED(format);

    return 0;
}

void dump_module_buffer(void *handle,
                        const char *func,
                        int line,
                        print_level_t level,
                        const void *data,
                        int length,
                        const char *message, ...)
{
    PORT_SYSLOG_UNUSED(handle);
    PORT_SYSLOG_UNUSED(func);
    PORT_SYSLOG_UNUSED(line);
    PORT_SYSLOG_UNUSED(level);
    PORT_SYSLOG_UNUSED(data);
    PORT_SYSLOG_UNUSED(length);
    PORT_SYSLOG_UNUSED(message);
}
#endif

#ifdef MTK_SYSLOG_SUB_FEATURE_BINARY_LOG_SUPPORT

#define MAX_TLV_DUMP_DATA_SIZE 2048

NO_INLINE static int32_t binary_log_handler(void *handle,
                                  print_level_t level,
                                  log_type_t type,
                                  const char *message,
                                  const void **p_data,
                                  uint32_t *p_length)
{
    uint8_t *p_log_fill;
    uint32_t i, tlv_header[3], align_size, total_size, curr_size, payload_size, dummy_word, timestamp;
    log_control_block_t *context = (log_control_block_t *)handle;

    /* check log control */
    if (check_log_control(p_data, context, level) == false) {
        return 0;
    }

    /* Calculate total size of payload */
    payload_size = 0;
    for (i = 0; p_data[i] != NULL; i++) {
        payload_size += p_length[i];
    }

    /* Limit the max size of dump data when calling LOG_TLVDUMP_*() */
    if (g_syslog_share_variable->init_phase == SYSLOG_FULL_INIT) {
        if (payload_size > MAX_TLV_DUMP_DATA_SIZE) {
            payload_size = MAX_TLV_DUMP_DATA_SIZE;
        }
    }

    /* Calculate the align size of dump data when calling LOG_TLVDUMP_*() */
    if (g_syslog_share_variable->init_phase == SYSLOG_FULL_INIT) {
        ALIGNMENT_TO_4_BYTE_BOUNDARY(payload_size, align_size);
    } else {
        align_size = 0;
    }

    /* Initialize the TLV header */
    if (type == LOG_TYPE_MSG_ID_LOG) {
        TLV_MSGID_LOG_FORMAT_HEADER(tlv_header, ((uint32_t)message - PORT_SYSLOG_MSG_ADDR_OFFSET), payload_size, total_size);
    } else {
        TLV_DUMP_LOG_FORMAT_HEADER(tlv_header, type, payload_size, align_size, total_size);
    }

#ifdef MTK_CPU_NUMBER_0
    /* Use UART polling mode to send log dirrectly when DMA mode is not ready */
    if (g_syslog_share_variable->init_phase == SYSLOG_EARLY_INIT) {
        port_syslog_device_early_send(g_syslog_share_variable->port_type, g_syslog_share_variable->port_index, (uint8_t *)tlv_header, TLV_LOG_HEADER_SIZE);
        for (i = 0; p_data[i] != NULL; i++) {
            port_syslog_device_early_send(g_syslog_share_variable->port_type, g_syslog_share_variable->port_index, (uint8_t *)p_data[i], p_length[i]);
        }
        return payload_size;
    }
#endif

    /* Alloc the buffer space for the log */
    p_log_fill = log_buffer_pre_alloc(total_size, false, &timestamp);
    if (p_log_fill == NULL) {
        return 0;
    }

    /* Re-fill the timestamp */
    TLV_INSERT_TIMESTAMP(tlv_header, timestamp);

    /* Copy TLV header to VFIFO buffer */
    p_log_fill = buffer_copy_data((uint8_t *)tlv_header, p_log_fill, TLV_LOG_HEADER_SIZE);

    /* Copy data/parameters to VFIFO buffer */
    total_size = payload_size;
    for (i = 0; p_data[i] != NULL; i++) {
        if (total_size >= p_length[i]) {
            curr_size = p_length[i];
            total_size -= p_length[i];
        } else {
            curr_size = total_size;
            total_size = 0;
        }
        p_log_fill = buffer_copy_data((uint8_t *)p_data[i], p_log_fill, curr_size);
        if (total_size == 0) {
            break;
        }
    }

    /* Copy align bytes to VFIFO buffer */
    if (align_size) {
        dummy_word = 0;
        buffer_copy_data((uint8_t *)&dummy_word, p_log_fill, align_size);
    }

    /* Update HW WPTR if it's possible. */
    log_buffer_wptr_update();

    return payload_size;
}

#define MAX_SYSLOG_MSG_ID_PARAMETER_NUMBER 20

void log_print_msgid(void *handle,
                             print_level_t level,
                             const char *message,
                             uint32_t arg_cnt,
                             va_list list)
{
    uint32_t i, buffer[MAX_SYSLOG_MSG_ID_PARAMETER_NUMBER];
    void *p_data[2];
    uint32_t p_length[2];

    if (arg_cnt > MAX_SYSLOG_MSG_ID_PARAMETER_NUMBER) {
        return;
    }

    for (i = 0; i < arg_cnt; i++) {
        buffer[i] = va_arg(list, uint32_t);
    }

    p_data[0] = buffer;
    p_data[1] = NULL;
    p_length[0] = arg_cnt * sizeof(uint32_t);
    p_length[1] = 0;
    binary_log_handler(handle, level, LOG_TYPE_MSG_ID_LOG, message, (const void **)p_data, p_length);
}

/* For LOG_MSGID_*() */
void print_module_msgid_log(void *handle,
                            print_level_t level,
                            const char *message,
                            uint32_t arg_cnt, ...)
{
    va_list list;

    va_start(list, arg_cnt);
    log_print_msgid(handle, level, message, arg_cnt, list);
    va_end(list);
}

/* For LOG_TLVDUMP_*() */
uint32_t dump_module_tlv_buffer(void *handle,
                                print_level_t level,
                                log_type_t type,
                                const void **p_data,
                                uint32_t *p_length)
{
    return binary_log_handler(handle, level, type, NULL, p_data, p_length);
}
#else
void print_module_msgid_log(void *handle,
                            print_level_t level,
                            const char *message,
                            uint32_t arg_cnt, ...)
{
    PORT_SYSLOG_UNUSED(handle);
    PORT_SYSLOG_UNUSED(level);
    PORT_SYSLOG_UNUSED(message);
    PORT_SYSLOG_UNUSED(arg_cnt);
}

uint32_t dump_module_tlv_buffer(void *handle,
                                print_level_t level,
                                log_type_t type,
                                const void **p_data,
                                uint32_t *p_length)
{
    PORT_SYSLOG_UNUSED(handle);
    PORT_SYSLOG_UNUSED(level);
    PORT_SYSLOG_UNUSED(type);
    PORT_SYSLOG_UNUSED(p_data);
    PORT_SYSLOG_UNUSED(p_length);

    return 0;
}

#endif

#else

bool log_set_filter(void)
{
    return false;
}

void filter_config_print_switch(void *handle, log_switch_t log_switch)
{
    PORT_SYSLOG_UNUSED(handle);
    PORT_SYSLOG_UNUSED(log_switch);
}

void filter_config_print_level(void *handle, print_level_t log_level)
{
    PORT_SYSLOG_UNUSED(handle);
    PORT_SYSLOG_UNUSED(log_level);
}

void log_sleep_restore_callback(void)
{
    g_syslog_share_variable->sw_wptr = 0;
}

void vprint_module_log(void *handle,
                       const char *func,
                       int line,
                       print_level_t level,
                       const char *message,
                       va_list list)
{
    PORT_SYSLOG_UNUSED(handle);
    PORT_SYSLOG_UNUSED(func);
    PORT_SYSLOG_UNUSED(line);
    PORT_SYSLOG_UNUSED(level);
    PORT_SYSLOG_UNUSED(message);
    PORT_SYSLOG_UNUSED(list);
}

void print_module_log(void *handle,
                      const char *func,
                      int line,
                      print_level_t level,
                      const char *message, ...)
{
    PORT_SYSLOG_UNUSED(handle);
    PORT_SYSLOG_UNUSED(func);
    PORT_SYSLOG_UNUSED(line);
    PORT_SYSLOG_UNUSED(level);
    PORT_SYSLOG_UNUSED(message);
}

int __wrap_printf( const char *format, ... )
{
    PORT_SYSLOG_UNUSED(format);

    return 0;
}

void vdump_module_buffer(void *handle,
                         const char *func,
                         int line,
                         print_level_t level,
                         const void *data,
                         int length,
                         const char *message,
                         va_list list)
{
    PORT_SYSLOG_UNUSED(handle);
    PORT_SYSLOG_UNUSED(func);
    PORT_SYSLOG_UNUSED(line);
    PORT_SYSLOG_UNUSED(level);
    PORT_SYSLOG_UNUSED(data);
    PORT_SYSLOG_UNUSED(length);
    PORT_SYSLOG_UNUSED(message);
    PORT_SYSLOG_UNUSED(list);
}

void dump_module_buffer(void *handle,
                        const char *func,
                        int line,
                        print_level_t level,
                        const void *data,
                        int length,
                        const char *message, ...)
{
    PORT_SYSLOG_UNUSED(handle);
    PORT_SYSLOG_UNUSED(func);
    PORT_SYSLOG_UNUSED(line);
    PORT_SYSLOG_UNUSED(level);
    PORT_SYSLOG_UNUSED(data);
    PORT_SYSLOG_UNUSED(length);
    PORT_SYSLOG_UNUSED(message);
}

void log_print_msgid(void *handle,
                             print_level_t level,
                             const char *message,
                             uint32_t arg_cnt,
                             va_list list)
{
    PORT_SYSLOG_UNUSED(handle);
    PORT_SYSLOG_UNUSED(level);
    PORT_SYSLOG_UNUSED(message);
    PORT_SYSLOG_UNUSED(arg_cnt);
    PORT_SYSLOG_UNUSED(list);
}

void print_module_msgid_log(void *handle,
                            print_level_t level,
                            const char *message,
                            uint32_t arg_cnt, ...)
{
    PORT_SYSLOG_UNUSED(handle);
    PORT_SYSLOG_UNUSED(level);
    PORT_SYSLOG_UNUSED(message);
    PORT_SYSLOG_UNUSED(arg_cnt);
}

uint32_t dump_module_tlv_buffer(void *handle,
                                print_level_t level,
                                log_type_t type,
                                const void **p_data,
                                uint32_t *p_length)
{
    PORT_SYSLOG_UNUSED(handle);
    PORT_SYSLOG_UNUSED(level);
    PORT_SYSLOG_UNUSED(type);
    PORT_SYSLOG_UNUSED(p_data);
    PORT_SYSLOG_UNUSED(p_length);

    return 0;
}

#endif

#else

typedef struct {
    uint8_t port_index;
} syslog_share_variable_t;

static volatile syslog_share_variable_t *g_syslog_share_variable = (volatile syslog_share_variable_t *)(PORT_SYSLOG_SHARE_VARIABLE_BEGIN);

log_create_module(common, PRINT_LEVEL_INFO);

void log_sleep_restore_callback(void)
{}

#ifdef MTK_CPU_NUMBER_0

#include "exception_handler.h"

static volatile bool g_primary_cpu_syslog_is_initialized = false;
static volatile bool g_exception_is_happen = false;

static void exception_syslog_callback(void)
{
    port_syslog_device_exception_init(LOG_PORT_TYPE_UART, g_syslog_share_variable->port_index);

    g_exception_is_happen = true;
}

/* This API will bypass all log APIs including exception APIs. */
void log_global_turn_off(void)
{
    /* Reset status of syslog to make other CPU's log to be bypass. */
    g_exception_is_happen = false;
}

int log_print_exception_log(const char *message, ...)
{
    va_list list;
    int32_t log_size;
    char frame_header[PORT_SYSLOG_MAX_ONE_PLAIN_LOG_SIZE];

    if (g_exception_is_happen == false) {
        return 0;
    }

    va_start(list, message);

    log_size = vsnprintf(frame_header, sizeof(frame_header), message, list);
    if (log_size < 0) {
        va_end(list);
        return 0;
    }
    if ((uint32_t)log_size >= sizeof(frame_header)) {
        log_size = sizeof(frame_header) - 1;
    }

    va_end(list);

    port_syslog_device_exception_send(LOG_PORT_TYPE_UART, g_syslog_share_variable->port_index, (uint8_t *)frame_header, log_size);

    return log_size;
}

/* Internal API for exception data dump. */
void log_dump_exception_data(const uint8_t *data, uint32_t size)
{
    uint8_t *curr_data;
    uint32_t curr_size;

    if (g_exception_is_happen == false) {
        return;
    }

    curr_data = (uint8_t *)data;
    while (size > 0) {
        if (size >= (0x10000 - 4)) {
            curr_size = 0x10000 - 4;
            size -= 0x10000 - 4;
        } else {
            curr_size = size;
            size = 0;
        }

        port_syslog_device_exception_send(LOG_PORT_TYPE_UART, g_syslog_share_variable->port_index, curr_data, curr_size);
        curr_data += curr_size;
    }
}


bool log_uart_init(hal_uart_port_t port, hal_uart_baudrate_t baudrate)
{
    exception_config_type exception_config;

    if (g_primary_cpu_syslog_is_initialized == true) {
        return false;
    }

    port_syslog_device_early_init(LOG_PORT_TYPE_UART, port, baudrate);

    exception_config.init_cb = exception_syslog_callback;
    exception_config.dump_cb = NULL;
    exception_register_callbacks(&exception_config);
    g_exception_is_happen = false;

    g_primary_cpu_syslog_is_initialized = true;

    return true;
}

bool log_init(syslog_port_type_t port_type, uint8_t port_index)
{
    PORT_SYSLOG_UNUSED(port_type);
    PORT_SYSLOG_UNUSED(port_index);

    return false;
}

void log_save_filter(void)
{}

#endif

bool log_set_filter(void)
{
    return false;
}

void filter_config_print_switch(void *handle, log_switch_t log_switch)
{
    PORT_SYSLOG_UNUSED(handle);
    PORT_SYSLOG_UNUSED(log_switch);
}

void filter_config_print_level(void *handle, print_level_t log_level)
{
    PORT_SYSLOG_UNUSED(handle);
    PORT_SYSLOG_UNUSED(log_level);
}

void vprint_module_log(void *handle,
                       const char *func,
                       int line,
                       print_level_t level,
                       const char *message,
                       va_list list)
{
    PORT_SYSLOG_UNUSED(handle);
    PORT_SYSLOG_UNUSED(func);
    PORT_SYSLOG_UNUSED(line);
    PORT_SYSLOG_UNUSED(level);
    PORT_SYSLOG_UNUSED(message);
    PORT_SYSLOG_UNUSED(list);
}

void print_module_log(void *handle,
                      const char *func,
                      int line,
                      print_level_t level,
                      const char *message, ...)
{
    PORT_SYSLOG_UNUSED(handle);
    PORT_SYSLOG_UNUSED(func);
    PORT_SYSLOG_UNUSED(line);
    PORT_SYSLOG_UNUSED(level);
    PORT_SYSLOG_UNUSED(message);
}

int __wrap_printf( const char *format, ... )
{
    va_list ap;
    int32_t log_size;
    char frame_header[PORT_SYSLOG_MAX_ONE_PLAIN_LOG_SIZE];

    va_start(ap, format);
    log_size = vsnprintf(frame_header, sizeof(frame_header), format, ap);
    if (log_size < 0) {
        return -1;
    }
    if ((uint32_t)log_size >= (sizeof(frame_header))) {
        log_size = sizeof(frame_header) - 1;
    }
    va_end(ap);

    port_syslog_cross_cpu_enter_critical();
    port_syslog_device_printf_send(LOG_PORT_TYPE_UART, g_syslog_share_variable->port_index, (uint8_t *)frame_header, log_size);
    port_syslog_cross_cpu_exit_critical();

    return log_size;
}

void vdump_module_buffer(void *handle,
                         const char *func,
                         int line,
                         print_level_t level,
                         const void *data,
                         int length,
                         const char *message,
                         va_list list)
{
    PORT_SYSLOG_UNUSED(handle);
    PORT_SYSLOG_UNUSED(func);
    PORT_SYSLOG_UNUSED(line);
    PORT_SYSLOG_UNUSED(level);
    PORT_SYSLOG_UNUSED(data);
    PORT_SYSLOG_UNUSED(length);
    PORT_SYSLOG_UNUSED(message);
    PORT_SYSLOG_UNUSED(list);
}

void dump_module_buffer(void *handle,
                        const char *func,
                        int line,
                        print_level_t level,
                        const void *data,
                        int length,
                        const char *message, ...)
{
    PORT_SYSLOG_UNUSED(handle);
    PORT_SYSLOG_UNUSED(func);
    PORT_SYSLOG_UNUSED(line);
    PORT_SYSLOG_UNUSED(level);
    PORT_SYSLOG_UNUSED(data);
    PORT_SYSLOG_UNUSED(length);
    PORT_SYSLOG_UNUSED(message);
}

void log_print_msgid(void *handle,
                             print_level_t level,
                             const char *message,
                             uint32_t arg_cnt,
                             va_list list)
{
    PORT_SYSLOG_UNUSED(handle);
    PORT_SYSLOG_UNUSED(level);
    PORT_SYSLOG_UNUSED(message);
    PORT_SYSLOG_UNUSED(arg_cnt);
    PORT_SYSLOG_UNUSED(list);
}


void print_module_msgid_log(void *handle,
                            print_level_t level,
                            const char *message,
                            uint32_t arg_cnt, ...)
{
    PORT_SYSLOG_UNUSED(handle);
    PORT_SYSLOG_UNUSED(level);
    PORT_SYSLOG_UNUSED(message);
    PORT_SYSLOG_UNUSED(arg_cnt);
}

uint32_t dump_module_tlv_buffer(void *handle,
                                print_level_t level,
                                log_type_t type,
                                const void **p_data,
                                uint32_t *p_length)
{
    PORT_SYSLOG_UNUSED(handle);
    PORT_SYSLOG_UNUSED(level);
    PORT_SYSLOG_UNUSED(type);
    PORT_SYSLOG_UNUSED(p_data);
    PORT_SYSLOG_UNUSED(p_length);

    return 0;
}

#endif

