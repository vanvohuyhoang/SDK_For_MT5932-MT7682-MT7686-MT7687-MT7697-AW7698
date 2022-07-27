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

#include "offline_dump.h"

/* status of control block */
#define CONTROL_BLOCK_STATUS_ERASED         0xFF
#define CONTROL_BLOCK_STATUS_WRITTING       0xFE
#define CONTROL_BLOCK_STATUS_WRITE_FINISH   0xFC
#define CONTROL_BLOCK_STATUS_ERASING        0xF8

#define ARRAY_SIZE(a) (sizeof(a) / sizeof((a)[0]))

typedef struct {
    uint8_t type;
    char *magic_number;
    uint32_t start_address;
    uint32_t cell_size;
    uint32_t cell_count;
} offline_dump_info_t;

static volatile bool g_offline_dump_is_initialized = false;

static offline_dump_info_t g_offline_dump_info_array[] = {
    {
        OFFLINE_REGION_TYPE_SYSLOG,
        SYSLOG_MAGIC_NUMBER,
        0,
        OFFLINE_REGION_SYSLOG_CELL_SIZE,
        OFFLINE_REGION_SYSLOG_CELL_COUNT,
    },
    {
        OFFLINE_REGION_TYPE_MINIDUMP,
        MINI_DUMP_MAGIC_NUMBER,
        0,
        OFFLINE_REGION_MINI_DUMP_CELL_SIZE,
        OFFLINE_REGION_MINI_DUMP_CELL_COUNT,
    },
};

static volatile uint32_t g_offline_dump_current_sequence[ARRAY_SIZE(g_offline_dump_info_array)] = {0};
static volatile bool g_offline_dump_region_is_written[ARRAY_SIZE(g_offline_dump_info_array)] = {false};

static bool port_offline_dump_access_is_allowed(void)
{
#ifdef MTK_FOTA_ENABLE
    //PORT_FOTA_REGION_ACCESS();
#endif
    return true;
}

static uint16_t calculate_checksum(uint8_t *p_buf, uint32_t count)
{
    uint32_t sum;

    sum = 0;
    while (count > 1) {
        sum += *(uint16_t *)p_buf;
        count -= 2;
        p_buf += 2;
    }

    if (count > 0) {
        sum += *p_buf;
    }

    while (sum >> 16) {
        sum = (sum & 0xffff) + (sum >> 16);
    }

    return (uint16_t)(~sum);
}

static void check_offline_dump_region_integrity(void)
{
    uint8_t status;
    uint8_t *p_payload;
    uint32_t i, j;
    int32_t ongoing_cell;
    uint16_t header_size, checksum, calc_checksum;
    uint32_t total_block, curr_addr, curr_seq_number, max_seq_number;
    bool need_total_format, dump_is_empty;
    offline_dump_header_t *p_header;

    for (i = 0; i < ARRAY_SIZE(g_offline_dump_info_array); i++) {
        /* check dump status */
        ongoing_cell = -1;
        need_total_format = false;
        max_seq_number = 0;
        dump_is_empty = true;
        for (j = 0; j < g_offline_dump_info_array[i].cell_count; j++) {
            p_header = (offline_dump_header_t *)(g_offline_dump_info_array[i].start_address + j * g_offline_dump_info_array[i].cell_size);
            PORT_FLASH_READ((uint32_t)&p_header->status, &status, 1);
            switch (status) {
                case CONTROL_BLOCK_STATUS_ERASED:
                    continue;
                case CONTROL_BLOCK_STATUS_WRITTING:
                case CONTROL_BLOCK_STATUS_ERASING:
                    if (ongoing_cell >= 0) {
                        need_total_format = true;
                    } else {
                        ongoing_cell = j;
                        continue;
                    }
                    break;
                case CONTROL_BLOCK_STATUS_WRITE_FINISH:
                    PORT_FLASH_READ((uint32_t)&p_header->seq_number, (uint8_t *)&curr_seq_number, sizeof(p_header->seq_number));
                    if (curr_seq_number > max_seq_number) {
                        max_seq_number = curr_seq_number;
                    }
                    dump_is_empty = false;
                    break;
                default:
                    need_total_format = true;
            }

            if (need_total_format == true) {
                break;
            }

            /* check magic number */
            PORT_FLASH_READ((uint32_t)&p_header->header_checksum, (uint8_t *)&checksum, sizeof(p_header->header_checksum));
            PORT_FLASH_READ((uint32_t)&p_header->header_size, (uint8_t *)&header_size, sizeof(p_header->header_size));

            p_payload = (uint8_t *)malloc(header_size - 6);

            PORT_FLASH_READ((uint32_t)&p_header->header_size, p_payload, header_size - 6);
            calc_checksum = calculate_checksum(p_payload, header_size - 6);

            free(p_payload);

            if (checksum != calc_checksum) {
                need_total_format = true;
                break;
            }
        }

        if (need_total_format == true) {
            /* Format the region  */
            total_block = (g_offline_dump_info_array[i].cell_count * g_offline_dump_info_array[i].cell_size) / SERIAL_FLASH_BLOCK_SIZE;
            curr_addr = g_offline_dump_info_array[i].start_address;
            for (j = 0; j < total_block; j++) {
                PORT_FLASH_ERASE(curr_addr, PORT_FLASH_BLOCK_SIZE);
                curr_addr += SERIAL_FLASH_BLOCK_SIZE;
            }
            g_offline_dump_current_sequence[i] = 0;
        } else {
            if (ongoing_cell >= 0) {
                total_block = g_offline_dump_info_array[i].cell_size / SERIAL_FLASH_BLOCK_SIZE;
                curr_addr = g_offline_dump_info_array[i].start_address + ongoing_cell * g_offline_dump_info_array[i].cell_size;
                for (j = 0; j < total_block; j++) {
                    PORT_FLASH_ERASE(curr_addr, PORT_FLASH_BLOCK_SIZE);
                    curr_addr += SERIAL_FLASH_BLOCK_SIZE;
                }
            }
            if (dump_is_empty == false) {
                g_offline_dump_current_sequence[i] = max_seq_number + 1;
            }
        }
    }
}

extern char build_date_time_str[];
extern char sw_verno_str[];

static void initial_verno_build_time(offline_dump_header_t *p_header)
{
    uint8_t buffer[OFFLINE_BUILD_INFO_SIZE];
    uint32_t sdk_ver_cnt, build_time_cnt;

    sdk_ver_cnt = 1;
    PORT_FLASH_WRITE((uint32_t)&p_header->sdk_ver_cnt, (uint8_t *)&sdk_ver_cnt, sizeof(p_header->sdk_ver_cnt));
    build_time_cnt = OFFLINE_BUILD_TIME_COUNT;
    PORT_FLASH_WRITE((uint32_t)&p_header->build_time_cnt, (uint8_t *)&build_time_cnt, sizeof(p_header->build_time_cnt));
    memset(buffer, 0, sizeof(buffer));
    memcpy(buffer, sw_verno_str, (strlen(sw_verno_str) + 1));
    PORT_FLASH_WRITE((uint32_t)p_header->sdk_version, buffer, OFFLINE_BUILD_INFO_SIZE);
    memset(buffer, 0, sizeof(buffer));
    memcpy(buffer, build_date_time_str, (strlen(build_date_time_str) + 1));
    PORT_FLASH_WRITE((uint32_t)p_header->build_time, buffer, OFFLINE_BUILD_INFO_SIZE);
}

/* Call this API to do initialization of total block information. */
bool offline_dump_region_init(void)
{
    if (g_offline_dump_is_initialized == true) {
        return false;
    }

    /* Check whether FOTA allows us to write to FOTA */
    if (port_offline_dump_access_is_allowed() == false) {
        return false;
    }

    /* workaround for addr can NOT be static initialized */
    g_offline_dump_info_array[0].start_address = OFFLINE_REGION_SYSLOG_BASE_ADDR;
    g_offline_dump_info_array[1].start_address = OFFLINE_REGION_MINI_DUMP_BASE_ADDR;

    check_offline_dump_region_integrity();

    g_offline_dump_is_initialized = true;

    return true;
}

/* Call this API to alloc the next position to write to. */
bool offline_dump_region_alloc(offline_dump_region_type_t region_type, uint32_t *p_start_addr)
{
    uint8_t status;
    uint32_t i, curr_index, start_addr;
    offline_dump_header_t *p_header;

    if (g_offline_dump_is_initialized == false) {
        return false;
    }

    /* Check whether FOTA allows us to write to FOTA */
    if (port_offline_dump_access_is_allowed() == false) {
        return false;
    }

    if (region_type >= OFFLINE_REGION_MAX) {
        return false;
    }

    if (p_start_addr == NULL) {
        return false;
    }

    if (g_offline_dump_region_is_written[region_type] == true) {
        return false;
    }

    check_offline_dump_region_integrity();

    /* Calculate the position of dump cell */
    curr_index = g_offline_dump_current_sequence[region_type] % g_offline_dump_info_array[region_type].cell_count;
    start_addr = g_offline_dump_info_array[region_type].start_address + curr_index * g_offline_dump_info_array[region_type].cell_size;
    p_header = (offline_dump_header_t *)start_addr;

    *p_start_addr = start_addr + sizeof(offline_dump_header_t);

    /* mark current block cell as ERASING status */
    status = CONTROL_BLOCK_STATUS_ERASING;
    PORT_FLASH_WRITE((uint32_t)&p_header->status, &status, sizeof(p_header->status));

    /* Erase current block cell */
    for (i = 0; i < (g_offline_dump_info_array[region_type].cell_size / SERIAL_FLASH_BLOCK_SIZE); i++) {
        PORT_FLASH_ERASE((uint32_t)start_addr, HAL_FLASH_BLOCK_4K);
        start_addr += SERIAL_FLASH_BLOCK_SIZE;
    }

    /* mark current block cell as WRITTING status */
    status = CONTROL_BLOCK_STATUS_WRITTING;
    PORT_FLASH_WRITE((uint32_t)&p_header->status, &status, sizeof(p_header->status));

    g_offline_dump_region_is_written[region_type] = true;

    return true;
}

/* Call this API to write the dump data to flash, call be repeated many times */
bool offline_dump_region_write(offline_dump_region_type_t region_type, uint32_t curr_addr, uint8_t *data, uint32_t length)
{
    if (g_offline_dump_is_initialized == false) {
        return false;
    }

    /* Check whether FOTA allows us to write to FOTA */
    if (port_offline_dump_access_is_allowed() == false) {
        return false;
    }

    if (region_type >= OFFLINE_REGION_MAX) {
        return false;
    }

    if ((data == NULL) || (length == 0)) {
        return false;
    }

    if (g_offline_dump_region_is_written[region_type] == false) {
        return false;
    }

    /* write data to flash now */
    PORT_FLASH_WRITE(curr_addr, data, length);

    return true;
}

/* Call this API to end current dump. */
bool offline_dump_region_write_end(offline_dump_region_type_t region_type, uint32_t total_length)
{
    uint8_t buffer[32];
    uint8_t status, dump_version;
    uint16_t header_size, calc_checksum;
    uint32_t curr_index, start_addr, curr_seq_number, payload_size;
    offline_dump_header_t *p_header;

    if (g_offline_dump_is_initialized == false) {
        return false;
    }

    /* Check whether FOTA allows us to write to FOTA */
    if (port_offline_dump_access_is_allowed() == false) {
        return false;
    }

    if (region_type >= OFFLINE_REGION_MAX) {
        return false;
    }

    if (g_offline_dump_region_is_written[region_type] == false) {
        return false;
    }

    /* Calculate the position of dump cell */
    curr_index = g_offline_dump_current_sequence[region_type] % g_offline_dump_info_array[region_type].cell_count;
    start_addr = g_offline_dump_info_array[region_type].start_address + curr_index * g_offline_dump_info_array[region_type].cell_size;
    p_header = (offline_dump_header_t *)start_addr;

    /* Initialize the header */
    dump_version = OFFLINE_REGION_VERSION;
    PORT_FLASH_WRITE((uint32_t)&p_header->dump_version, &dump_version, sizeof(p_header->dump_version));
    PORT_FLASH_WRITE((uint32_t)&p_header->payload_type, &g_offline_dump_info_array[region_type].type, sizeof(p_header->payload_type));
    memset(buffer, 0, sizeof(buffer));
    strcpy((char *)buffer, g_offline_dump_info_array[region_type].magic_number);
    PORT_FLASH_WRITE((uint32_t)p_header->magic_number, buffer, sizeof(p_header->magic_number));
    payload_size = g_offline_dump_info_array[region_type].cell_size - sizeof(offline_dump_header_t);
    PORT_FLASH_WRITE((uint32_t)&p_header->payload_size, (uint8_t *)&payload_size, sizeof(p_header->payload_size));
    PORT_FLASH_WRITE((uint32_t)&p_header->payload_real_size, (uint8_t *)&total_length, sizeof(p_header->payload_real_size));
    curr_seq_number = g_offline_dump_current_sequence[region_type]++;
    PORT_FLASH_WRITE((uint32_t)&p_header->seq_number, (uint8_t *)&curr_seq_number, sizeof(p_header->seq_number));
#ifdef HAL_RTC_MODULE_ENABLED
    {
        hal_rtc_time_t time;
        hal_rtc_status_t status;
        uint8_t rtc_val[8];

        status = hal_rtc_get_time(&time);
        if (status == HAL_RTC_STATUS_OK) {
            rtc_val[0] = time.rtc_year;
            rtc_val[1] = time.rtc_mon;
            rtc_val[2] = time.rtc_day;
            rtc_val[3] = time.rtc_hour;
            rtc_val[4] = time.rtc_min;
            rtc_val[5] = time.rtc_sec;
            rtc_val[6] = 0;
            rtc_val[7] = 0;
            PORT_FLASH_WRITE((uint32_t)&p_header->rtc_time, rtc_val, sizeof(p_header->rtc_time));
        }
    }
#endif
    initial_verno_build_time(p_header);
    header_size = sizeof(offline_dump_header_t);
    PORT_FLASH_WRITE((uint32_t)&p_header->header_size, (uint8_t *)&header_size, sizeof(p_header->header_size));
    calc_checksum = calculate_checksum((uint8_t *)&p_header->header_size, header_size - 6);
    PORT_FLASH_WRITE((uint32_t)&p_header->header_checksum, (uint8_t *)&calc_checksum, sizeof(p_header->header_checksum));

    /* mark current block cell as WRITTING status */
    status = CONTROL_BLOCK_STATUS_WRITE_FINISH;
    PORT_FLASH_WRITE((uint32_t)&p_header->status, &status, sizeof(p_header->status));

    g_offline_dump_region_is_written[region_type] = false;

    return true;
}

bool offline_dump_region_read(offline_dump_region_type_t region_type, uint32_t curr_addr, uint8_t *buf, uint32_t length)
{
    if (g_offline_dump_is_initialized == false) {
        return false;
    }

    /* Check whether FOTA allows us to write to FOTA */
    if (port_offline_dump_access_is_allowed() == false) {
        return false;
    }

    if (region_type >= OFFLINE_REGION_MAX) {
        return false;
    }

    if (buf == NULL) {
        return false;
    }

    if (g_offline_dump_region_is_written[region_type] == true) {
        return false;
    }

    PORT_FLASH_READ(curr_addr, buf, length);

    return true;
}

bool offline_dump_region_query_seq_range(offline_dump_region_type_t region_type, uint32_t *p_min_seq, uint32_t *p_max_seq)
{
    uint32_t min_seq, max_seq;

    if (g_offline_dump_is_initialized == false) {
        return false;
    }

    if (region_type >= OFFLINE_REGION_MAX) {
        return false;
    }

    if ((p_min_seq == NULL) || (p_max_seq == NULL)) {
        return false;
    }

    if (g_offline_dump_region_is_written[region_type] == true) {
        return false;
    }

    if (g_offline_dump_current_sequence[region_type] == 0) {
        return false;
    }

    max_seq = g_offline_dump_current_sequence[region_type] - 1;
    if (g_offline_dump_current_sequence[region_type] >= g_offline_dump_info_array[region_type].cell_count) {
        min_seq = max_seq - (g_offline_dump_info_array[region_type].cell_count - 1);
    } else {
        min_seq = 0;
    }

    *p_min_seq = min_seq;
    *p_max_seq = max_seq;

    return true;
}

bool offline_dump_region_query_by_seq(offline_dump_region_type_t region_type, uint32_t seq, uint32_t *p_start_addr, uint32_t *p_total_length)
{
    uint32_t index;

    if (g_offline_dump_is_initialized == false) {
        return false;
    }

    if (region_type >= OFFLINE_REGION_MAX) {
        return false;
    }

    if ((p_start_addr == NULL) || (p_total_length == NULL)) {
        return false;
    }

    if (g_offline_dump_region_is_written[region_type] == true) {
        return false;
    }

    index = seq % g_offline_dump_info_array[region_type].cell_count;
    *p_start_addr = g_offline_dump_info_array[region_type].start_address + index * g_offline_dump_info_array[region_type].cell_size;
    *p_total_length = g_offline_dump_info_array[region_type].cell_size;

    return true;
}

