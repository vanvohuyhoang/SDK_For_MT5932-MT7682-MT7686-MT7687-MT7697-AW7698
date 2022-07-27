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

// Includes --------------------------------------------------------------------
#include "iperf_task.h"
#include "hal_gpt.h"
#include "ctype.h"
#include "syslog.h"

log_create_module(iperf, PRINT_LEVEL_INFO);

#ifdef IPERF_MODULE_PRINTF
#define IPERF_LOGE(fmt,arg...)   printf(("[iperf]: "fmt), ##arg)
#define IPERF_LOGW(fmt,arg...)   printf(("[iperf]: "fmt), ##arg)
#define IPERF_LOGI(fmt,arg...)   printf(("[iperf]: "fmt), ##arg)
#else
#define IPERF_LOGE(fmt,arg...)   LOG_E(iperf, "[iperf]: "fmt,##arg)
#define IPERF_LOGW(fmt,arg...)   LOG_W(iperf, "[iperf]: "fmt,##arg)
#define IPERF_LOGI(fmt,arg...)   LOG_I(iperf, "[iperf]: "fmt,##arg)
#endif

// debug macro
#ifdef MTK_IPERF_DEBUG_ENABLE
bool g_iperf_debug_feature = 0;
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

enum {
    kConv_Unit,
    kConv_Kilo,
    kConv_Mega,
    kConv_Giga
};

/* factor to multiply the number by */
const double kConversion[] = {
    1.0,                       /* unit */
    1.0 / 1024,                /* kilo */
    1.0 / 1024 / 1024,         /* mega */
    1.0 / 1024 / 1024 / 1024   /* giga */
};

/* factor to multiply the number by for bits*/
const double kConversionForBits[] = {
    1.0,                       /* unit */
    1.0 / 1000,                /* kilo */
    1.0 / 1000 / 1000,         /* mega */
    1.0 / 1000 / 1000 / 1000   /* giga */
};

/* labels for Byte formats [KMG] */
const char *kLabel_Byte[] = {
    "Byte",
    "KByte",
    "MByte",
    "GByte"
};

/* labels for bit formats [kmg] */
const char *kLabel_bit[]  = {
    "bit",
    "Kbit",
    "Mbit",
    "Gbit"
};

typedef struct _iperf_context {
    uint32_t server_addr;
    uint32_t port;
    uint32_t buffer_len;
    uint32_t win_band;
    uint32_t amount;
    iperf_result_t result;
    iperf_callback_t callback;
} iperf_context_t;

// For udp -r
int g_iperf_is_tradeoff_test_client = 0;
int g_iperf_is_tradeoff_test_server = 0;
// For udp client -d
int g_iperf_is_udp_client_dual_test = 0;

iperf_context_t g_iperf_context = {0};

// GPT tick count
static uint32_t start_count = 0;
static uint32_t end_count = 0;

// Private function prototypes -------------------------------------------------
static void iperf_calculate_result(int pkt_size, count_t *pkt_count);
static void iperf_display_result(char *report_title, unsigned ms_num, count_t *pkt_count);
static void iperf_reset_count(count_t *pkt_count);
static void iperf_copy_count(count_t *pkt_count_src, count_t *pkt_count_dest);
static void iperf_diff_count(count_t *result_count, count_t *pkt_count, count_t *tmp_count);
static char *iperf_ftoa(double f, char *buf, int precision);
static int byte_snprintf(char *outString, double inNum, char inFormat);
static uint32_t iperf_get_current_ms(void);
#ifdef UDP_SERVER_JITTER_REPORT_ENABLE
static uint32_t iperf_get_current_us(void);
#endif
static void iperf_pattern(char *outBuf, int inBytes);
static int iperf_get_tos(char *tos_str);
static void iperf_show_MSS(int sockfd);
static void iperf_set_MSS(int sockfd, int mss_val);
static void iperf_disable_nagle(int sockfd);
// Private functions -----------------------------------------------------------

void iperf_udp_run_server(char *parameters[])
{
    int sockfd = -1;
    struct sockaddr_in servaddr;
    struct sockaddr_in cliaddr;
#if LWIP_IPV6
    struct sockaddr_in6 servaddr6;
    struct sockaddr_in6 cliaddr6;
#endif
    int cli_len;
    int server_port;
    int i;
    count_t pkt_count;
    count_t tmp_count;
    int nbytes = 0; /* the number of read */
    int send_bytes = 0; /* the number of send */
#if LWIP_IGMP
    struct ip_mreq group;
    char *mcast = NULL;
    int mcast_tag = 0; /* the tag of parameter "-B"  */
#endif
    int interval_tag = 0, current_interval = 0; /* the tag of parameter "-i"  */
    char *buffer = NULL;
    uint32_t init_cost_ms = 0, current_ms = 0;
    UDP_datagram *udp_h;
    client_hdr *client_h;
    int32_t old_flags = 0;
    struct timeval timeout;
#ifdef MTK_TCPIP_FOR_NB_MODULE_ENABLE
    timeout.tv_sec = 120; //set recvive timeout = 120(sec)
#else
    timeout.tv_sec = 20; //set recvive timeout = 20(sec)
#endif
    timeout.tv_usec = 0;
    int udp_h_id = 0;
    int ipv6_flag = 0;
    int ret = 0;
    int dual_test_enable = 0; /* TCP Server dual_test - bidirectional traffic  */
    // UDP Server Report: jitter
#ifdef UDP_SERVER_JITTER_REPORT_ENABLE
    uint32_t pre_send_time_sec = 0, pre_send_time_usec = 0, pre_receive_time_us = 0;
    uint32_t total_jitter_us = 0;
#endif

    // Statistics init
    iperf_reset_count(&pkt_count);
    iperf_reset_count(&tmp_count);
    hal_gpt_get_free_run_count(HAL_GPT_CLOCK_SOURCE_32K, &start_count);
    server_port = 0;
    int offset = IPERF_COMMAND_BUFFER_SIZE / sizeof(char *);
    int data_size = IPERF_TEST_BUFFER_SIZE;

    // Handle input parameters
    if (g_iperf_is_tradeoff_test_client == 0) {
        for (i = 0; i < IPERF_COMMAND_BUFFER_NUM; i++) {
            if (strcmp((char *)&parameters[i * offset], "-p") == 0) {
                i++;
                server_port = atoi((char *)&parameters[i * offset]);
            } else if (strcmp((char *)&parameters[i * offset], "-B") == 0) {
                i++;
#if LWIP_IGMP
                mcast = (char *)&parameters[i * offset];
                mcast_tag = 1;
                IPERF_LOGI("Join Multicast %s \n", mcast);
#endif
            } else if (strcmp((char *)&parameters[i * offset], "-i") == 0) {
                i++;
                interval_tag = atoi((char *)&parameters[i * offset]);
                if (interval_tag < 1 || interval_tag > 10) {
                    interval_tag = 10;
                }
                current_interval = interval_tag;
                IPERF_LOGI("Set %d seconds as periodic report, range [1, 10]\n", interval_tag);
            } else if (strcmp((char *)&parameters[i * offset], "-l") == 0) {
                i++;
                data_size = atoi((char *)&parameters[i * offset]);
                IPERF_LOGI("Set buffer size = %d Bytes\n", data_size);
                if (data_size > IPERF_TEST_BUFFER_SIZE) {
                    data_size = IPERF_TEST_BUFFER_SIZE;
                    IPERF_LOGI("Upper limit of buffer size = %d Bytes\n", IPERF_TEST_BUFFER_SIZE);
                } else if (data_size < (sizeof(UDP_datagram) + sizeof(client_hdr))) {
                    data_size = sizeof(UDP_datagram) + sizeof(client_hdr);
                    IPERF_LOGI("Lower limit of buffer size = %d Bytes\n", data_size);
                }
            } else if (strcmp((char *)&parameters[i * offset], "-V") == 0) {
                ipv6_flag = 1;
                IPERF_LOGI("ipv6 mode\n");
            }
        }
    }

    // Create a new UDP socket
    if (ipv6_flag == 0) {
        sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    }
#if LWIP_IPV6
    else {
        sockfd = socket(AF_INET6, SOCK_DGRAM, 0);
    }
#endif
    // Create new UDP socket fail
    if (sockfd < 0) {
        IPERF_LOGE("[%s:%d] create sockfd fail = %d\n", __FUNCTION__, __LINE__, sockfd);
        if (parameters) {
            vPortFree(parameters);
        }
        if (g_iperf_context.callback) {
            g_iperf_context.callback(NULL);
        }
        vTaskDelete(NULL);
    }

    // set receive timeout
    if (setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO, (char *)&timeout, sizeof(timeout)) < 0) {
        IPERF_LOGE("Setsockopt failed - cancel receive timeout\n");
    }

    // set port and any IP address
    if (ipv6_flag == 0) {
        memset(&servaddr, 0, sizeof(servaddr));
        servaddr.sin_family = AF_INET;
        servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
        if (server_port == 0) {
            servaddr.sin_port = htons(IPERF_DEFAULT_PORT);
            IPERF_LOGI("Default server port = %d \n", IPERF_DEFAULT_PORT);
        } else {
            servaddr.sin_port = htons(server_port);
            IPERF_LOGI("Set server port = %d \n", server_port);
        }
    }
#if LWIP_IPV6
    else {
        ip6_addr_t ip6addr;
        memset(&servaddr6, 0, sizeof(servaddr6));
        servaddr6.sin6_family = AF_INET6;
        ip6_addr_set_any(&ip6addr);
        inet6_addr_from_ip6addr(&servaddr6.sin6_addr, &ip6addr);
        if (server_port == 0) {
            servaddr6.sin6_port = htons(IPERF_DEFAULT_PORT);
            IPERF_LOGI("Default server port = %d \n", IPERF_DEFAULT_PORT);
        } else {
            servaddr6.sin6_port = htons(server_port);
            IPERF_LOGI("Set server port = %d \n", server_port);
        }
    }
#endif
    // UDP Multicast settings
#if LWIP_IGMP
    if (ipv6_flag == 0 && mcast_tag == 1) {
        group.imr_multiaddr.s_addr = inet_addr(mcast);
        group.imr_interface.s_addr = htonl(INADDR_ANY);
        if (setsockopt(sockfd, IPPROTO_IP, IP_ADD_MEMBERSHIP, (char *)&group, sizeof(struct ip_mreq)) < 0) {
            IPERF_LOGI("Setsockopt failed - multicast settings\n");
        }
    }
#endif
    // Bind to port and any IP address
    if (ipv6_flag == 0) {
        ret = bind(sockfd, (struct sockaddr *)&servaddr, sizeof(servaddr));
    }
#if LWIP_IPV6
    else {
        ret = bind(sockfd, (struct sockaddr *)&servaddr6, sizeof(servaddr6));
    }
#endif
    // bind fail
    if (ret < 0) {
        IPERF_LOGE("[%s:%d] bind fail\n", __FUNCTION__, __LINE__);
        close(sockfd);
        if (parameters) {
            vPortFree(parameters);
        }
        if (g_iperf_context.callback) {
            g_iperf_context.callback(NULL);
        }
        vTaskDelete(NULL);
    }
    if (ipv6_flag == 0) {
        cli_len = sizeof(cliaddr);
    }
#if LWIP_IPV6
    else {
        cli_len = sizeof(cliaddr6);
    }
#endif
    // Malloc UDP data
    buffer = pvPortMalloc(IPERF_TEST_BUFFER_SIZE);
    if (buffer == NULL) {
        IPERF_LOGE("[%s:%d] pvPortMalloc fail\n", __FUNCTION__, __LINE__);
        close(sockfd);
        if (parameters) {
            vPortFree(parameters);
        }
        if (g_iperf_context.callback) {
            g_iperf_context.callback(NULL);
        }
        vTaskDelete(NULL);
    }
    memset(buffer, 0, IPERF_TEST_BUFFER_SIZE);

    // Wait and check the request from udp client, only once, see while(0)
    do {

        init_cost_ms = iperf_get_current_ms();
        do {
            if (ipv6_flag == 0) {
                nbytes = recvfrom(sockfd, buffer, data_size, MSG_TRUNC, (struct sockaddr *)&cliaddr, (socklen_t *)&cli_len);
                if(nbytes == -1)
                    IPERF_LOGI("Recv timeout, iperf client send no data.");
            }
#if LWIP_IPV6
            else {
                nbytes = recvfrom(sockfd, buffer, data_size, MSG_TRUNC, (struct sockaddr *)&cliaddr6, (socklen_t *)&cli_len);
                if(nbytes == -1)
                    IPERF_LOGI("Recv timeout, iperf client send no data.");
            }
#endif
            // delay some ms in UDP_Dual_Test to increase throughput
            if (g_iperf_is_udp_client_dual_test == 1) {
                vTaskDelay(2);
            }

            udp_h = (UDP_datagram *)buffer;
            udp_h_id = (int)ntohl(udp_h->id);

            // UDP Server Report: jitter
#ifdef UDP_SERVER_JITTER_REPORT_ENABLE
            if (nbytes > 0) {
                uint32_t udp_h_sec = (uint32_t)ntohl(udp_h->tv_sec);
                uint32_t udp_h_usec = (uint32_t)ntohl(udp_h->tv_usec);
                if (pre_send_time_sec == 0 && pre_send_time_usec == 0) {
                    pre_send_time_sec = udp_h_sec;
                    pre_send_time_usec = udp_h_usec;
                    pre_receive_time_us = iperf_get_current_us();
                } else {
                    // jitter = ((receive_time)j - (send_time)j) - ((receive_time)i - (send_time)i)
                    // = ((receive_time)j - (receive_time)i) - ((send_time)j - (send_time)i)
                    uint32_t cur_receive_time_us = iperf_get_current_us();
                    int32_t jitter_us = (cur_receive_time_us - pre_receive_time_us)
                        - ((udp_h_sec - pre_send_time_sec) * 1000 * 1000 + (udp_h_usec - pre_send_time_usec));
                    total_jitter_us += (jitter_us > 0 ? jitter_us : (-jitter_us));
                    pre_send_time_sec = udp_h_sec;
                    pre_send_time_usec = udp_h_usec;
                    pre_receive_time_us =  cur_receive_time_us;
                }
            }
#endif

            // UDP Server dual_test - bidirectional traffic
            client_h = (client_hdr *)&buffer[12];
            if (dual_test_enable == 0 && (IPERF_HEADER_DUAL_TEST == ntohl(client_h->flags))) {
                int port = ntohl(client_h->port);
                int time = ntohl(client_h->amount);
                time = -(time / 100);
                ip4_addr_t addr_t;
                addr_t.addr = cliaddr.sin_addr.s_addr;
                char *addr_str = ip4addr_ntoa(&addr_t);
                IPERF_LOGI("UDP Server dual_test addr=%s port=%d time=%d\n", addr_str, port, time);

                char **g_iperf_param = NULL;
                g_iperf_param = pvPortMalloc(IPERF_COMMAND_BUFFER_NUM * IPERF_COMMAND_BUFFER_SIZE);
                int offset = IPERF_COMMAND_BUFFER_SIZE / sizeof(char *);
                if (g_iperf_param == NULL) {
                    IPERF_LOGE("Warning: No enough memory to running iperf.");
                } else {
                    memset(g_iperf_param, 0, IPERF_COMMAND_BUFFER_NUM * IPERF_COMMAND_BUFFER_SIZE);
                    char port_str[10] = {0};
                    char time_str[10] = {0};
                    strcpy((char *)&g_iperf_param[0], addr_str);
                    strcpy((char *)&g_iperf_param[1 * offset], "-p");
                    sprintf(port_str, "%d", port);
                    strcpy((char *)&g_iperf_param[2 * offset], port_str);
                    strcpy((char *)&g_iperf_param[3 * offset], "-t");
                    sprintf(time_str, "%d", time);
                    strcpy((char *)&g_iperf_param[4 * offset], time_str);
                    g_iperf_is_udp_client_dual_test = 1;
                    xTaskCreate((TaskFunction_t)iperf_udp_run_client, IPERF_TASK_NAME, IPERF_TASK_STACKSIZE / sizeof(portSTACK_TYPE), g_iperf_param, IPERF_TASK_PRIO , NULL);
                    vTaskDelay(20);
                }
            }
            dual_test_enable = 1;

            iperf_calculate_result(nbytes, &pkt_count);

            // show temp result according to interval_tag
            if (interval_tag > 0) {
                current_ms = iperf_get_current_ms();
                int sec_num = (current_ms - init_cost_ms) / 1000;
                if (sec_num >= current_interval) {
                    count_t result_count;
                    IPERF_LOGI("Interval: %d - %d sec   ", sec_num - interval_tag, sec_num);
                    iperf_diff_count(&result_count, &pkt_count, &tmp_count);
                    iperf_display_result("UDP Server",  interval_tag * 1000, &result_count);
                    iperf_copy_count(&pkt_count, &tmp_count);
                    current_interval += interval_tag;
                }
            }

            DBGPRINT_IPERF("udp_h_id %d 0x%X || nbytes %d", udp_h_id, udp_h_id, nbytes);
            // Check the last END packet, Send Report(ACK) packet, close UDP server
            if (udp_h_id < 0 || nbytes <= 0) {
                current_ms = iperf_get_current_ms();
                // show Total Result (UDP Server)
                IPERF_LOGI("[Total]UDP Server receive_time: %d ms", current_ms - init_cost_ms);
                iperf_display_result("[Total]UDP Server", current_ms - init_cost_ms, &pkt_count);

                // Tradeoff mode, init data
                old_flags = ntohl(client_h->flags);
                if (IPERF_HEADER_VERSION1 == old_flags) {
                    memset(&g_iperf_context, 0, sizeof(iperf_context_t));
                    g_iperf_context.server_addr = cliaddr.sin_addr.s_addr;
                    g_iperf_context.port = ntohl(client_h->port);
                    g_iperf_context.buffer_len = ntohl(client_h->buffer_len);
                    g_iperf_context.win_band = ntohl(client_h->win_band);
                    g_iperf_context.amount = ntohl(client_h->amount);
                }

                // Need to send the correct report(ack) to client-side
                if (udp_h_id < 0) {
                    uint32_t datagrams = (-udp_h_id); // +udp_h_id + 1
                    int32_t error_cnt = datagrams -(pkt_count.times - 1); // minus last END packet
                    IPERF_LOGI("Receive End Package: 0x%X %d, send report to client\n", udp_h_id, nbytes);
                    server_hdr *report_hdr = (server_hdr *)&buffer[12];
                    report_hdr->flags = htonl(0x80000000);
                    report_hdr->total_len1 = htonl(0);
                    report_hdr->total_len2 = htonl(pkt_count.Bytes);
                    report_hdr->stop_sec = htonl((current_ms - init_cost_ms) / 1000);
                    report_hdr->stop_usec = htonl(((current_ms - init_cost_ms) % 1000) * 1000);
                    report_hdr->error_cnt = htonl(error_cnt);
                    report_hdr->outorder_cnt = htonl(0);
                    report_hdr->datagrams = htonl(datagrams);
#ifdef UDP_SERVER_JITTER_REPORT_ENABLE
                    uint32_t jitter_avg = total_jitter_us / pkt_count.times;
                    report_hdr->jitter1 = htonl(jitter_avg / 1000000);
                    report_hdr->jitter2 = htonl(jitter_avg % 1000000);
#else
                    report_hdr->jitter1 = htonl(0);
                    report_hdr->jitter2 = htonl(0);
#endif
                    if (ipv6_flag == 0) {
                        send_bytes = sendto(sockfd, buffer, IPERF_TEST_BUFFER_SIZE, 0, (struct sockaddr *)&cliaddr, cli_len);
                    }
#if LWIP_IPV6
                    else {
                        send_bytes = sendto(sockfd, buffer, IPERF_TEST_BUFFER_SIZE, 0, (struct sockaddr *)&cliaddr6, cli_len);
                    }
#endif
                    IPERF_LOGI("old_flags=%d, sendto report %d", old_flags, send_bytes);
                    vTaskDelay(500);
                }

                // Tradeoff mode, start UDP client-side after UDP server finished
                if (IPERF_HEADER_VERSION1 == old_flags) {
                    IPERF_LOGI("Tradeoff mode, start UDP client-side after UDP server finished.\n");
                    g_iperf_is_tradeoff_test_server = 1;
                    iperf_udp_run_client(NULL);
                    g_iperf_is_tradeoff_test_server = 0;
                }

                // UDP Client DUAL_Test -d
                IPERF_LOGI("UDP Server finished. Please check UDP server report in 76x7 console!\n");
                g_iperf_is_udp_client_dual_test = 0;
                break;
            }
        } while (nbytes > 0);

    } while (0);

    // clean & free
    if (buffer) {
        vPortFree(buffer);
    }
    if (parameters) {
        vPortFree(parameters);
    }
    IPERF_LOGI("UDP server close socket!\n");
    close(sockfd);

    IPERF_LOGI("If you want to execute iperf udp server again, please enter \"iperf -s -u\".\n");

    // g_iperf_context callback
    g_iperf_context.result.data_size = data_size;
    g_iperf_context.result.time = current_ms - init_cost_ms;
    if (g_iperf_context.callback) {
        g_iperf_context.callback(&g_iperf_context.result);
    }

    // For tradeoff mode, task will be deleted in iperf_udp_run_client
    if (g_iperf_is_tradeoff_test_client == 0) {
        vTaskDelete(NULL);
    }
}

void iperf_tcp_run_server(char *parameters[])
{
    int listenfd = -1;
    int connfd = -1;
    struct sockaddr_in servaddr, cliaddr;
#if LWIP_IPV6
    struct sockaddr_in6 servaddr6, cliaddr6;
#endif
    socklen_t clilen;
    int server_port;
    int i;
    count_t pkt_count;
    count_t tmp_count;
    int nbytes = 0; /* the number of read */
    int total_rcv = 0; /* the total number of receive  */
    int num_tag = 0; /* the tag of parameter "-n"  */
    int interval_tag = 0, current_interval = 0; /* the tag of parameter "-i"  */
    int print_mss_tag = 0; /* the tag of print mss "-m"  */
    int set_mss_num = 0; /* the tag of set mss "-M"  */
    int disable_nagle_tag = 0; /* the tag of disable nagle "-N"  */
    int ipv6_flag = 0;
    int dual_test_enable = 0; /* TCP Server dual_test - bidirectional traffic  */
    char *buffer = NULL;
    uint32_t init_cost_ms = 0, current_ms = 0;
    int offset = IPERF_COMMAND_BUFFER_SIZE / sizeof(char *);
    int ret = 0;

    // set receive timeout
    struct timeval timeout, timeout_recv;
#ifdef MTK_TCPIP_FOR_NB_MODULE_ENABLE
    timeout.tv_sec = 120; // set receive timeout = 120 (sec)
#else
    timeout.tv_sec = 20; // set receive timeout = 20 (sec)
#endif
    timeout.tv_usec = 0;
    timeout_recv.tv_sec = 5;
    timeout_recv.tv_usec = 0;

    // Statistics init
    iperf_reset_count(&pkt_count);
    iperf_reset_count(&tmp_count);
    hal_gpt_get_free_run_count(HAL_GPT_CLOCK_SOURCE_32K, &start_count);
    server_port = 0;

    // Handle input parameters
    for (i = 0; i < IPERF_COMMAND_BUFFER_NUM; i++) {
        if (strcmp((char *)&parameters[i * offset], "-p") == 0) {
            i++;
            server_port = atoi((char *)&parameters[i * offset]);
        } else if (strcmp((char *)&parameters[i * offset], "-n") == 0) {
            i++;
            total_rcv = iperf_format_transform((char *)&parameters[i * offset]);
            num_tag = 1;
            IPERF_LOGI("Set number to receive = %d Bytes\n", total_rcv);
        } else if (strcmp((char *)&parameters[i * offset], "-i") == 0) {
            i++;
            interval_tag = atoi((char *)&parameters[i * offset]);
            if (interval_tag < 1 || interval_tag > 10) {
                interval_tag = 10;
            }
            current_interval = interval_tag;
            IPERF_LOGI("Set %d seconds as periodic report, range [1, 10]\n", interval_tag);
        } else if (strcmp((char *)&parameters[i * offset], "-V") == 0) {
            ipv6_flag = 1;
            IPERF_LOGI("ipv6 mode\n");
        } else if (strcmp((char *)&parameters[i * offset], "-m") == 0) {
            print_mss_tag = 1;
        } else if (strcmp((char *)&parameters[i * offset], "-M") == 0) {
            i++;
            set_mss_num = atoi((char *)&parameters[i * offset]);
            IPERF_LOGI("set_mms_num=%d\n", set_mss_num);
            if (set_mss_num > IPERF_TEST_BUFFER_SIZE) {
                set_mss_num = IPERF_TEST_BUFFER_SIZE;
                IPERF_LOGI("Upper limit of mms_num=%d\n", set_mss_num);
            } else if (set_mss_num < 40) {
                set_mss_num = 40;
                IPERF_LOGI("Lower limit of mms_num=%d\n", set_mss_num);
            }
        } else if (strcmp((char *)&parameters[i * offset], "-N") == 0) {
            disable_nagle_tag = 1;
            IPERF_LOGI("TCP disable nagle\n");
        }
    }

    // Create a new TCP socket
    if (ipv6_flag == 0) {
        listenfd = socket(AF_INET, SOCK_STREAM, 0);
    }
#if LWIP_IPV6
    else {
        listenfd = socket(AF_INET6, SOCK_STREAM, 0);
    }
#endif
    // Create new TCP socket fail
    if (listenfd < 0) {
        IPERF_LOGE("[%s:%d] create listenfd fail = %d\n", __FUNCTION__, __LINE__, listenfd);
        if (parameters) {
            vPortFree(parameters);
        }
        if (g_iperf_context.callback) {
            g_iperf_context.callback(NULL);
        }
        vTaskDelete(NULL);
    }
    // set receive timeout
    if (setsockopt(listenfd, SOL_SOCKET, SO_RCVTIMEO, (char *)&timeout, sizeof(timeout)) < 0) {
        IPERF_LOGI("Setsockopt failed - cancel receive timeout\n");
    }

    do {
        // Bind to port and any IP address
        if (ipv6_flag == 0) {
            memset(&servaddr, 0, sizeof(servaddr));
            servaddr.sin_family = AF_INET;
            servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
            if (server_port == 0) {
                servaddr.sin_port = htons(IPERF_DEFAULT_PORT);
                IPERF_LOGI("Default server port = %d \n", IPERF_DEFAULT_PORT);
            } else {
                servaddr.sin_port = htons(server_port);
                IPERF_LOGI("Set server port = %d \n", server_port);
            }
            ret = bind(listenfd, (struct sockaddr *)&servaddr, sizeof(servaddr));
        }
#if LWIP_IPV6
        else {
            ip6_addr_t ip6addr;
            memset(&servaddr6, 0, sizeof(servaddr6));
            servaddr6.sin6_family = AF_INET6;
            ip6_addr_set_any(&ip6addr);
            inet6_addr_from_ip6addr(&servaddr6.sin6_addr, &ip6addr);
            if (server_port == 0) {
                servaddr6.sin6_port = htons(IPERF_DEFAULT_PORT);
                IPERF_LOGI("Default server port = %d \n", IPERF_DEFAULT_PORT);
            } else {
                servaddr6.sin6_port = htons(server_port);
                IPERF_LOGI("Set server port = %d \n", server_port);
            }
            ret = bind(listenfd, (struct sockaddr *)&servaddr6, sizeof(servaddr6));
        }
#endif
        // bind fail
        if (ret < 0) {
            IPERF_LOGE("[%s:%d] bind listenfd fail\n", __FUNCTION__, __LINE__);
            if (g_iperf_context.callback) {
                g_iperf_context.callback(NULL);
            }
            break;
        }

        // Put the connection into LISTEN state, only accept one client
        if ((listen(listenfd, 1)) < 0) {
            IPERF_LOGE("[%s:%d] listen fail\n", __FUNCTION__, __LINE__);
            if (g_iperf_context.callback) {
                g_iperf_context.callback(NULL);
            }
            break;
        }
        buffer = pvPortMalloc(IPERF_TEST_BUFFER_SIZE);
        if (buffer == NULL) {
            IPERF_LOGE("[%s:%d] pvPortMalloc fail\n", __FUNCTION__, __LINE__);
            if (g_iperf_context.callback) {
                g_iperf_context.callback(NULL);
            }
            break;
        }

        memset(buffer, 0, IPERF_TEST_BUFFER_SIZE);
        do {
            if (server_port != 0) {
                IPERF_LOGI("Listen...(port = %d)\n", server_port);
            } else {
                IPERF_LOGI("Listen...(port = %d)\n", IPERF_DEFAULT_PORT);
            }
            // Block and wait for an incoming connection
            if (ipv6_flag == 0) {
                clilen = sizeof(cliaddr);
                connfd = accept(listenfd, (struct sockaddr *)&cliaddr, &clilen);
            }
#if LWIP_IPV6
            else {
                clilen = sizeof(cliaddr6);
                connfd = accept(listenfd, (struct sockaddr *)&cliaddr6, &clilen);
            }
#endif
            if (connfd != -1) {
                IPERF_LOGI("[%s:%d] Accept... (sockfd=%d)\n", __FUNCTION__, __LINE__, connfd);
                // set receive timeout
                if (setsockopt(connfd, SOL_SOCKET, SO_RCVTIMEO, (char *)&timeout_recv, sizeof(timeout_recv)) < 0) {
                    IPERF_LOGI("Setsockopt failed - cancel receive timeout\n");
                }
                // disable nagle
                if (disable_nagle_tag == 1) {
                    iperf_disable_nagle(connfd);
                    disable_nagle_tag = 0;
                }
                // set TCP MMS
                if (set_mss_num > 0 && set_mss_num < 1460) {
                    iperf_set_MSS(connfd, set_mss_num);
                    set_mss_num = 0;
                }
                // print TCP MSS
                if (print_mss_tag == 1) {
                    iperf_show_MSS(connfd);
                    print_mss_tag = 0;
                }

                // connected, set first time = init_cost_ms
                init_cost_ms = iperf_get_current_ms();
                do {
                    nbytes = recv(connfd, buffer, IPERF_TEST_BUFFER_SIZE, 0);
                    if(nbytes == -1)
                        IPERF_LOGI("Recv timeout, iperf client send no data.");
                    // TCP Server dual_test - bidirectional traffic
                    client_hdr *client_h = (client_hdr *)buffer;
                    if (dual_test_enable == 0 && (IPERF_HEADER_DUAL_TEST == ntohl(client_h->flags))) {
                        int port = ntohl(client_h->port);
                        int time = ntohl(client_h->amount);
                        time = -(time / 100);
                        ip4_addr_t addr_t;
                        addr_t.addr = cliaddr.sin_addr.s_addr;
                        char *addr_str = ip4addr_ntoa(&addr_t);
                        IPERF_LOGI("TCP Server dual_test addr=%s port=%d time=%d\n", addr_str, port, time);

                        char **g_iperf_param = NULL;
                        g_iperf_param = pvPortMalloc(IPERF_COMMAND_BUFFER_NUM * IPERF_COMMAND_BUFFER_SIZE);
                        int offset = IPERF_COMMAND_BUFFER_SIZE / sizeof(char *);
                        if (g_iperf_param == NULL) {
                            IPERF_LOGE("Warning: No enough memory to running iperf.");
                        } else {
                            memset(g_iperf_param, 0, IPERF_COMMAND_BUFFER_NUM * IPERF_COMMAND_BUFFER_SIZE);
                            char port_str[10] = {0};
                            char time_str[10] = {0};
                            strcpy((char *)&g_iperf_param[0], addr_str);
                            strcpy((char *)&g_iperf_param[1 * offset], "-p");
                            sprintf(port_str, "%d", port);
                            strcpy((char *)&g_iperf_param[2 * offset], port_str);
                            strcpy((char *)&g_iperf_param[3 * offset], "-t");
                            sprintf(time_str, "%d", time);
                            strcpy((char *)&g_iperf_param[4 * offset], time_str);
                            xTaskCreate((TaskFunction_t)iperf_tcp_run_client, IPERF_TASK_NAME, IPERF_TASK_STACKSIZE / sizeof(portSTACK_TYPE), g_iperf_param, IPERF_TASK_PRIO , NULL);
                            vTaskDelay(20);
                        }
                    }
                    dual_test_enable = 1;

                    iperf_calculate_result(nbytes, &pkt_count);
                    if (num_tag == 1) {
                        total_rcv -= nbytes;
                    }
                    // Reach total receive number "-n"
                    if (total_rcv < 0) {
                        IPERF_LOGI("Finish Receiving \n");
                        break;
                    }

                    // show temp result according to interval_tag
                    if (interval_tag > 0) {
                        current_ms = iperf_get_current_ms();
                        int sec_num = (current_ms - init_cost_ms) / 1000;
                        if (sec_num >= current_interval) {
                            count_t result_count;
                            IPERF_LOGI("Interval: %d - %d sec   ", sec_num - interval_tag, sec_num);
                            iperf_diff_count(&result_count, &pkt_count, &tmp_count);
                            iperf_display_result("TCP Server",  interval_tag * 1000, &result_count);
                            iperf_copy_count(&pkt_count, &tmp_count);
                            current_interval += interval_tag;
                        }
                    }
                } while (nbytes > 0);

                current_ms = iperf_get_current_ms();
                // show Total Result (TCP Server)
                IPERF_LOGI("[Total]TCP Server receive_time: %d ms", current_ms - init_cost_ms);
                iperf_display_result("[Total]TCP Server", current_ms - init_cost_ms, &pkt_count);

                // g_iperf_context callback
                g_iperf_context.result.data_size = IPERF_TEST_BUFFER_SIZE;
                g_iperf_context.result.time = current_ms - init_cost_ms;
                if (g_iperf_context.callback) {
                    g_iperf_context.callback(&g_iperf_context.result);
                }

                iperf_reset_count(&pkt_count);
                iperf_reset_count(&tmp_count);

                IPERF_LOGI("TCP Server-connfd Close socket!\n");
                close(connfd);
                // sleep 200ms and close listenfd to exit accept, user need to re-start "iperf -s"
                vTaskDelay(200);
                // exit "accept" immediately, only accept one client
                connfd = -1;
            }
        } while (connfd != -1);

    } while (0);  // Loop just once, for listen state

    // clean & free
    close(listenfd);
    IPERF_LOGI("TCP Server-listenfd Close socket!\n");
    if (buffer) {
        vPortFree(buffer);
    }
    if (parameters) {
        vPortFree(parameters);
    }
    IPERF_LOGI("If you want to execute iperf tcp server again, please enter \"iperf -s\".\n");
    vTaskDelete(NULL);
}

void iperf_tcp_run_client(char *parameters[])
{
    int sockfd = -1;
    struct sockaddr_in servaddr;
#if LWIP_IPV6
    struct sockaddr_in6 servaddr6;
#endif
    char *Server_IP;
    count_t pkt_count;
    count_t tmp_count;
    int nbytes = 0; /* the number of send */
    int total_send = 0; /* the total number of transmit  */
    int num_tag = 0; /* the tag of parameter "-n"  */
    int interval_tag = 0, current_interval = 0; /* the tag of parameter "-i"  */
    int dual_tag = 0; /* the tag of dual test "-d"  */
    int print_mss_tag = 0; /* the tag of print mss "-m"  */
    int set_mss_num = 0; /* the tag of set mss "-M"  */
    int disable_nagle_tag = 0; /* the tag of disable nagle "-N"  */
    int ipv6_flag = 0;
    char *str = NULL;
    int i;
    int data_size, send_time, server_port, tos;
    uint32_t init_cost_ms = 0, current_ms = 0;
    int offset = IPERF_COMMAND_BUFFER_SIZE / sizeof(char *);
    int ret = 0;

    // Statistics init
    iperf_reset_count(&pkt_count);
    iperf_reset_count(&tmp_count);
    hal_gpt_get_free_run_count(HAL_GPT_CLOCK_SOURCE_32K, &start_count);
    data_size = 0;
    send_time = 0;
    server_port = 0;
    tos = 0;

    // Handle input parameters
    Server_IP = (char *)&parameters[0];

    for (i = 1; i < IPERF_COMMAND_BUFFER_NUM; i++) {
        if (strcmp((char *)&parameters[i * offset], "-l") == 0) {
            i++;
            data_size = atoi((char *)&parameters[i * offset]);
            IPERF_LOGI("Set socket buffer size = %d Bytes\n", data_size);
            if (data_size > 10 * 1024) {
                data_size = 10 * 1024;
                IPERF_LOGI("Adjust socket buffer size = 10 * 1024 Bytes\n");
            }
        } else if (strcmp((char *)&parameters[i * offset], "-t") == 0) {
            i++;
            send_time = atoi((char *)&parameters[i * offset]);
            IPERF_LOGI("Set send times = %d (secs)\n", atoi((char *)&parameters[i * offset]));
        } else if (strcmp((char *)&parameters[i * offset], "-p") == 0) {
            i++;
            server_port = atoi((char *)&parameters[i * offset]);
        } else if (strcmp((char *)&parameters[i * offset], "-n") == 0) {
            i++;
            total_send = iperf_format_transform((char *)&parameters[i * offset]);
            IPERF_LOGI("Set number to transmit = %d Bytes\n", total_send);
            total_send = (total_send / IPERF_TEST_BUFFER_SIZE) * IPERF_TEST_BUFFER_SIZE + IPERF_TEST_BUFFER_SIZE;
            num_tag = 1;
            IPERF_LOGI("Adjust number to transmit = %d Bytes\n", total_send);
        } else if (strcmp((char *)&parameters[i * offset], "-S") == 0) {
            i++;
            char *tos_str = (char *)&parameters[i * offset];
            tos = iperf_get_tos(tos_str);
            IPERF_LOGI("Set TOS = %s %d\n", tos_str, tos);
        } else if (strcmp((char *)&parameters[i * offset], "-i") == 0) {
            i++;
            interval_tag = atoi((char *)&parameters[i * offset]);
            if (interval_tag < 1 || interval_tag > 10) {
                interval_tag = 10;
            }
            current_interval = interval_tag;
            IPERF_LOGI("Set %d seconds as periodic report, range [1, 10]\n", interval_tag);
        } else if (strcmp((char *)&parameters[i * offset], "-V") == 0) {
            ipv6_flag = 1;
            IPERF_LOGI("ipv6 mode\n");
        } else if (strcmp((char *)&parameters[i * offset], "-d") == 0) {
            // TCP Client DUAL_Test -d
            dual_tag = 1;
            IPERF_LOGI("Set dual_tag: Do a bidirectional test simultaneously\n");
        } else if (strcmp((char *)&parameters[i * offset], "-m") == 0) {
            print_mss_tag = 1;
        } else if (strcmp((char *)&parameters[i * offset], "-M") == 0) {
            i++;
            set_mss_num = atoi((char *)&parameters[i * offset]);
            IPERF_LOGI("set_mms_num=%d\n", set_mss_num);
            if (set_mss_num > IPERF_TEST_BUFFER_SIZE) {
                set_mss_num = IPERF_TEST_BUFFER_SIZE;
                IPERF_LOGI("Upper limit of mms_num=%d\n", set_mss_num);
            } else if (set_mss_num < 40) {
                set_mss_num = 40;
                IPERF_LOGI("Lower limit of mms_num=%d\n", set_mss_num);
            }
        } else if (strcmp((char *)&parameters[i * offset], "-N") == 0) {
            disable_nagle_tag = 1;
            IPERF_LOGI("TCP disable nagle\n");
        }
    }
    // set data_size
    if (data_size == 0) {
        data_size = 1460;
        IPERF_LOGI("Default buffer size = %d Bytes\n", data_size);
    }
    // set send_time
    if (send_time == 0) {
        if (num_tag == 1) {
            send_time = 999999;
        } else {
            send_time = 10;
            IPERF_LOGI("Default send times = %d (secs)\n", send_time);
        }
    }
    // Create a new TCP socket
    if (ipv6_flag == 0) {
        sockfd = socket(AF_INET, SOCK_STREAM, 0);
    }
#if LWIP_IPV6
    else {
        sockfd = socket(AF_INET6, SOCK_STREAM, 0);
    }
#endif
    // Create new TCP socket fail
    if (sockfd < 0) {
        IPERF_LOGI("[%s:%d] sockfd = %d\n", __FUNCTION__, __LINE__, sockfd);
        if (parameters) {
            vPortFree(parameters);
        }
        if (g_iperf_context.callback) {
            g_iperf_context.callback(NULL);
        }
        vTaskDelete(NULL);
    }
    if (ipv6_flag == 0) {
        if (setsockopt(sockfd, IPPROTO_IP, IP_TOS, &tos, sizeof(tos)) < 0) {
            IPERF_LOGI("Set TOS: fail!\n");
        }
    }
    // set server addr & port
    if (ipv6_flag == 0) {
        memset(&servaddr, 0, sizeof(servaddr));
        servaddr.sin_family = AF_INET;
        servaddr.sin_addr.s_addr = inet_addr(Server_IP);
        if (server_port == 0) {
            server_port = IPERF_DEFAULT_PORT;
            servaddr.sin_port = htons(IPERF_DEFAULT_PORT);
            IPERF_LOGI("Default server port = %d \n", IPERF_DEFAULT_PORT);
        } else {
            servaddr.sin_port = htons(server_port);
            IPERF_LOGI("Set server port = %d \n", server_port);
        }
    }
#if LWIP_IPV6
    else {
        ip6_addr_t ip6addr;
        memset(&servaddr6, 0, sizeof(servaddr6));
        servaddr6.sin6_family = AF_INET6;
        ip6addr_aton(Server_IP, &ip6addr);
        inet6_addr_from_ip6addr(&servaddr6.sin6_addr, &ip6addr);
        if (server_port == 0) {
            server_port = IPERF_DEFAULT_PORT;
            servaddr6.sin6_port = htons(IPERF_DEFAULT_PORT);
            IPERF_LOGI("Default server port = %d \n", IPERF_DEFAULT_PORT);
        } else {
            servaddr6.sin6_port = htons(server_port);
            IPERF_LOGI("Set server port = %d \n", server_port);
        }
    }
#endif
    // tcp connect
    if (ipv6_flag == 0) {
        ret = connect(sockfd, (struct sockaddr *)&servaddr, sizeof(servaddr));
    }
#if LWIP_IPV6
    else {
        ret = connect(sockfd, (struct sockaddr *)&servaddr6, sizeof(servaddr6));
    }
#endif
    // tcp connect fail
    if (ret < 0) {
        IPERF_LOGI("Connect failed, sockfd is %d, addr is \"%s\"\n", (int)sockfd, Server_IP);
        close(sockfd);
        if (parameters) {
            vPortFree(parameters);
        }
        if (g_iperf_context.callback) {
            g_iperf_context.callback(NULL);
        }
        vTaskDelete(NULL);
    }

    // disable nagle
    if (disable_nagle_tag == 1) {
        iperf_disable_nagle(sockfd);
        disable_nagle_tag = 0;
    }
    // set TCP MMS
    if (set_mss_num > 0 && set_mss_num < 1460) {
        iperf_set_MSS(sockfd, set_mss_num);
        set_mss_num = 0;
    }
    // print TCP MSS
    if (print_mss_tag == 1) {
        iperf_show_MSS(sockfd);
        print_mss_tag = 0;
    }

    // Malloc TCP data
    str = pvPortCalloc(1, IPERF_TEST_BUFFER_SIZE);
    if (str == NULL) {
        IPERF_LOGI("not enough buffer to send data!\n");
        close(sockfd);
        if (parameters) {
            vPortFree(parameters);
        }
        if (g_iperf_context.callback) {
            g_iperf_context.callback(NULL);
        }
        vTaskDelete(NULL);
    }

    // TCP Client -d dual_test
    if (dual_tag == 1) {
        char **g_iperf_param = NULL;
        g_iperf_param = pvPortMalloc(IPERF_COMMAND_BUFFER_NUM * IPERF_COMMAND_BUFFER_SIZE);
        if (g_iperf_param == NULL) {
            IPERF_LOGE("Warning: No enough memory to running iperf.");
        } else {
            memset(g_iperf_param, 0, IPERF_COMMAND_BUFFER_NUM * IPERF_COMMAND_BUFFER_SIZE);
            xTaskCreate((TaskFunction_t)iperf_tcp_run_server, IPERF_TASK_NAME, IPERF_TASK_STACKSIZE / sizeof(portSTACK_TYPE), g_iperf_param, IPERF_TASK_PRIO , NULL);
            vTaskDelay(100);
            // send dual_start cmd to notfy iperf_tcp_server to create tcp_client and connect us.
            memset(str, 0, IPERF_TEST_BUFFER_SIZE);
            iperf_pattern(str, IPERF_TEST_BUFFER_SIZE);
            client_hdr *client_h = (client_hdr *)&str[0];
            client_h->flags = htonl(IPERF_HEADER_DUAL_TEST);
            client_h->num_threads = htonl(1);
            client_h->port = htonl(server_port);
            client_h->buffer_len = 0;
            client_h->win_band = 0;
            // adjust client send_time (sec * 1000ms / 10ms -> negative -> htonl), no support send_num mode
            int count10ms = (send_time * 1000 / 10);
            client_h->amount = htonl(-count10ms);
            nbytes = send(sockfd, str, 36, 0);
            vTaskDelay(100);
        }
    }

    memset(str, 0, IPERF_TEST_BUFFER_SIZE);
    iperf_pattern(str, IPERF_TEST_BUFFER_SIZE);

    init_cost_ms = iperf_get_current_ms();
    do {
        nbytes = send(sockfd, str, data_size, 0);
        iperf_calculate_result(nbytes, &pkt_count);

        if (num_tag == 1) {
            total_send -= nbytes;
        }
        // Reach total receive number "-n"
        if (total_send < 0) {
            IPERF_LOGI("Finish Sending \n");
            break;
        }

        // show temp result according to interval_tag
        if (interval_tag > 0) {
            current_ms = iperf_get_current_ms();
            int sec_num = (current_ms - init_cost_ms) / 1000;
            if (sec_num >= current_interval) {
                count_t result_count;
                IPERF_LOGI("Interval: %d - %d sec   ", sec_num - interval_tag, sec_num);
                iperf_diff_count(&result_count, &pkt_count, &tmp_count);
                iperf_display_result("TCP Client",  interval_tag * 1000, &result_count);
                iperf_copy_count(&pkt_count, &tmp_count);
                current_interval += interval_tag;
            }
        }

        current_ms = iperf_get_current_ms();
    } while ((current_ms - init_cost_ms) < send_time * 1000);

    // clean & free
    if (str) {
        vPortFree(str);
    }
    close(sockfd);
    if (parameters) {
        vPortFree(parameters);
    }
    IPERF_LOGI("TCP Client Close socket!\n");
    // show Total Result (TCP Client)
    iperf_display_result("[Total]TCP Client", current_ms - init_cost_ms, &pkt_count);
    // TCP Client DUAL_Test -d
    if (dual_tag == 1) {
        IPERF_LOGI("Please re-start iperf -s in PC Console for dual_test!\n");
    }

    // g_iperf_context callback
    g_iperf_context.result.data_size = data_size;
    g_iperf_context.result.time = current_ms - init_cost_ms;
    if (g_iperf_context.callback) {
        g_iperf_context.callback(&g_iperf_context.result);
    }

    vTaskDelete(NULL);
}

void iperf_udp_run_client(char *parameters[])
{
    int sockfd = -1;
    struct sockaddr_in servaddr;
#if LWIP_IPV6
    struct sockaddr_in6 servaddr6;
#endif
#if LWIP_IGMP
    struct ip_mreq group;
    char *mcast = NULL;
    int mcast_tag = 0; /* the tag of parameter "-B"  */
    uint8_t mcast_ttl = 1;
#endif
    char *Server_IP = 0;
    count_t pkt_count;
    count_t tmp_count;
    int nbytes = 0; /* the number of send */
    int total_send = 0; /* the total number of transmit  */
    int num_tag = 0; /* the tag of parameter "-n"  */
    int interval_tag = 0, current_interval = 0; /* the tag of parameter "-i"  */
    int tradeoff_tag = 0; /* the tag of parameter "-r"  */
    int dual_tag = 0; /* the tag of dual test "-d"  */
    int ipv6_flag = 0;
    char *str = NULL;
    int i = 0;
    int data_size = 0, send_time = 0, server_port = 0, tos = 0, bw = 0;
    uint32_t send_one_pkt_us = 0;
    uint32_t init_cost_ms = 0, current_ms = 0;
    UDP_datagram *udp_h;
    client_hdr *client_h;
    int udp_h_id = 0;
    int offset = IPERF_COMMAND_BUFFER_SIZE / sizeof(char *);
    int ret = 0;

    // Statistics init
    iperf_reset_count(&pkt_count);
    iperf_reset_count(&tmp_count);
    hal_gpt_get_free_run_count(HAL_GPT_CLOCK_SOURCE_32K, &start_count);

    // Handle input parameters
    if (g_iperf_is_tradeoff_test_server == 0) {
        Server_IP = (char *)&parameters[0];
        for (i = 1; i < IPERF_COMMAND_BUFFER_NUM; i++) {
            if (strcmp((char *)&parameters[i * offset], "-l") == 0) {
                i++;
                data_size = atoi((char *)&parameters[i * offset]);
                IPERF_LOGI("Set datagram size = %d Bytes\n", data_size);
                if (data_size > IPERF_TEST_BUFFER_SIZE) {
                    data_size = IPERF_TEST_BUFFER_SIZE;
                    IPERF_LOGI("Upper limit of buffer size = %d Bytes\n", IPERF_TEST_BUFFER_SIZE);
                } else if (data_size < (sizeof(UDP_datagram) + sizeof(client_hdr))) {
                    data_size = sizeof(UDP_datagram) + sizeof(client_hdr);
                    IPERF_LOGI("Lower limit of buffer size = %d Bytes\n", data_size);
                }
            } else if (strcmp((char *)&parameters[i * offset], "-t") == 0) {
                i++;
                send_time = atoi((char *)&parameters[i * offset]);
                IPERF_LOGI("Set send times = %d (secs)\n", atoi((char *)&parameters[i * offset]));
            } else if (strcmp((char *)&parameters[i * offset], "-p") == 0) {
                i++;
                server_port = atoi((char *)&parameters[i * offset]);
            } else if (strcmp((char *)&parameters[i * offset], "-n") == 0) {
                i++;
                total_send = iperf_format_transform((char *)&parameters[i * offset]);
                IPERF_LOGI("Set number to transmit = %d Bytes\n", total_send);
                total_send = (total_send / IPERF_TEST_BUFFER_SIZE) * IPERF_TEST_BUFFER_SIZE + IPERF_TEST_BUFFER_SIZE;
                num_tag = 1;
                IPERF_LOGI("Adjust number to transmit = %d Bytes\n", total_send);
            } else if (strcmp((char *)&parameters[i * offset], "-S") == 0) {
                i++;
                char *tos_str = (char *)&parameters[i * offset];
                tos = iperf_get_tos(tos_str);
                IPERF_LOGI("Set TOS = %s %d\n", tos_str, tos);
            } else if (strcmp((char *)&parameters[i * offset], "-b") == 0) {
                i++;
                IPERF_LOGI("Set bandwidth = %s\n", (char *)&parameters[i * offset]);
                bw = iperf_format_transform((char *)&parameters[i * offset]) / 8;
                // 2500000 = 20 * 1000 * 1000 / 8 Bytes
                if (bw > 2500000 || bw <= 0) {
                    bw = 2500000;
                    IPERF_LOGI("Upper limit of bandwith setting = 20Mbits/sec\n");
                } else if (bw < IPERF_TEST_BUFFER_SIZE) {
                    bw = IPERF_TEST_BUFFER_SIZE;
                    IPERF_LOGI("Lower limit of bandwith setting = IPERF_TEST_BUFFER_SIZE\n");
                }
                IPERF_LOGI("bandwidth = %d\n", bw);
            } else if (strcmp((char *)&parameters[i * offset], "-i") == 0) {
                i++;
                interval_tag = atoi((char *)&parameters[i * offset]);
                if (interval_tag < 1 || interval_tag > 10) {
                    interval_tag = 10;
                }
                current_interval = interval_tag;
                IPERF_LOGI("Set %d seconds as periodic report, range [1, 10]\n", interval_tag);
            } else if (strcmp((char *)&parameters[i * offset], "-r") == 0) {
                tradeoff_tag = 1;
                IPERF_LOGI("Set to tradeoff mode\n");
            } else if (strcmp((char *)&parameters[i * offset], "-V") == 0) {
                ipv6_flag = 1;
                IPERF_LOGI("ipv6 mode\n");
            } else if (strcmp((char *)&parameters[i * offset], "-d") == 0) {
                // UDP Client DUAL_Test -d
                dual_tag = 1;
                g_iperf_is_udp_client_dual_test = 1;
                IPERF_LOGI("Set dual_tag: Do a bidirectional test simultaneously\n");
            } else if (strcmp((char *)&parameters[i * offset], "-B") == 0) {
                i++;
#if LWIP_IGMP
                mcast = (char *)&parameters[i * offset];
                mcast_tag = 1;
                IPERF_LOGI("Join Multicast %s \n", mcast);
#endif
            } else if (strcmp((char *)&parameters[i * offset], "-T") == 0) {
                i++;
#if LWIP_IGMP
                mcast_ttl = atoi((char *)&parameters[i * offset]);
                IPERF_LOGI("Multicast TLL %d\n", mcast_ttl);
#endif
            }
        }
    }

    // UDP Client DUAL_Test -d
    if (tradeoff_tag == 1 && dual_tag == 1) {
        tradeoff_tag = 0;
        IPERF_LOGI("Set dual_tag and tradeoff: clear tradeoff mode\n");
    }

    if (ipv6_flag == 0) {
        memset(&servaddr, 0, sizeof(servaddr));
        servaddr.sin_family = AF_INET;
    }
#if LWIP_IPV6
    else {
        memset(&servaddr6, 0, sizeof(servaddr6));
        servaddr6.sin6_family = AF_INET6;
    }
#endif
    // set Server IP
    if (g_iperf_is_tradeoff_test_server == 0) {
        if (ipv6_flag == 0) {
            servaddr.sin_addr.s_addr = inet_addr(Server_IP);
        }
#if LWIP_IPV6
        else {
            ip6_addr_t ip6addr;
            ip6addr_aton(Server_IP, &ip6addr);
            inet6_addr_from_ip6addr(&servaddr6.sin6_addr, &ip6addr);
        }
#endif
    } else {
        // set parameter by using udp_server trade-off packet
        if (ipv6_flag == 0) {
            servaddr.sin_addr.s_addr = g_iperf_context.server_addr;
        }
#if LWIP_IPV6
        else {
            ip6_addr_t ip6addr;
            ip6addr_aton(Server_IP, &ip6addr);
            inet6_addr_from_ip6addr(&servaddr6.sin6_addr, &ip6addr);
        }
#endif
        // set data when tradeoff_test_server
        server_port = g_iperf_context.port;
        bw = g_iperf_context.win_band / 8;
        total_send = g_iperf_context.amount;
        if (total_send > 0) {
            num_tag = 1;
        } else {
            // iperf limitation -r: only support "num_tag", cannot "send_time"
            num_tag = 1;
            total_send = 1500 * IPERF_TEST_BUFFER_SIZE;
            IPERF_LOGE("Warning:  we only support \"total_send\"\n");
        }
        IPERF_LOGI("Tradeoff-Server Start Client, %d %d 0x%X %d %d %d\n", server_port, bw,
                   g_iperf_context.server_addr, total_send, num_tag, send_time);
    }
    IPERF_LOGI("Server address = \"%s\" \n", Server_IP);
    // set data_size
    if (data_size == 0) {
        data_size = 1460;
        IPERF_LOGI("Default datagram size = %d Bytes\n", data_size);
    }
    // set send_one_pkt_us according to bw
    if (bw > 0) {
        // send_one_pkt_us = 1000 * 1000 / (bw / data_size)  -> us
        send_one_pkt_us = 1000 * 1000 / (bw / data_size);
        IPERF_LOGI("send_one_pkt_us = %d us\n", send_one_pkt_us);
    }
    // set send_time
    if (send_time == 0) {
        if (num_tag == 1) {
            send_time = 999999;
        } else {
            send_time = 10;
            IPERF_LOGI("Default send times = %d (secs)\n", send_time);
        }
    }

    // Create a new UDP socket
    if (ipv6_flag == 0) {
        sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    }
#if LWIP_IPV6
    else {
        sockfd = socket(AF_INET6, SOCK_DGRAM, 0);
    }
#endif
    // Create new UDP socket fail
    if (sockfd < 0) {
        IPERF_LOGI("[%s:%d] sockfd = %d\n", __FUNCTION__, __LINE__, sockfd);
        if (parameters) {
            vPortFree(parameters);
        }
        if (g_iperf_context.callback) {
            g_iperf_context.callback(NULL);
        }
        vTaskDelete(NULL);
    }
    // set IP TOS, only for IPv4
    if (ipv6_flag == 0) {
        if (setsockopt(sockfd, IPPROTO_IP, IP_TOS, &tos, sizeof(tos)) < 0) {
            IPERF_LOGI("Set TOS: fail!\n");
        }
    }
    // set server port
    if (server_port == 0) {
        if (ipv6_flag == 0) {
            servaddr.sin_port = htons(IPERF_DEFAULT_PORT);
        }
#if LWIP_IPV6
        else {
            servaddr6.sin6_port = htons(IPERF_DEFAULT_PORT);
        }
#endif
        IPERF_LOGI("Default server port = %d \n", IPERF_DEFAULT_PORT);
    } else {
        if (ipv6_flag == 0) {
            servaddr.sin_port = htons(server_port);
        }
#if LWIP_IPV6
        else {
            servaddr6.sin6_port = htons(server_port);
        }
#endif
        IPERF_LOGI("Set server port = %d \n", server_port);
    }
    // UDP Multicast settings
#if LWIP_IGMP
    if (ipv6_flag == 0 && mcast_tag == 1) {
        group.imr_multiaddr.s_addr = inet_addr(mcast);
        group.imr_interface.s_addr = htonl(INADDR_ANY);
        if (setsockopt(sockfd, IPPROTO_IP, IP_ADD_MEMBERSHIP, (char *)&group, sizeof(struct ip_mreq)) < 0) {
            IPERF_LOGI("Setsockopt failed - multicast settings\n");
        }
        if (mcast_ttl >= 1) {
            if (setsockopt(sockfd, IPPROTO_IP, IP_MULTICAST_TTL, (uint8_t *)&mcast_ttl, sizeof(uint8_t)) < 0) {
                IPERF_LOGI("Setsockopt failed - multicast settings\n");
            }
        }
    }
#endif
    // udp connect
    if (ipv6_flag == 0) {
        ret = connect(sockfd, (struct sockaddr *)&servaddr, sizeof(servaddr));
    }
#if LWIP_IPV6
    else {
        ret = connect(sockfd, (struct sockaddr *)&servaddr6, sizeof(servaddr6));
    }
#endif
    // udp connect fail
    if (ret < 0) {
        IPERF_LOGI("Connect failed\n");
        close(sockfd);
        if (parameters) {
            vPortFree(parameters);
        }
        if (g_iperf_context.callback) {
            g_iperf_context.callback(NULL);
        }
        vTaskDelete(NULL);
    }
    // UDP Client DUAL_Test -d
    if (dual_tag == 1) {
        char **g_iperf_param = NULL;
        g_iperf_param = pvPortMalloc(IPERF_COMMAND_BUFFER_NUM * IPERF_COMMAND_BUFFER_SIZE);
        if (g_iperf_param == NULL) {
            IPERF_LOGE("Warning: No enough memory to running iperf.");
        } else {
            memset(g_iperf_param, 0, IPERF_COMMAND_BUFFER_NUM * IPERF_COMMAND_BUFFER_SIZE);
            xTaskCreate((TaskFunction_t)iperf_udp_run_server, IPERF_TASK_NAME, IPERF_TASK_STACKSIZE / sizeof(portSTACK_TYPE), g_iperf_param, IPERF_TASK_PRIO , NULL);
        }
    }

    // Malloc UDP data
    str = pvPortCalloc(1, IPERF_TEST_BUFFER_SIZE);
    if (str == NULL) {
        IPERF_LOGI("not enough buffer to send data!\n");
        close(sockfd);
        if (parameters) {
            vPortFree(parameters);
        }
        if (g_iperf_context.callback) {
            g_iperf_context.callback(NULL);
        }
        vTaskDelete(NULL);
    }
    memset(str, 0, IPERF_TEST_BUFFER_SIZE);
    iperf_pattern(str, IPERF_TEST_BUFFER_SIZE);

    // Init UDP data header
    udp_h = (UDP_datagram *)&str[0];
    client_h = (client_hdr *)&str[12];
    // UDP Client DUAL_Test -d
    if (dual_tag == 1) {
        client_h->flags = htonl(IPERF_HEADER_DUAL_TEST);
    } else if (tradeoff_tag == 1) {
        client_h->flags = htonl(IPERF_HEADER_VERSION1);
    } else {
        client_h->flags = 0;
    }
    client_h->num_threads = htonl(1);
    client_h->port = htonl(IPERF_DEFAULT_PORT);
    client_h->buffer_len = 0;
    client_h->win_band = 0;
    if (num_tag != 1) { // send_time mode
        int count10ms = (send_time * 1000 / 10);
        client_h->amount = htonl(-count10ms);
    } else { // send_num mode
        client_h->amount = htonl((long)(total_send));
        client_h->amount &= htonl(0x7FFFFFFF);
    }

    init_cost_ms = iperf_get_current_ms();
    // udp client -b send_one_pkt_us
    uint32_t do_send_start = 0;
    uint32_t do_send_end = 0;
    uint32_t delay_us = 0;
    do {
        if (send_one_pkt_us > 0) {
            hal_gpt_get_free_run_count(HAL_GPT_CLOCK_SOURCE_32K, &do_send_start);
        }

        udp_h->id = htonl(udp_h_id++);
        udp_h->tv_sec = htonl((current_ms - init_cost_ms) / 1000);
        udp_h->tv_usec = htonl(0);

        nbytes = send(sockfd, str, data_size, 0);
//        if (nbytes < data_size) {
//            IPERF_LOGI("exception nbytes = %d ", nbytes);
//        }
        iperf_calculate_result(nbytes, &pkt_count);

        // delay some ms in UDP_Dual_Test to increase throughput
        if (dual_tag == 1 || g_iperf_is_udp_client_dual_test == 1) {
            vTaskDelay(2);
        }

        if (num_tag == 1) {
            total_send -= nbytes;
        }
        // Reach total receive number "-n"
        if (num_tag == 1 && total_send < 0) {
            IPERF_LOGI("Finish Sending ");
            break;
        }

        // show temp result according to interval_tag
        if (interval_tag > 0) {
            current_ms = iperf_get_current_ms();
            int sec_num = (current_ms - init_cost_ms) / 1000;
            if (sec_num >= current_interval) {
                count_t result_count;
                IPERF_LOGI("Interval: %d - %d sec   ", sec_num - interval_tag, sec_num);
                iperf_diff_count(&result_count, &pkt_count, &tmp_count);
                iperf_display_result("UDP Client",  interval_tag * 1000, &result_count);
                iperf_copy_count(&pkt_count, &tmp_count);
                current_interval += interval_tag;
            }
        }

        // delay for UDP Client -b bandwidth option
        delay_us = 0;
        if (send_one_pkt_us > 0) {
            hal_gpt_get_free_run_count(HAL_GPT_CLOCK_SOURCE_32K, &do_send_end);
            double code_cost_us = (double)((do_send_end - do_send_start) * 1000.0 * 1000.0 / 32768.0);
            if (code_cost_us < send_one_pkt_us) {
                delay_us = send_one_pkt_us - (uint32_t)code_cost_us;
                hal_gpt_delay_us(delay_us);
            } else {
                DBGPRINT_IPERF("code_cost_us=%f > %d ", code_cost_us, send_one_pkt_us);
            }
            // DBGPRINT_IPERF("code_cost_us=%f delay_us=%d send_one_pkt_us=%d", code_cost_us,
            //    delay_us, send_one_pkt_us);
        }
        current_ms = iperf_get_current_ms();
    } while ((current_ms  - init_cost_ms) < send_time * 1000);

    // show Total Result (UDP Client)
    iperf_display_result("[Total]UDP Client", current_ms - init_cost_ms, &pkt_count);

    // send the last END datagram and receive ACK(server report), try 20 times
    udp_h->id = htonl((-udp_h_id));
    udp_h->tv_sec = htonl((current_ms - init_cost_ms) / 1000);
    udp_h->tv_usec = htonl(((current_ms - init_cost_ms) % 1000) * 1000);
    int try_num = 0;
    struct timeval timeout;
    timeout.tv_sec = 1;
    timeout.tv_usec = 0;
    if (setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO, (char *)&timeout, sizeof(timeout)) < 0) {
        DBGPRINT_IPERF("Setsockopt failed - cancel receive timeout\n");
    }
    do {
        try_num++;
        DBGPRINT_IPERF("try to send END UDP datagram: %d!", try_num);
        nbytes = send(sockfd, str, data_size, 0);
        if (nbytes > 0) {
            DBGPRINT_IPERF("try to receive ACK of END UDP datagram!");
            char recvBuf[100] = {0};
            nbytes = recv(sockfd, recvBuf, 100, 0);
            if (nbytes > 0) {
                DBGPRINT_IPERF("try to receive ACK of END UDP datagram (pass): %d!", try_num);
                break;
            }
        } else {
            DBGPRINT_IPERF("try to send END UDP datagram (fail): %d!", try_num);
            vTaskDelay(100);
        }
    } while (nbytes <= 0 && try_num <= 20);

    // clean & free
    if (str) {
        vPortFree(str);
    }
    if (parameters) {
        vPortFree(parameters);
    }
    IPERF_LOGI("UDP Client close socket!");
    close(sockfd);
    // UDP Client DUAL_Test -d
    g_iperf_is_udp_client_dual_test = 0;

    // tradeoff testing
    if (tradeoff_tag == 1) {
        IPERF_LOGI("Tradoff test, start server-side.");
        g_iperf_is_tradeoff_test_client = 1;
        iperf_udp_run_server(NULL);
        g_iperf_is_tradeoff_test_client = 0;
    }

    // g_iperf_context callback
    g_iperf_context.result.data_size = data_size;
    g_iperf_context.result.time = current_ms  - init_cost_ms;
    if (g_iperf_context.callback) {
        g_iperf_context.callback(&g_iperf_context.result);
    }

    // For tradeoff mode, task will be deleted in iperf_udp_run_server
    if (g_iperf_is_tradeoff_test_server == 0) {
        vTaskDelete(NULL);
    }
}

static void iperf_calculate_result(int pkt_size, count_t *pkt_count)
{
    if (pkt_size > 0) {
        pkt_count->Bytes += pkt_size;
        pkt_count->times++;
    }
}

static char *iperf_ftoa(double f, char *buf, int precision)
{
    char *ptr = buf;
    char *p = ptr;
    char *p1;
    char c;
    long intPart;
    char *temp_str;
    // sign stuff
    if (f < 0) {
        f = -f;
        *ptr++ = '-';
    }
    f += (double)0.005;
    intPart = (long)f;
    f -= intPart;
    if (!intPart) {
        *ptr++ = '0';
    } else {
        // save start pointer
        p = ptr;

        // convert (reverse order)
        while (intPart) {
            *p++ = '0' + intPart % 10;
            intPart /= 10;
        }
        // save end pos
        p1 = p;
        // reverse result
        while (p > ptr) {
            c = *--p;
            *p = *ptr;
            *ptr++ = c;
        }
        // restore end pos
        ptr = p1;
    }
    // decimal part
    if (precision) {
        // place decimal point
        *ptr++ = '.';
        // convert
        while (precision--) {
            f *= (double)10.0;
            c = (char)f;
            *ptr++ = '0' + c;
            f -= c;
        }
    }
    // terminating zero
    *ptr = 0;
    temp_str = --ptr;
    while (*temp_str != '.') {
        if (*temp_str == '0') {
            *temp_str = '\0';
        } else {
            break;
        }
        temp_str--;
    }
    if ((*(temp_str + 1) == '\0') && (*temp_str == '.')) {
        *(temp_str + 1) = '0';
    }
    return buf;
}

static int byte_snprintf(char *outString, double inNum, char inFormat)
{
    int conv = kConv_Unit;
    double tmpNum = inNum;
    if (isupper((int)inFormat)) {
        while (tmpNum >= (double)1024.0  &&  conv <= kConv_Giga) {
            tmpNum /= (double)1024.0;
            conv++;
        }
    } else {
        while (tmpNum >= (double)1000.0  &&  conv <= kConv_Giga) {
            tmpNum /= (double)1000.0;
            conv++;
        }
    }
    if (conv > 3) {
        conv = 3;
    }

    if (!isupper((int)inFormat)) {
        inNum *= kConversionForBits[ conv ];
    } else {
        inNum *= kConversion [conv];
    }
    iperf_ftoa(inNum, outString, 2);
    return conv;
} /* end byte_snprintf */

void iperf_display_result(char *report_title, unsigned ms_num, count_t *pkt_count)
{
    char s[9] = {0};
    double output = 0.0;
    int conv;
    memcpy(g_iperf_context.result.report_title, report_title, strlen(report_title));
    DBGPRINT_IPERF("ms_num = %d, Bytes= %d\n", ms_num, pkt_count->Bytes);

    output = (double)(pkt_count->Bytes);
    conv = byte_snprintf(s, output, 'K');
    IPERF_LOGI("%s Total len: %s %s", report_title, s, kLabel_Byte[conv]);
    sprintf(g_iperf_context.result.total_len, "%s %s", s, kLabel_Byte[conv]);

    output = (double)(pkt_count->Bytes * 8) / (double)(ms_num / 1000.0);
    conv = byte_snprintf(s, output, 'k');
    IPERF_LOGI("%s Bandwidth: %s %s/sec.", report_title, s, kLabel_bit[conv]);
    sprintf(g_iperf_context.result.bandwidth, "%s %s/sec.", s, kLabel_bit[conv]);

    DBGPRINT_IPERF("Receive times: %d\n", pkt_count->times);
}

static void iperf_reset_count(count_t *pkt_count)
{
    pkt_count->Bytes = 0;
    pkt_count->times = 0;
}

static void iperf_copy_count(count_t *pkt_count_src, count_t *pkt_count_dest)
{
    pkt_count_dest->Bytes = pkt_count_src->Bytes;
    pkt_count_dest->times = pkt_count_src->times;
}

static void iperf_diff_count(count_t *result_count, count_t *pkt_count, count_t *tmp_count)
{
    /* pkt_count > tmp_count */
    result_count->times = pkt_count->times - tmp_count->times;
    if (pkt_count->Bytes >= tmp_count->Bytes) {
        result_count->Bytes = pkt_count->Bytes - tmp_count->Bytes;
    } else {
        IPERF_LOGI("Warning: Diff data is wrong.");
    }
}

static uint32_t iperf_get_current_ms()
{
    uint32_t count = 0;
    uint64_t count_temp = 0;
    uint32_t ms = 0;
    hal_gpt_status_t ret_status;

    ret_status = hal_gpt_get_free_run_count(HAL_GPT_CLOCK_SOURCE_32K, &count);
    if (HAL_GPT_STATUS_OK != ret_status) {
        IPERF_LOGI("[%s:%d]get count error, ret_status = %d", __FUNCTION__, __LINE__, ret_status);
    }
    hal_gpt_get_free_run_count(HAL_GPT_CLOCK_SOURCE_32K, &end_count);
    hal_gpt_get_duration_count(start_count, end_count, &count);

    count_temp = (uint64_t)count * 1000;
    ms = (uint32_t)(count_temp / 32768);
    return ms;
}

#ifdef UDP_SERVER_JITTER_REPORT_ENABLE
static uint32_t iperf_get_current_us()
{
    uint32_t count = 0;
    uint64_t count_temp = 0;
    uint32_t us = 0;
    hal_gpt_status_t ret_status;

    ret_status = hal_gpt_get_free_run_count(HAL_GPT_CLOCK_SOURCE_32K, &count);
    if (HAL_GPT_STATUS_OK != ret_status) {
        IPERF_LOGI("[%s:%d]get count error, ret_status = %d", __FUNCTION__, __LINE__, ret_status);
    }
    hal_gpt_get_free_run_count(HAL_GPT_CLOCK_SOURCE_32K, &end_count);
    hal_gpt_get_duration_count(start_count, end_count, &count);

    count_temp = (uint64_t)count * 1000 * 1000;
    us = (uint32_t)(count_temp / 32768);
    return us;
}
#endif

void iperf_set_debug_mode(bool debug)
{
#ifdef MTK_IPERF_DEBUG_ENABLE
    g_iperf_debug_feature = debug;
#endif
}

int iperf_format_transform(char *param)
{
    char *temp;
    int data_size = 0;
    int i;
    temp = param;
    for (i = 0; temp[i] != '\0'; i++) {
        if (temp[i] == 'k') {
            temp[i] = '\0';
            data_size = (int)(1000 * atof(temp));
        } else if (temp[i] == 'm') {
            temp[i] = '\0';
            data_size = (int)(1000 * 1000 * atof(temp));
        } else if (temp[i] == 'K') {
            temp[i] = '\0';
            data_size = (int)(1000 * atof(temp));
        } else if (temp[i] == 'M') {
            temp[i] = '\0';
            data_size = (int)(1000 * 1000 * atof(temp));
        } else {
            data_size = atoi(param);
        }
    }
    return data_size;
}

/*
 * Initialize the buffer with a pattern of (index mod 10).
 */
static void iperf_pattern(char *outBuf, int inBytes)
{
    while (inBytes -- > 0) {
        outBuf[inBytes] = (inBytes % 10) + '0';
    }
}

static int iperf_get_tos(char *tos_str)
{
    int tos = 0;
    int tos_temp = 0;
    if (tos_str[0] == '0' && (tos_str[1] == 'x' || tos_str[1] == 'X')) {
        tos_str = &tos_str[2];
        for (int i = 0; i < strlen(tos_str); i++) {
            if (tos_str[i] <= '9') {
                tos_temp = tos_str[i] - '0';
            } else {
                tos_temp = ((tos_str[i] <= 'F') ? (tos_str[i] - 'A') : (tos_str[i] - 'a')) + 10;
            }
            tos = tos * 16 + tos_temp;
        }
    } else {
        tos = atoi(tos_str);
    }
    return tos;
}

void iperf_show_MSS(int sockfd)
{
    int mss_val = 0;
    int opt_len = 4;
    int ret = getsockopt(sockfd, IPPROTO_TCP, TCP_MAXSEG, &mss_val, (socklen_t *)&opt_len);
    if (ret == 0) {
        IPERF_LOGI("Connected, Get TCP MSS = %d \n", mss_val);
    } else {
        IPERF_LOGI("Connected, Get TCP MSS fail, ret = %d \n", ret);
    }
}

void iperf_set_MSS(int sockfd, int mss_val)
{
    int ret = setsockopt(sockfd, IPPROTO_TCP, TCP_MAXSEG, &mss_val, sizeof(int));
    if (ret == 0) {
        IPERF_LOGI("Connected, Set TCP MSS = %d \n", mss_val);
    } else {
        IPERF_LOGI("Connected, Set TCP MSS fail, ret = %d \n", ret);
    }
}

void iperf_disable_nagle(int sockfd)
{
    int disable_val = 1;
    int ret = setsockopt(sockfd, IPPROTO_TCP, TCP_NODELAY, &disable_val, sizeof(int));
    if (ret == 0) {
        IPERF_LOGI("Connected, iperf_disable_nagle successfully\n");
    } else {
        IPERF_LOGI("Connected, iperf_disable_nagle, ret = %d \n", ret);
    }
}

void iperf_register_callback(iperf_callback_t callback)
{
    g_iperf_context.callback = callback;
}

