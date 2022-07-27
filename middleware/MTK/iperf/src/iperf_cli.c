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

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

#include <iperf_cli.h>
#include "iperf_task.h"
#include "connsys_driver.h"
#include "task_def.h"
#include "syslog.h"

#ifdef IPERF_MODULE_PRINTF
#define IPERF_LOGE(fmt,arg...)   printf(("\n[iperf]: "fmt), ##arg)
#define IPERF_LOGW(fmt,arg...)   printf(("\n[iperf]: "fmt), ##arg)
#define IPERF_LOGI(fmt,arg...)   printf(("\n[iperf]: "fmt), ##arg)
#else
#define IPERF_LOGE(fmt,arg...)   LOG_E(iperf, "[iperf]: "fmt,##arg)
#define IPERF_LOGW(fmt,arg...)   LOG_W(iperf, "[iperf]: "fmt,##arg)
#define IPERF_LOGI(fmt,arg...)   LOG_I(iperf, "[iperf]: "fmt,##arg)
#endif

// debug macro
#ifdef MTK_IPERF_DEBUG_ENABLE
#define DBGPRINT_IPERF(fmt, arg...)                    \
    do {                                               \
        if (g_iperf_debug_feature)                     \
        {                                              \
            LOG_I(iperf, "[iperf]: "fmt,##arg);        \
        }                                              \
    } while(0)
#else
#define DBGPRINT_IPERF(fmt, arg...)
#endif

static uint8_t _cli_iperf_server(uint8_t len, char *param[])
{
    int i;
    char **g_iperf_param = NULL;
    int is_create_task = 0;
    int offset = IPERF_COMMAND_BUFFER_SIZE / sizeof(char *);
    int udps_prio = 6;

    g_iperf_param = pvPortMalloc(IPERF_COMMAND_BUFFER_NUM * IPERF_COMMAND_BUFFER_SIZE);
    if (g_iperf_param == NULL) {
        IPERF_LOGI("Warning: No enough memory to running iperf.");
        return 0;
    }
    memset(g_iperf_param, 0, IPERF_COMMAND_BUFFER_NUM * IPERF_COMMAND_BUFFER_SIZE);

    for (i = 0; i < 13 && i < len; i++) {
        strcpy((char *)&g_iperf_param[i * offset], param[i]);
        DBGPRINT_IPERF("_cli_iperf_client, g_iperf_param[%d] is \"%s\"", i, (char *)&g_iperf_param[i * offset]);
        if (param[i][0] == 0 &&  param[i][1] == 0) {
            break;
        }
    }

    for (i = 0; i < 13 && i < len; i++) {
        if (strcmp(param[i], "-u") == 0) {
            IPERF_LOGI("Iperf UDP Server: Start!");
            IPERF_LOGI("Iperf UDP Server Receive Timeout = 20 (secs)");
            xTaskCreate((TaskFunction_t)iperf_udp_run_server, IPERF_TASK_NAME, IPERF_TASK_STACKSIZE / sizeof(portSTACK_TYPE), g_iperf_param, udps_prio , NULL);
            is_create_task = 1;
            break;
        }
    }

    if (0 == is_create_task) {
        IPERF_LOGI("Iperf TCP Server: Start!");
        IPERF_LOGI("Iperf TCP Server Receive Timeout = 20 (secs)");
        xTaskCreate((TaskFunction_t)iperf_tcp_run_server, IPERF_TASK_NAME, IPERF_TASK_STACKSIZE / sizeof(portSTACK_TYPE), g_iperf_param, IPERF_TASK_PRIO , NULL);
        is_create_task = 1;
    }

    if (is_create_task == 0) {
        vPortFree(g_iperf_param);
    }
    return 0;
}

static uint8_t _cli_iperf_client(uint8_t len, char *param[])
{
    int i;
    char **g_iperf_param = NULL;
    int is_create_task = 0;
    int offset = IPERF_COMMAND_BUFFER_SIZE / sizeof(char *);

    g_iperf_param = pvPortMalloc(IPERF_COMMAND_BUFFER_NUM * IPERF_COMMAND_BUFFER_SIZE);
    if (g_iperf_param == NULL) {
        IPERF_LOGI("Warning: No enough memory to running iperf.");
        return 0;
    }
    memset(g_iperf_param, 0, IPERF_COMMAND_BUFFER_NUM * IPERF_COMMAND_BUFFER_SIZE);

    for (i = 0; i < 18 && i < len; i++) {
        strcpy((char *)&g_iperf_param[i * offset], param[i]);
        DBGPRINT_IPERF("_cli_iperf_client, g_iperf_param[%d] is \"%s\"", i, (char *)&g_iperf_param[i * offset]);
        if (param[i][0] == 0 &&  param[i][1] == 0) {
            break;
        }
    }

    for (i = 0; i < 18 && i < len; i++) {
        if (strcmp(param[i], "-u") == 0) {
            IPERF_LOGI("Iperf UDP Client: Start!");
            xTaskCreate((TaskFunction_t)iperf_udp_run_client, IPERF_TASK_NAME, IPERF_TASK_STACKSIZE / sizeof(portSTACK_TYPE), g_iperf_param, IPERF_TASK_PRIO , NULL);
            is_create_task = 1;
            break;
        }
    }

    if (0 == is_create_task) {
        IPERF_LOGI("Iperf TCP Client: Start!");
#if (CFG_CONNSYS_TRX_BALANCE_EN == 1)
        xTaskCreate((TaskFunction_t)iperf_tcp_run_client, IPERF_TASK_NAME, IPERF_TASK_STACKSIZE / sizeof(portSTACK_TYPE), g_iperf_param, IPERF_TASK_PRIO , (TaskHandle_t *)&g_balance_ctr.tx_handle);
#else
        xTaskCreate((TaskFunction_t)iperf_tcp_run_client, IPERF_TASK_NAME, IPERF_TASK_STACKSIZE / sizeof(portSTACK_TYPE), g_iperf_param, IPERF_TASK_PRIO , NULL);
#endif
        is_create_task = 1;
    }

    if (is_create_task == 0) {
        vPortFree(g_iperf_param);
    }
    return 0;
}

static uint8_t _cli_iperf_help(uint8_t len, char *param[])
{
    IPERF_LOGI("Usage: iperf [-s|-c] [options]");
    IPERF_LOGI("       iperf [-h]\n");
    IPERF_LOGI("Client/Server:");
    IPERF_LOGI("  -u,        use UDP rather than TCP");
    IPERF_LOGI("  -p,    #    server port to listen on/connect to (default 5001)");
    IPERF_LOGI("  -m,    #   print TCP MSS (only for TCP)");
    IPERF_LOGI("  -M,    #   set TCP MSS [40, 1460] (only for TCP)");
    IPERF_LOGI("  -N,    nodelay, set TCP no delay, disabling Nagle's Algorithm");
    IPERF_LOGI("  -i,    #   set periodic times of bandwidth report [1, 10] \n");
    IPERF_LOGI("Server specific:");
    IPERF_LOGI("  -s,        run in server mode");
    IPERF_LOGI("  -B,    <ip>    bind to <ip>, and join to a multicast group (only for UDP & IPv4)\n");
    IPERF_LOGI("Client specific:");
    IPERF_LOGI("  -c,    <ip>    run in client mode, connecting to <ip>");
    IPERF_LOGI("  -d,    Do a bidirectional test simultaneously, only IPv4");
    IPERF_LOGI("  -r,    Do a bidirectional test individually (only Support UDP, not recommend)");
    IPERF_LOGI("  -n,    #[kmKM]    number of bytes to transmit, multiply 1000 or 1000*1000");
    IPERF_LOGI("  -b,    #[kmKM]    bandwidth to send at in bits/sec, multiply 1000 or 1000*1000 (only for UDP)");
    IPERF_LOGI("  -l,    #   TCP socket buffer size (default 1460, max 10*1024) or UDP datagram size (default/max 1460)");
    IPERF_LOGI("  -t,    #   time in seconds to transmit for (default 10 secs)");
    IPERF_LOGI("  -S,    #   the type-of-service of outgoing packets, -S 128 or -S 0x80");
    IPERF_LOGI("  -T,    #   time-to-live, only for UDP multicast [1, 255] (default 1)\n");
    IPERF_LOGI("Help or User Manual:");
    IPERF_LOGI("  -h,        print this message and quit\n");
    IPERF_LOGI("TOS example options for -S parameter:");
    IPERF_LOGI("BE: -S 0");
    IPERF_LOGI("BK: -S 32");
    IPERF_LOGI("VI: -S 160");
    IPERF_LOGI("VO: -S 224\n");
    IPERF_LOGI("Example:");
    IPERF_LOGI("Iperf TCP Server: iperf -s");
    IPERF_LOGI("Iperf UDP Server: iperf -s -u");
    IPERF_LOGI("Iperf TCP Client: iperf -c <ip> -t <duration>");
    IPERF_LOGI("Iperf UDP Client: iperf -c <ip> -u -t <duration>\n");
    IPERF_LOGI("Tradeoff Testing Mode -r:");
    IPERF_LOGI("Command: iperf -s -u,  PC Tool: iperf -c <ip> -u -r -n <send_num>");
    IPERF_LOGI("Command: iperf -c <ip> -u -r -t <send_time>,  PC Tool: iperf -s -u\n");
    IPERF_LOGI("-d Example:  Server CMD is same as normal example.");
    IPERF_LOGI("Iperf TCP Client: iperf -c <ip> -d -t <duration>");
    IPERF_LOGI("Iperf UDP Client: iperf -c <ip> -u -d -t <duration>\n");
    IPERF_LOGI("Multi-case Example:");
    IPERF_LOGI("Iperf UDP Server: iperf -s -u -B <multicast-addr>, PC Tool: iperf -c <multicast-addr> -u -t <duration>");
    IPERF_LOGI("Iperf UDP Client: iperf -c <multicast-addr> -B <multicast-addr> -u -t <duration>\n");
#ifdef MTK_IPERF_DEBUG_ENABLE
    IPERF_LOGI("Iperf Debug Switch: iperf -d 1 or 0\n");
#endif
    IPERF_LOGI("If you need PC iPerf Tool as the other end, please use iperf-2.0.9-win32.exe as far as possible, because Airoha Iperf takes it as reference!\n");
    return 0;
}

#ifdef MTK_IPERF_DEBUG_ENABLE
static uint8_t _cli_iperf_debug(uint8_t len, char *param[])
{
    int debug;
    debug = atoi(param[0]);
    IPERF_LOGI("Set iperf debug to %d(0x%x)\n", debug, debug);
    iperf_set_debug_mode((debug != 0 ? 1 : 0));
    return 0;
}
#endif

cmd_t iperf_cli[] = {
    { "-s",        "iperf server",               _cli_iperf_server   },
    { "-c",        "iperf client",               _cli_iperf_client   },
    { "-h",                "help",               _cli_iperf_help     },
#ifdef MTK_IPERF_DEBUG_ENABLE
    { "-d",               "debug",               _cli_iperf_debug    },
#endif
    { NULL }

};
