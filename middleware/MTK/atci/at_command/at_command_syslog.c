/* Copyright Statement:
 *
 * (C) 2005-2016  MediaTek Inc. All rights reserved.
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. ("MediaTek") and/or its licensors.
 * Without the prior written permission of MediaTek and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 * You may only use, reproduce, modify, or distribute (as applicable) MediaTek Software
 * if you have agreed to and been bound by the applicable license agreement with
 * MediaTek ("License Agreement") and been granted explicit permission to do so within
 * the License Agreement ("Permitted User").  If you are not a Permitted User,
 * please cease any access or use of MediaTek Software immediately.
 * BY OPENING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 * THAT MEDIATEK SOFTWARE RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES
 * ARE PROVIDED TO RECEIVER ON AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL
 * WARRANTIES, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR NONINFRINGEMENT.
 * NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH RESPECT TO THE
 * SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY, INCORPORATED IN, OR
 * SUPPLIED WITH MEDIATEK SOFTWARE, AND RECEIVER AGREES TO LOOK ONLY TO SUCH
 * THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO. RECEIVER EXPRESSLY ACKNOWLEDGES
 * THAT IT IS RECEIVER'S SOLE RESPONSIBILITY TO OBTAIN FROM ANY THIRD PARTY ALL PROPER LICENSES
 * CONTAINED IN MEDIATEK SOFTWARE. MEDIATEK SHALL ALSO NOT BE RESPONSIBLE FOR ANY MEDIATEK
 * SOFTWARE RELEASES MADE TO RECEIVER'S SPECIFICATION OR TO CONFORM TO A PARTICULAR
 * STANDARD OR OPEN FORUM. RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S ENTIRE AND
 * CUMULATIVE LIABILITY WITH RESPECT TO MEDIATEK SOFTWARE RELEASED HEREUNDER WILL BE,
 * AT MEDIATEK'S OPTION, TO REVISE OR REPLACE MEDIATEK SOFTWARE AT ISSUE,
 * OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE CHARGE PAID BY RECEIVER TO
 * MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 */

// For Register AT command handler
// System head file

#if !defined(MTK_DEBUG_LEVEL_PRINTF)

#include <string.h>
#include <stdio.h>
#include <stdbool.h>
#include "FreeRTOS.h"
#include "task.h"
#include "at_command.h"
#include "syslog.h"
#include "toi.h"

#ifndef MTK_SYSLOG_VERSION_2

/*

AT+SYSLOG=?
+SYSLOG:
AT+SYSLOG? Query for the current setting
AT+SYSLOG=<module>,<log_switch>,<print_level> Config module's setting
<log_switch>  := 0 (on) | 1 (off)
<print_level> := 0 (debug) | 1 (info) | 2 (warning) | 3 (error)
OK

AT+SYSLOG?
+SYSLOG: list of (<module>,<log_switch>,<print_level>)s
OK

AT+SYSLOG=<module>,<log_switch>,<print_level>
OK (or ERROR)

Defined values

<module>: string type; log module name

<log_switch>: integer type;
0 enable the logging for the module
1 disable the logging for the module

<print_level>: integer type; configured log level for the module
0 DEBUG
1 INFO
2 WARNING
3 ERROR

*/

static void syslog_show_usage(uint8_t *buf)
{
    int pos = 0;

    pos += snprintf((char *)(buf + pos),
                    ATCI_UART_TX_FIFO_BUFFER_SIZE - pos,
                    "%s",
                    "+SYSLOG:\r\n");
    pos += snprintf((char *)(buf + pos),
                    ATCI_UART_TX_FIFO_BUFFER_SIZE - pos,
                    "(AT+SYSLOG?, query for the current setting)\r\n");
    pos += snprintf((char *)(buf + pos),
                    ATCI_UART_TX_FIFO_BUFFER_SIZE - pos,
                    "(AT+SYSLOG=<module>,<log_switch>,<print_level>, config module's setting)\r\n");
    pos += snprintf((char *)(buf + pos),
                    ATCI_UART_TX_FIFO_BUFFER_SIZE - pos,
                    "(<log_switch> = 0|1(meaning on|off), <print_level=0|1|2|3(meaning D/I/W/E))\r\n");
}

static void syslog_show_log_config(uint8_t *buf)
{
    syslog_config_t   config;
    int               i = 0, pos = 0;

    pos += snprintf((char *)(buf + pos),
                    ATCI_UART_TX_FIFO_BUFFER_SIZE - pos,
                    "%s",
                    "+SYSLOG:");

    syslog_get_config(&config);
    while (config.filters && config.filters[i] != NULL) {
        pos += snprintf((char *)(buf + pos),
                        ATCI_UART_TX_FIFO_BUFFER_SIZE - pos,
                        "%c(\"%s\",%d,%d)",
                        (i == 0) ? ' ' : ',',
                        config.filters[i]->module_name,
                        config.filters[i]->log_switch,
                        config.filters[i]->print_level);
        i++;
    }
    snprintf((char *)(buf + pos),
             ATCI_UART_TX_FIFO_BUFFER_SIZE - pos,
             "\r\n");
}

static int parse_syslog_execute_command(char *cmd, char **pp1, int *pp2, int *pp3)
{
    uint8_t type;
    uint32_t value;
    char *pch, *p2, *p3;

    if (cmd[0] != '"') {
        return -1;
    }

    pch = cmd + 1;
    *pp1 = pch;
    pch = strchr(pch, '"');

    if (pch != NULL) {
        if ( *( pch + 1) != ',') {
            return -1;
        }
        *pch = 0;
        pch += 2;
        p2 = pch;
    } else {
        return -1;
    }

    pch = strchr(pch, ',');

    if (pch != NULL) {
        *pch = 0;
        pch++;
        p3 = pch;
    } else {
        return -1;
    }

    while (*pch && (*pch != '\n') && (*pch != '\r')) {
        pch++;
    }

    *pch = 0;

    value = toi(p2, &type);
    if (! (type == TOI_DEC && value <= 1) ) {
        return -2;
    }
    *pp2 = value;

    value = toi(p3, &type);
    if (! (type == TOI_DEC && value <= 3) ) {
        return -3;
    }
    *pp3 = value;

    return 0;
}

/* AT command handler  */
atci_status_t atci_cmd_hdlr_syslog(atci_parse_cmd_param_t *parse_cmd)
{
    atci_response_t response = {{0}, 0};
    char *p1;
    int  p2, p3, ret;

    response.response_flag = 0; /* Command Execute Finish. */
#ifdef ATCI_APB_PROXY_ADAPTER_ENABLE
    response.cmd_id = parse_cmd->cmd_id;
#endif

    switch (parse_cmd->mode) {
        case ATCI_CMD_MODE_TESTING: /* rec: AT+SYSLOG=? */
            syslog_show_usage(response.response_buf);
            response.response_len = strlen((char *)response.response_buf);
            response.response_flag |= ATCI_RESPONSE_FLAG_APPEND_OK;
            atci_send_response(&response);
            break;

        case ATCI_CMD_MODE_READ:    /* rec: AT+SYSLOG? */
            syslog_show_log_config(response.response_buf);
            response.response_len = strlen((char *)response.response_buf);
            response.response_flag |= ATCI_RESPONSE_FLAG_APPEND_OK;
            atci_send_response(&response);
            break;

        case ATCI_CMD_MODE_EXECUTION: /* rec: AT+SYSLOG=<module>,<log_switch>,<print_level> */
            ret = parse_syslog_execute_command(parse_cmd->string_ptr + 10, &p1, &p2, &p3);
            if (ret == 0) {
                if (syslog_at_set_filter(p1, p2, p3, 1) == 0) {
                    response.response_flag = ATCI_RESPONSE_FLAG_APPEND_OK;
                } else {
                    strcpy((char *)response.response_buf, "<module> error\r\n");
                    response.response_flag = ATCI_RESPONSE_FLAG_APPEND_ERROR;
                }
            } else {
                if (ret == -1) {
                    strcpy((char *)response.response_buf, "command syntax error\r\n");
                } else {
                    snprintf((char *)response.response_buf,
                             ATCI_UART_TX_FIFO_BUFFER_SIZE,
                             "<%s> error\r\n",
                             (ret == -2) ? "log_switch" : "print_level");
                }
                response.response_flag = ATCI_RESPONSE_FLAG_APPEND_ERROR;
            }
            response.response_len = strlen((char *)response.response_buf);
            atci_send_response(&response);
            break;

        default :
            /* others are invalid command format */
            response.response_flag = ATCI_RESPONSE_FLAG_APPEND_ERROR;
            response.response_len = strlen((char *)response.response_buf);
            atci_send_response(&response);
            break;
    }
    return ATCI_STATUS_OK;
}

#else

#if !defined(MTK_DEBUG_LEVEL_PRINTF)

static void syslog_show_usage(uint8_t *buf)
{
    int pos = 0;

    pos += snprintf((char *)(buf + pos),
                    ATCI_UART_TX_FIFO_BUFFER_SIZE - pos,
                    "%s",
                    "+SYSLOG:\r\n");
    pos += snprintf((char *)(buf + pos),
                    ATCI_UART_TX_FIFO_BUFFER_SIZE - pos,
                    "(AT+SYSLOG?, query for the current setting)\r\n");
    pos += snprintf((char *)(buf + pos),
                    ATCI_UART_TX_FIFO_BUFFER_SIZE - pos,
                    "(AT+SYSLOG=0,<cpu>,<log_switch>,<print_level>, config cpu's setting)\r\n");
    pos += snprintf((char *)(buf + pos),
                    ATCI_UART_TX_FIFO_BUFFER_SIZE - pos,
                    "(AT+SYSLOG=1,<cpu>,<module>,<log_switch>,<print_level>, config module's setting)\r\n");
    pos += snprintf((char *)(buf + pos),
                    ATCI_UART_TX_FIFO_BUFFER_SIZE - pos,
                    "    (<log_switch> = 0|1(meaning on|off), <print_level=0|1|2|3(meaning D/I/W/E))\r\n");
    pos += snprintf((char *)(buf + pos),
                    ATCI_UART_TX_FIFO_BUFFER_SIZE - pos,
                    "(AT+SYSLOG=2, save cpu/module's filter setting to NVDM)\r\n");
#ifdef MTK_SAVE_LOG_TO_FLASH_ENABLE
    pos += snprintf((char *)(buf + pos),
                    ATCI_UART_TX_FIFO_BUFFER_SIZE - pos,
                    "(AT+SYSLOG=3,<dump_switch>,<dump_level>, config log to flash setting)\r\n");
    pos += snprintf((char *)(buf + pos),
                    ATCI_UART_TX_FIFO_BUFFER_SIZE - pos,
                    "    (<dump_switch> = 0|1(meaning off|on), <dump_level> = 0|1(meaning passive|active)\r\n");
    pos += snprintf((char *)(buf + pos),
                    ATCI_UART_TX_FIFO_BUFFER_SIZE - pos,
                    "(AT+SYSLOG=4,<log save interval(ms)>,<assert interval(s)>, test for log save to flash)\r\n");
#endif
}

void log_show_filter_config(uint8_t *buf, uint32_t size)
{
    uint32_t i, j, pos;
    uint32_t cpu_number, module_number;
    const char *module_name;
    log_switch_t log_switch;
    print_level_t print_level;

    pos = 0;
    pos += snprintf((char *)(buf + pos),
                    size - pos,
                    "%s",
                    "+SYSLOG:\r\nCPU filter status\r\n");

    log_get_cpu_filter_number(&cpu_number);
    for (i = 0; i < cpu_number; i++) {
        log_get_cpu_filter_config(i, &log_switch, &print_level);
        pos += snprintf((char *)(buf + pos),
                        size - pos,
                        "%s[%u](%d,%d)",
                        (i == 0) ? " " : ", ",
                        (unsigned int)i,
                        log_switch,
                        print_level);
    }

    pos += snprintf((char *)(buf + pos),
                    size - pos,
                    "%s",
                    "\r\nModule filter status");

    for (i = 0; i < cpu_number; i++) {
        if (log_get_module_filter_number(i, &module_number) == true) {
            pos += snprintf((char *)(buf + pos),
                            size - pos,
                            "\r\n CPU [%u]\r\n",
                            (unsigned int)i);
            for (j = 0; j < module_number; j++) {
                log_get_module_filter_config(i, j, &module_name, &log_switch, &print_level);
                pos += snprintf((char *)(buf + pos),
                                size - pos,
                                "%s(%s,%d,%d)",
                                (j == 0) ? "  " : ", ",
                                module_name,
                                log_switch,
                                print_level);
            }
            snprintf((char *)(buf + pos),
                     size - pos,
                     "\r\n");
        }
    }
}

#ifdef MTK_SAVE_LOG_TO_FLASH_ENABLE

#include "hal_trng.h"
#include "FreeRTOS.h"
#include "task.h"
#include "task_def.h"
#include <stdlib.h>
#include "hal_gpt.h"
#include "exception_handler.h"

static volatile uint32_t g_assert_during_time, g_log_during_time;

static uint32_t random_get_value(uint32_t min, uint32_t max)
{
    uint32_t random_count;

#ifdef HAL_TRNG_MODULE_ENABLED
    while (hal_trng_init() != HAL_TRNG_STATUS_OK) {
        vTaskDelay(50);
    }

    do {
        hal_trng_get_generated_random_number(&random_count);
        random_count %= max;
    } while (random_count < min);

    hal_trng_deinit();
#else
    random_count = (max + min) / 2;
#endif

    return random_count;
}

static void save_log_test_task1(void *pvParameters)
{
    while (1) {
        LOG_MSGID_I(common, "hello world at tick %d",1, xTaskGetTickCount());
        vTaskDelay(random_get_value(50, 100));
    }
}

static void save_log_test_task2(void *pvParameters)
{
    uint32_t gpt_start_count, gpt_curr_count;

    hal_gpt_get_free_run_count(HAL_GPT_CLOCK_SOURCE_1M, &gpt_start_count);

    while (1) {
        vTaskDelay(random_get_value(g_log_during_time, g_log_during_time + 100));
        log_trigger_write_to_flash();
        hal_gpt_get_free_run_count(HAL_GPT_CLOCK_SOURCE_1M, &gpt_curr_count);
        if (((gpt_curr_count + random_get_value(5 * 1000000, 10 * 1000000)) - gpt_start_count) > (g_assert_during_time * 1000000)) {
            assert(0);
        }
    }
}

#endif

/* AT command handler  */
atci_status_t atci_cmd_hdlr_syslog(atci_parse_cmd_param_t *parse_cmd)
{
    bool ret;
    uint8_t type;
    atci_response_t response = {{0}, 0};
    char *p1, *p2, *p3, *p4, *module_name;
    uint32_t cpu_id, log_switch, print_level, dump_switch, always_dump;

    response.response_flag = 0; /* Command Execute Finish. */
#ifdef ATCI_APB_PROXY_ADAPTER_ENABLE
    response.cmd_id = parse_cmd->cmd_id;
#endif

    switch (parse_cmd->mode) {
        case ATCI_CMD_MODE_TESTING:
            syslog_show_usage(response.response_buf);
            response.response_len = strlen((char *)response.response_buf);
            response.response_flag |= ATCI_RESPONSE_FLAG_APPEND_OK;
            break;

        case ATCI_CMD_MODE_READ:
            log_show_filter_config(response.response_buf, ATCI_UART_TX_FIFO_BUFFER_SIZE);
            response.response_len = strlen((char *)response.response_buf);
            response.response_flag |= ATCI_RESPONSE_FLAG_APPEND_OK;
            break;

        case ATCI_CMD_MODE_EXECUTION:
            if (strstr(parse_cmd->string_ptr, "AT+SYSLOG=0,") != NULL) {
                p1 = parse_cmd->string_ptr + 12;
                p2 = strchr(p1, ',');
                p3 = strchr(&p2[1], ',');
                p1[1] = 0;
                cpu_id = toi(p1, &type);
                if (type != TOI_DEC) {
                    response.response_flag = ATCI_RESPONSE_FLAG_APPEND_ERROR;
                    break;
                }
                p2[2] = 0;
                log_switch = toi(&p2[1], &type);
                if (type != TOI_DEC) {
                    response.response_flag = ATCI_RESPONSE_FLAG_APPEND_ERROR;
                    break;
                }
                p3[2] = 0;
                print_level = toi(&p3[1], &type);
                if (type != TOI_DEC) {
                    response.response_flag = ATCI_RESPONSE_FLAG_APPEND_ERROR;
                    break;
                }
                ret = log_set_cpu_filter_config(cpu_id, log_switch, print_level);
                if (ret == false) {
                    response.response_flag = ATCI_RESPONSE_FLAG_APPEND_ERROR;
                } else {
                    response.response_flag = ATCI_RESPONSE_FLAG_APPEND_OK;
                }
            } else if (strstr(parse_cmd->string_ptr, "AT+SYSLOG=1,") != NULL) {
                p1 = parse_cmd->string_ptr + 12;
                p2 = strchr(p1, ',');
                p3 = strchr(&p2[1], ',');
                p4 = strchr(&p3[1], ',');
                p1[1] = 0;
                cpu_id = toi(p1, &type);
                if (type != TOI_DEC) {
                    response.response_flag = ATCI_RESPONSE_FLAG_APPEND_ERROR;
                    break;
                }
                module_name = &p2[1];
                p3[0] = 0;
                p3[2] = 0;
                log_switch = toi(&p3[1], &type);
                if (type != TOI_DEC) {
                    response.response_flag = ATCI_RESPONSE_FLAG_APPEND_ERROR;
                    break;
                }
                p4[2] = 0;
                print_level = toi(&p4[1], &type);
                if (type != TOI_DEC) {
                    response.response_flag = ATCI_RESPONSE_FLAG_APPEND_ERROR;
                    break;
                }
                ret = log_set_module_filter_config(cpu_id, module_name, log_switch, print_level);
                if (ret == false) {
                    response.response_flag = ATCI_RESPONSE_FLAG_APPEND_ERROR;
                } else {
                    response.response_flag = ATCI_RESPONSE_FLAG_APPEND_OK;
                }
            } else if (strstr(parse_cmd->string_ptr, "AT+SYSLOG=2") != NULL) {
                log_trigger_save_filter();
                do {
                    vTaskDelay(100);
                    ret = log_query_save_filter();
                } while (ret == false);
                response.response_flag = ATCI_RESPONSE_FLAG_APPEND_OK;
            } else if (strstr(parse_cmd->string_ptr, "AT+SYSLOG=3,") != NULL) {
#ifdef MTK_SAVE_LOG_TO_FLASH_ENABLE
                p1 = parse_cmd->string_ptr + 12;
                p1[1] = 0;
                dump_switch = toi(p1, &type);
                if (type != TOI_DEC) {
                    response.response_flag = ATCI_RESPONSE_FLAG_APPEND_ERROR;
                    break;
                }
                p2 = &p1[2];
                p2[1] = 0;
                always_dump = toi(p2, &type);
                if (type != TOI_DEC) {
                    response.response_flag = ATCI_RESPONSE_FLAG_APPEND_ERROR;
                    break;
                }
                ret = log_path_switch(dump_switch, true, always_dump);
                if (ret == false) {
                    response.response_flag = ATCI_RESPONSE_FLAG_APPEND_ERROR;
                } else {
                    response.response_flag = ATCI_RESPONSE_FLAG_APPEND_OK;
                }
#else
                dump_switch = dump_switch;
                always_dump = always_dump;
                response.response_flag = ATCI_RESPONSE_FLAG_APPEND_ERROR;
#endif
            } else if (strstr(parse_cmd->string_ptr, "AT+SYSLOG=4,") != NULL) {
#ifdef MTK_SAVE_LOG_TO_FLASH_ENABLE
                p1 = strtok_r(parse_cmd->string_ptr, ",", &p3);
                p1 = strtok_r(NULL, ",", &p3);
                if (p1 == NULL) {
                    response.response_flag = ATCI_RESPONSE_FLAG_APPEND_ERROR;
                    break;
                }
                p2 = strtok_r(NULL, ",", &p3);
                if (p2 == NULL) {
                    response.response_flag = ATCI_RESPONSE_FLAG_APPEND_ERROR;
                    break;
                }
                g_log_during_time = atoi(p1);
                g_assert_during_time = atoi(p2);
                exception_dump_config(DISABLE_WHILELOOP_MAGIC);
                xTaskCreate(save_log_test_task1, "dump_test1", 2048 / sizeof(StackType_t), NULL, TASK_PRIORITY_HIGH, NULL);
                xTaskCreate(save_log_test_task2, "dump_test2", 2048 / sizeof(StackType_t), NULL, TASK_PRIORITY_HIGH, NULL);
                response.response_len = 0;
                response.response_flag = ATCI_RESPONSE_FLAG_APPEND_OK;
#else
                response.response_flag = ATCI_RESPONSE_FLAG_APPEND_ERROR;
#endif
            } else {
                response.response_flag = ATCI_RESPONSE_FLAG_APPEND_ERROR;
            }
            response.response_len = strlen((char *)response.response_buf);
            break;

        default :
            /* others are invalid command format */
            response.response_flag = ATCI_RESPONSE_FLAG_APPEND_ERROR;
            response.response_len = strlen((char *)response.response_buf);
            break;
    }

    atci_send_response(&response);

    return ATCI_STATUS_OK;
}
#endif

#endif

#endif

