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
#include "sys_init.h"
#include "wifi_api.h"
#include "lwip/netif.h"
#include "lwip/tcpip.h"
#include "lwip/sockets.h"
#include "ethernetif.h"
#include "lwip/sockets.h"
#include "netif/etharp.h"
#include "portmacro.h"
#include "wifi_lwip_helper.h"
#include "task_def.h"

#if defined(MTK_MINICLI_ENABLE)
#include "cli_def.h"
#endif

#define SOCK_TCP_SRV_PORT        6500
#define SOCK_UDP_SRV_PORT        6600
#define SOCK_6_TCP_SRV_PORT      7000
#define SOCK_6_UDP_SRV_PORT      7500
#define TRX_PACKET_COUNT         5

/**
  * Create the log control block for network example.
  * User needs to define their own log control blocks as project needs.
  * Please refer to the log dev guide under /doc folder for more details.
  */
log_create_module(lwip_socket_example, PRINT_LEVEL_INFO);

/**
  * @brief  Tcp client create socket, connect tcp server and send/receive data.
  * @param  None
  * @return int: test result of tcp_client
  */
static int tcp_client_test(void)
{
    int s;
    int ret;
    struct sockaddr_in addr;
    int count = 0;
    int rcv_len, rlen;
    char rcv_buf[32] = {0};
    struct netif *sta_if = netif_find_by_type(NETIF_TYPE_STA);

    vTaskDelay(5000);
    char send_data[] = "Hello Server!";

    LOG_I(lwip_socket_example, "tcp_client_test starts");

    memset(&addr, 0, sizeof(addr));
    addr.sin_len = sizeof(addr);
    addr.sin_family = AF_INET;
    addr.sin_port = lwip_htons(SOCK_TCP_SRV_PORT);
    /**
     * Airoha Change
     * Use inet_addr_from_ip4addr to replace inet_addr_from_ipaddr
     */
    //inet_addr_from_ipaddr(&addr.sin_addr, netif_ip4_addr(sta_if));
    inet_addr_from_ip4addr(&addr.sin_addr, netif_ip4_addr(sta_if));
    /** Airoha Change End */

    /* Create the socket */
    s = lwip_socket(AF_INET, SOCK_STREAM, 0);
    if (s < 0) {
        LOG_I(lwip_socket_example, "TCP client create failed");
        goto idle;
    }

    /* Connect */
    ret = lwip_connect(s, (struct sockaddr *)&addr, sizeof(addr));
    if (ret < 0) {
        lwip_close(s);
        LOG_I(lwip_socket_example, "TCP client connect failed");
        goto idle;
    }

    while (count < TRX_PACKET_COUNT) {
        /* Write something */
        ret = lwip_write(s, send_data, sizeof(send_data));
        LOG_I(lwip_socket_example, "TCP client write:ret = %d", ret);

        LOG_I(lwip_socket_example, "TCP client waiting for data...");
        rcv_len = 0;
        while (rcv_len < sizeof(send_data)) {  //sonar client
            rlen = lwip_recv(s, &rcv_buf[rcv_len], sizeof(rcv_buf) - 1 - rcv_len, 0);
            rcv_len += rlen;
        }
        LOG_I(lwip_socket_example, "TCP client received data:%s", rcv_buf);

        count++;
        vTaskDelay(2000);
    }

    /* close */
    ret = lwip_close(s);
    LOG_I(lwip_socket_example, "TCP client s close:ret = %d", ret);
    return ret;
idle:
    LOG_I(lwip_socket_example, "TCP client test completed");
    return -1;
}

/**
  * @brief  Tcp server create socket, wait for client connection and receive/send data
  * @param  None
  * @return None
  */
static void tcp_server_test(void)
{
    int s;
    int c;
    int ret;
    int rlen;
    struct sockaddr_in addr;
    char srv_buf[32] = {0};
    LOG_I(lwip_socket_example, "tcp_server_test starts");

    memset(&addr, 0, sizeof(addr));
    addr.sin_len = sizeof(addr);
    addr.sin_family = AF_INET;
    addr.sin_port = lwip_htons(SOCK_TCP_SRV_PORT);
    addr.sin_addr.s_addr = lwip_htonl(IPADDR_ANY);

    /* Create the socket */
    s = lwip_socket(AF_INET, SOCK_STREAM, 0);
    if (s < 0) {
        LOG_I(lwip_socket_example, "TCP server create failed");
        goto done;
    }

    ret = lwip_bind(s, (struct sockaddr *)&addr, sizeof(addr));
    if (ret < 0) {
        LOG_I(lwip_socket_example, "TCP server bind failed");
        goto clean;
    }

    ret = lwip_listen(s, 0);
    if (ret < 0) {
        LOG_I(lwip_socket_example, "TCP server listen failed");
        goto clean;
    }

    do {
        socklen_t sockaddr_len = sizeof(addr);
        c = lwip_accept(s, (struct sockaddr *)&addr, &sockaddr_len);
        if (c < 0) {
            LOG_I(lwip_socket_example, "TCP server accept error");
            break;   //connection request.
        }

        LOG_I(lwip_socket_example, "TCP server waiting for data...");
        while ((rlen = lwip_read(c, srv_buf, sizeof(srv_buf) - 1)) != 0) {
            if (rlen < 0) {
                LOG_I(lwip_socket_example, "read error");
                break;
            }
            srv_buf[rlen] = 0; //for the next statement - printf string.
            LOG_I(lwip_socket_example, "TCP server received data:%s", srv_buf);

            lwip_write(c, srv_buf, rlen);      // sonar server
        }

        lwip_close(c);
    }while(0);

clean:
    lwip_close(s);
    LOG_I(lwip_socket_example, "TCP server s close:ret = %d", ret);
done:
    LOG_I(lwip_socket_example, "TCP server test completed");
}

/**
  * @brief  Udp client create socket and send/receive data
  * @param  None
  * @return  int: test result of udp_client
  */
static int udp_client_test(void)
{
    int s;
    int ret;
    int rlen;
    struct sockaddr_in addr;
    int count = 0;
    char rcv_buf[32] = {0};
    char send_data[] = "Hello Server!";
    LOG_I(lwip_socket_example, "udp_client_test starts");
    struct netif *sta_if = netif_find_by_type(NETIF_TYPE_STA);

    memset(&addr, 0, sizeof(addr));
    addr.sin_len = sizeof(addr);
    addr.sin_family = AF_INET;
    addr.sin_port = lwip_htons(SOCK_UDP_SRV_PORT);
    /**
     * Airoha Change
     * Use inet_addr_from_ip4addr to replace inet_addr_from_ipaddr
     */
    //inet_addr_from_ipaddr(&addr.sin_addr, netif_ip4_addr(sta_if));
    inet_addr_from_ip4addr(&addr.sin_addr, netif_ip4_addr(sta_if));
    /** Airoha Change End */

    /* Create the socket */
    s = lwip_socket(AF_INET, SOCK_DGRAM, 0);
    if (s < 0) {
        LOG_I(lwip_socket_example, "UDP client create failed");
        goto idle;
    }

    /* Connect */
    ret = lwip_connect(s, (struct sockaddr *)&addr, sizeof(addr));
    if (ret < 0) {
        lwip_close(s);
        LOG_I(lwip_socket_example, "UDP client connect failed");
        goto idle;
    }

    while (count < TRX_PACKET_COUNT) {
        /* Write something */
        ret = lwip_write(s, send_data, sizeof(send_data));
        LOG_I(lwip_socket_example, "UDP client write:ret = %d", ret);

        LOG_I(lwip_socket_example, "UDP client waiting for server data...");
        rlen = lwip_read(s, rcv_buf, sizeof(rcv_buf) - 1);
        rcv_buf[rlen] = 0;
        LOG_I(lwip_socket_example, "UDP client received data:%s", rcv_buf);
        count++;
        vTaskDelay(2000);
    }

    /* Close */
    ret = lwip_close(s);
    LOG_I(lwip_socket_example, "UDP client s close:ret = %d", ret);
    return ret;
idle:
    LOG_I(lwip_socket_example, "UDP client test completed");
    return -1;
}

/**
  * @brief  Udp server create socket and receive/send data
  * @param  None
  * @return None
  */
static void udp_server_test(void)
{
    int s;
    int ret;
    struct sockaddr_in addr, clnt_addr;
    char rcv_buf[32] = {0};
    int count = 0;
    LOG_I(lwip_socket_example, "udp_server_test starts");

    memset(&addr, 0, sizeof(addr));
    addr.sin_len = sizeof(addr);
    addr.sin_family = AF_INET;
    addr.sin_port = lwip_htons(SOCK_UDP_SRV_PORT);
    addr.sin_addr.s_addr = lwip_htonl(IPADDR_ANY);

    /* Create the socket */
    s = lwip_socket(AF_INET, SOCK_DGRAM, 0);
    if (s < 0) {
        LOG_I(lwip_socket_example, "UDP server create failed");
        goto idle;
    }

    ret = lwip_bind(s, (struct sockaddr *)&addr, sizeof(addr));
    if (ret < 0) {
        lwip_close(s);
        LOG_I(lwip_socket_example, "UDP server bind failed");
        goto idle;
    }

    while (count < TRX_PACKET_COUNT) {
        socklen_t clnt_len = sizeof(clnt_addr);
        ret = lwip_recvfrom(s, rcv_buf, sizeof(rcv_buf), 0, (struct sockaddr *)&clnt_addr, &clnt_len);
        if (ret <= 0) {
            lwip_close(s);
            LOG_I(lwip_socket_example, "UDP server recv failed");
            goto idle;
        }
        LOG_I(lwip_socket_example, "UDP server received data:%s", rcv_buf);

        lwip_sendto(s, rcv_buf, strlen(rcv_buf), 0, (struct sockaddr *)&clnt_addr, clnt_len);

        count++;
    }

    ret = lwip_close(s);
    LOG_I(lwip_socket_example, "UDP server s close:ret = %d", ret);
idle:
    LOG_I(lwip_socket_example, "UDP server test completed");
}

/**
 * Airoha Change
 * Add IPv6 socket test support
 *
 * Link local address cannot be used to run the socket test
 * Must use the global address to run the socket test
 * */
#if LWIP_IPV6

#define IPV6_TCP_CLIENT_SEND_BUF "Hello IPv6 TCP Server, This Is IPv6 TCP Client"
#define IPV6_TCP_SERVER_SEND_BUF "Hello IPv6 TCP Client, This Is IPv6 TCP Server"

#define IPV6_TCP_CLIENT_SEND_BUF_LEN (strlen(IPV6_TCP_CLIENT_SEND_BUF) + 1)
#define IPV6_TCP_SERVER_SEND_BUF_LEN (strlen(IPV6_TCP_SERVER_SEND_BUF) + 1)

#define IPV6_UDP_CLIENT_SEND_BUF "Hello IPv6 UDP Server, This Is IPv6 UDP Client"
#define IPV6_UDP_SERVER_SEND_BUF "Hello IPv6 UDP Client, This Is IPv6 UDP Server"

#define IPV6_UDP_CLIENT_SEND_BUF_LEN (strlen(IPV6_UDP_CLIENT_SEND_BUF) + 1)
#define IPV6_UDP_SERVER_SEND_BUF_LEN (strlen(IPV6_UDP_SERVER_SEND_BUF) + 1)

int global_ip6_addr_index = -1;

static void tcp_ipv6_server_test(void)
{
    struct sockaddr_in6             srv_in6_addr, clt_in6_addr;
    int                             socket_id = 0;
    int                             client_socket_id = 0;
    int                             ret = 0;
    int                             read_len = 0;
    char                            read_buf[120] = {0};

    LOG_I(lwip_socket_example, "tcp_ipv6_server_test start");

    memset(&srv_in6_addr, 0, sizeof(srv_in6_addr));
    srv_in6_addr.sin6_len = sizeof(struct sockaddr_in6);
    srv_in6_addr.sin6_family = AF_INET6;
    srv_in6_addr.sin6_port = lwip_htons(SOCK_6_TCP_SRV_PORT);
    srv_in6_addr.sin6_addr = in6addr_any;

    socket_id = lwip_socket(AF_INET6, SOCK_STREAM, 0);
    if (socket_id < 0) {
        LOG_I(lwip_socket_example, "TCP IPv6 Server Socket Create Failed");
        return;
    }
    ret = lwip_bind(socket_id, (struct sockaddr *)&srv_in6_addr, sizeof(srv_in6_addr));
    if (ret < 0) {
        lwip_close(socket_id);
        LOG_I(lwip_socket_example, "TCP IPv6 Server Socket Bind Failed");
        return;
    }
    LOG_I(lwip_socket_example, "TCP IPv6 Server Socket Bind Succeed");

    ret = lwip_listen(socket_id, 0);
    if (ret < 0) {
        lwip_close(socket_id);
        LOG_I(lwip_socket_example, "TCP IPv6 Server Socket Listen Failed");
        return;
    }
    LOG_I(lwip_socket_example, "TCP IPv6 Server Socket Listen Succeed");

    ret = sizeof(clt_in6_addr);
    client_socket_id = lwip_accept(socket_id, (struct sockaddr *)&clt_in6_addr, (socklen_t *)&ret);
    if (client_socket_id < 0) {
        lwip_close(socket_id);
        LOG_I(lwip_socket_example, "TCP IPv6 Server Socket Accept Failed");
        return;
    }

    LOG_I(lwip_socket_example, "Accept Client Socket Address : %s", inet6_ntoa(clt_in6_addr.sin6_addr));

    while ((read_len = lwip_read(client_socket_id, read_buf, sizeof(read_buf) - 1)) != 0) {
        if (read_len < 0) {
            LOG_I(lwip_socket_example, "TCP IPv6 Server Socket Read Failed : %d", read_len);
            break;
        }
        read_buf[read_len] = 0;
        LOG_I(lwip_socket_example, "TCP IPv6 Server Socket read : %s", read_buf);

        lwip_write(client_socket_id, IPV6_TCP_SERVER_SEND_BUF, IPV6_TCP_SERVER_SEND_BUF_LEN);
    }

    lwip_close(client_socket_id);
    lwip_close(socket_id);

    LOG_I(lwip_socket_example, "TCP IPv6 Socket Server Finished");
}

static int tcp_ipv6_client_test(void)
{
    struct sockaddr_in6             clt_in6_addr;
    int                             socket_id = 0;
    int                             ret = 0;
    char                            read_buf[120] = {0};
    int                             count = 0;
    struct netif                    *sta_if = netif_find_by_type(NETIF_TYPE_STA);

    LOG_I(lwip_socket_example, "tcp_ipv6_client_test start");

    memset(&clt_in6_addr, 0, sizeof(clt_in6_addr));
    clt_in6_addr.sin6_len = sizeof(struct sockaddr_in6);
    clt_in6_addr.sin6_family = AF_INET6;
    clt_in6_addr.sin6_port = lwip_htons(SOCK_6_TCP_SRV_PORT);
    inet6_addr_from_ip6addr(&(clt_in6_addr.sin6_addr), netif_ip6_addr(sta_if, global_ip6_addr_index));

#if 0
    /** For remote test */
    ret = ip6addr_aton("2001::b4bd:5cb1:3269:1a80", &(clt_in6_addr.sin6_addr));
    ret = ip6addr_aton("2001::20C:76FF:FE87:C56", &(clt_in6_addr.sin6_addr));

    if (ret == 0) {
        LOG_I(lwip_socket_example, "Failed to aton ip6 addr\n");
        return -1;
    }
#endif

    socket_id = lwip_socket(AF_INET6, SOCK_STREAM, 0);
    if (socket_id < 0) {
        LOG_I(lwip_socket_example, "TCP IPv6 Client Socket Create Failed");
        return -1;
    }

    ret = lwip_connect(socket_id, (struct sockaddr*)&clt_in6_addr, sizeof(clt_in6_addr));
    if (ret < 0) {
        lwip_close(socket_id);
        LOG_I(lwip_socket_example, "TCP IPv6 Client Socket Connect Failed, %d", ret);
        return -1;
    }

    LOG_I(lwip_socket_example, "TCP IPv6 Client Socket Connect Server Succeed");

    while (count < TRX_PACKET_COUNT) {
        ret = lwip_write(socket_id, IPV6_TCP_CLIENT_SEND_BUF, IPV6_TCP_CLIENT_SEND_BUF_LEN);
        if (ret < 0) {
            LOG_I(lwip_socket_example, "TCP IPv6 write failed : %d\n", ret);
            break;
        }
        memset(read_buf, 0, 120);
        ret = lwip_recv(socket_id, read_buf, sizeof(read_buf) - 1, 0);
        if (ret < 0) {
            LOG_I(lwip_socket_example, "TCP IPv6 receive failed : %d\n", ret);
            break;
        }
        LOG_I(lwip_socket_example, "TCP IPv6 Client Socket Read : %s", read_buf);
        count ++;
    }

    lwip_close(socket_id);
    LOG_I(lwip_socket_example, "TCP IPv6 Client Socket Finished, %d", count);
    if (count < TRX_PACKET_COUNT) {
        return -1;
    }
    return 0;
}


static void udp_ipv6_server_test(void)
{
    struct sockaddr_in6             srv_in6_addr, clt_in6_addr;
    int                             socket_id = 0;
    int                             ret = 0;
    int                             read_len = 0;
    char                            read_buf[120] = {0};

    LOG_I(lwip_socket_example, "udp_ipv6_server_test start");

    memset(&srv_in6_addr, 0, sizeof(srv_in6_addr));
    srv_in6_addr.sin6_len = sizeof(struct sockaddr_in6);
    srv_in6_addr.sin6_family = AF_INET6;
    srv_in6_addr.sin6_port = lwip_htons(SOCK_6_UDP_SRV_PORT);
    srv_in6_addr.sin6_addr = in6addr_any;

    socket_id = lwip_socket(AF_INET6, SOCK_DGRAM, 0);
    if (socket_id < 0) {
        LOG_I(lwip_socket_example, "UDP IPv6 Server Socket Create Failed");
        return;
    }
    ret = lwip_bind(socket_id, (struct sockaddr *)&srv_in6_addr, sizeof(srv_in6_addr));
    if (ret < 0) {
        lwip_close(socket_id);
        LOG_I(lwip_socket_example, "UDP IPv6 Server Socket Bind Failed");
        return;
    }
    LOG_I(lwip_socket_example, "UDP IPv6 Server Socket Bind Succeed");

    socklen_t clt_len = sizeof(struct sockaddr_in6);
    while ((read_len = lwip_recvfrom(socket_id, read_buf, sizeof(read_buf) - 1, 0, (struct sockaddr *)&clt_in6_addr, &clt_len)) != 0) {
        if (read_len < 0) {
            LOG_I(lwip_socket_example, "UDP IPv6 Server Socket Read Failed : %d", read_len);
            break;
        }
        read_buf[read_len] = 0;
        LOG_I(lwip_socket_example, "UDP IPv6 Server Socket read : %s", read_buf);

        lwip_sendto(socket_id, IPV6_UDP_SERVER_SEND_BUF, IPV6_UDP_SERVER_SEND_BUF_LEN, 0, (struct sockaddr *)&clt_in6_addr, clt_len);
    }

    lwip_close(socket_id);

    LOG_I(lwip_socket_example, "UDP IPv6 Socket Server Finished");
}

static int udp_ipv6_client_test(void)
{
    struct sockaddr_in6             clt_in6_addr;
    int                             socket_id = 0;
    int                             ret = 0;
    char                            read_buf[120] = {0};
    int                             count = 0;
    struct netif                    *sta_if = netif_find_by_type(NETIF_TYPE_STA);

    LOG_I(lwip_socket_example, "udp_ipv6_client_test start");

    memset(&clt_in6_addr, 0, sizeof(clt_in6_addr));
    clt_in6_addr.sin6_len = sizeof(struct sockaddr_in6);
    clt_in6_addr.sin6_family = AF_INET6;
    clt_in6_addr.sin6_port = lwip_htons(SOCK_6_UDP_SRV_PORT);
    inet6_addr_from_ip6addr(&(clt_in6_addr.sin6_addr), netif_ip6_addr(sta_if, global_ip6_addr_index));
#if 0
    /** For remote test */
    ret = ip6addr_aton("2001::20C:76FF:FE87:C56", &(clt_in6_addr.sin6_addr));
    if (ret == 0) {
        LOG_I(lwip_socket_example, "Failed to aton ip6 addr\n");
        return -1;
    }
#endif

    socket_id = lwip_socket(AF_INET6, SOCK_DGRAM, 0);
    if (socket_id < 0) {
        LOG_I(lwip_socket_example, "UDP IPv6 Client Socket Create Failed");
        return -1;
    }

    while (count < TRX_PACKET_COUNT) {
        ret = lwip_sendto(socket_id, IPV6_UDP_CLIENT_SEND_BUF, IPV6_UDP_CLIENT_SEND_BUF_LEN,
                            0, (struct sockaddr *)&(clt_in6_addr), sizeof(clt_in6_addr));
        if (ret < 0) {
            LOG_I(lwip_socket_example, "UDP IPv6 Client Socket Write Failed : %d", ret);
            break;
        }

        memset(read_buf, 0, 120);
        ret = lwip_read(socket_id, read_buf, sizeof(read_buf) - 1);
        LOG_I(lwip_socket_example, "UDP IPv6 Client Socket Read : %s", read_buf);
        count ++;
    }

    lwip_close(socket_id);
    LOG_I(lwip_socket_example, "UDP IPv6 Client Socket Finished, %d", count);
    if (count < TRX_PACKET_COUNT) {
        return -1;
    }
    return 0;
}

/**
 * Check the STA IPv6 address is global address or not
 * Only the global IPv6 address support to run the socket test
 * Otherwise, the lwip_connect should be failed in the ip6_route function (cannot find netif)
 */
static int ip6_check_sta_address_global()
{
    struct netif *sta_if = netif_find_by_type(NETIF_TYPE_STA);
    ip6_addr_t *ip6_addr = NULL;
    int addr_index = 0;

    if (sta_if == NULL) {
        LOG_I(lwip_socket_example, "Failed to get STA net if");
        return 0;
    }

    for (addr_index = 0; addr_index < LWIP_IPV6_NUM_ADDRESSES; addr_index ++) {
        ip6_addr = (ip6_addr_t *)netif_ip6_addr(sta_if, addr_index);
        if (!ip6_addr_isany(ip6_addr) && ip6_addr_isglobal(ip6_addr)
                && ip6_addr_isvalid(netif_ip6_addr_state(sta_if, addr_index))) {
            global_ip6_addr_index = addr_index;
            return 1;
        }
    }

    return 0;
}

static void tcp_ip6_server_thread(void *user_data)
{
    if (global_ip6_addr_index != -1) {
        tcp_ipv6_server_test();
    } else {
        LOG_I(common, "Tcp server IP6 thread -> global ip6 addr index is -1");
    }

    //Keep the task alive
    while (1) {
        vTaskDelay(1000 / portTICK_RATE_MS); // release CPU
    }
}

static void udp_ip6_server_thread(void *user_data)
{
    if (global_ip6_addr_index != -1) {
        udp_ipv6_server_test();
    } else {
        LOG_I(common, "UDP server IP6 thread -> global ip6 addr index is -1");
    }

    //Keep the task alive
    while (1) {
        vTaskDelay(1000 / portTICK_RATE_MS); // release CPU
    }
}

#endif
/** Airoha Change End */

/**
  * @brief     Tcp server test entry
  * @param[in] void *not_used:Not used
  * @return    None
  */
static void tcp_server_thread(void *not_used)
{
    tcp_server_test();

    //Keep the task alive
    while (1) {
        vTaskDelay(1000 / portTICK_RATE_MS); // release CPU
    }
}

/**
  * @brief     Udp server test entry
  * @param[in] void *not_used:Not used
  * @return    None
  */
static void udp_server_thread(void *not_used)
{
    udp_server_test();

    //Keep the task alive
    while (1) {
        vTaskDelay(1000 / portTICK_RATE_MS); // release CPU
    }
}

/**
  * @brief     Tcp/Udp client test entry
  * @param[in] void *not_used:Not used
  * @return    None
  */
static void client_test_thread(void *not_used)
{
    int ret = 0;
    ret = tcp_client_test();
    ret += udp_client_test();
    /**
     * Airoha Change
     * Add IPv6 client test support
     */
#if LWIP_IPV6
    if (global_ip6_addr_index != -1) {
        ret += tcp_ipv6_client_test();
        ret += udp_ipv6_client_test();
    } else {
        LOG_I(common, "Client test thread -> global ip6 addr index is -1");
    }
#endif
    /** Airoha Change End */
    if (ret == 0) {
        LOG_I(lwip_socket_example, "example project test success.\n");
    }
    //Keep the task alive
    while (1) {
        vTaskDelay(1000 / portTICK_RATE_MS); // release CPU
    }
}

/**
  * @brief     Create a task for tcp/udp test
  * @param[in] void *args:Not used
  * @return    None
  */
void user_entry(void *args)
{
    lwip_net_ready();
    LOG_I(lwip_socket_example, "Begin to create socket_sample_task");
#if LWIP_IPV6
    int ret = 0;
    while (true) {
        ret = ip6_check_sta_address_global();
        if (ret == 1) {
            break;
        }
        vTaskDelay(100 / portTICK_RATE_MS);
    }
#endif
    //xTaskHandle xHandle;
    if (pdPASS != xTaskCreate(tcp_server_thread,
                              SOCKET_TCPS_EXAMPLE_TASK_NAME,
                              SOCKET_EXAMPLE_TASK_STACKSIZE / sizeof(portSTACK_TYPE),
                              NULL,
                              SOCKET_EXAMPLE_TASK_PRIO,
                              NULL)) {
        LOG_I(lwip_socket_example, "Cannot create tcp_server_thread");
    }

    if (pdPASS != xTaskCreate(udp_server_thread,
                              SOCKET_UDPS_EXAMPLE_TASK_NAME,
                              SOCKET_EXAMPLE_TASK_STACKSIZE / sizeof(portSTACK_TYPE),
                              NULL,
                              SOCKET_EXAMPLE_TASK_PRIO,
                              NULL)) {
        LOG_I(lwip_socket_example, "Cannot create udp_server_thread");
    }

#if LWIP_IPV6
    if (pdPASS != xTaskCreate(tcp_ip6_server_thread,
                              SOCKET_TCP_IP6_S_EXAMPLE_TASK_NAME,
                              SOCKET_EXAMPLE_TASK_STACKSIZE / sizeof(portSTACK_TYPE),
                              NULL,
                              SOCKET_EXAMPLE_TASK_PRIO,
                              NULL)) {
        LOG_I(lwip_socket_example, "Cannot create tcp ip6 server thread");
    }

    if (pdPASS != xTaskCreate(udp_ip6_server_thread,
                              SOCKET_UDP_IP6_S_EXAMPLE_TASK_NAME,
                              SOCKET_EXAMPLE_TASK_STACKSIZE / sizeof(portSTACK_TYPE),
                              NULL,
                              SOCKET_EXAMPLE_TASK_PRIO,
                              NULL)) {
        LOG_I(lwip_socket_example, "Cannot create udp ip6 server thread");
    }
#endif

    if (pdPASS != xTaskCreate(client_test_thread,
                              SOCKET_CLINET_EXAMPLE_TASK_NAME,
                              SOCKET_EXAMPLE_TASK_STACKSIZE / sizeof(portSTACK_TYPE),
                              NULL,
                              SOCKET_EXAMPLE_TASK_PRIO,
                              NULL)) {
        LOG_I(lwip_socket_example, "Cannot create socket_sample_task");
    }

    LOG_I(lwip_socket_example, "Finish to create socket_sample_task");
    while (1) {
        vTaskDelay(1000 / portTICK_RATE_MS); // release CPU
    }
}
