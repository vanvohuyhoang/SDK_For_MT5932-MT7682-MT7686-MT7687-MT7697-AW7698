/*
 * Copyright (c) 2001, 2002 Swedish Institute of Computer Science.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without modification,
 * are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 * 3. The name of the author may not be used to endorse or promote products
 *    derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR IMPLIED
 * WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT
 * SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT
 * OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING
 * IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY
 * OF SUCH DAMAGE.
 *
 * This file is part of the lwIP TCP/IP stack.
 *
 * Author: Adam Dunkels <adam@sics.se>
 *
 */

#ifndef __LWIPOPTS_H__
#define __LWIPOPTS_H__


#include "FreeRTOSConfig.h"


/*
   ------------------------------------
   ----------- Core locking -----------
   ------------------------------------
*/
#define LWIP_TCPIP_CORE_LOCKING         1

/*
   ------------------------------------
   ---------- Memory options ----------
   ------------------------------------
*/
#define MEMP_MEM_MALLOC                 1
#define MEM_ALIGNMENT                   4
/* MEM_SIZE: the size of the heap memory. If the application will send
a lot of data that needs to be copied, this should be set high. */
#if defined(MTK_WIFI_TGN_VERIFY_ENABLE) && !defined(MTK_HOMEKIT_ENABLE)
#define MEM_SIZE                        (100 * 1024)
#else
#define MEM_SIZE                        (48 * 1024)
#endif

/*
   ------------------------------------------------
   ---------- Internal Memory Pool Sizes ----------
   ------------------------------------------------
*/
#define MEMP_NUM_NETCONN                10

/*
   ---------------------------------
   ---------- ARP options ----------
   ---------------------------------
*/
#define LWIP_ARP                        1
#define ARP_TABLE_SIZE                  10
#define ARP_MAXAGE                      300
#define ARP_QUEUEING                    1
#define ARP_QUEUE_LEN                   3
#define LWIP_ETHERNET                   LWIP_ARP
#define ETH_PAD_SIZE                    0


/*
   --------------------------------
   ---------- IP options ----------
   --------------------------------
*/
#define LWIP_IPV4                       1
#define IP_FORWARD                      0
#define IP_REASSEMBLY                   1
#define IP_FRAG                         1
#define IP_OPTIONS_ALLOWED              1
#define IP_REASS_MAXAGE                 15
#define IP_DEFAULT_TTL                  255

#if defined(MTK_WIFI_TGN_VERIFY_ENABLE)
#define IP_REASS_MAX_PBUFS              25
#else
#define IP_REASS_MAX_PBUFS              10
#endif

/*
   ----------------------------------
   ---------- DHCP options ----------
   ----------------------------------
*/
#define LWIP_DHCP                       1
#ifdef MTK_USER_FAST_TX_ENABLE
#define DHCP_DOES_ARP_CHECK             0
#else
#define DHCP_DOES_ARP_CHECK             1
#endif

/*
   ----------------------------------
   ---------- IGMP options ----------
   ----------------------------------
*/
#define LWIP_IGMP                       0

/*
   ----------------------------------
   ---------- DNS options -----------
   ----------------------------------
*/
#define LWIP_DNS                        1
#define DNS_TABLE_SIZE                  4
#define DNS_MAX_NAME_LENGTH             256
#define DNS_MAX_SERVERS                 2
#define DNS_MAX_RETRIES                 4


/*
   ---------------------------------
   ---------- UDP options ----------
   ---------------------------------
*/
#define LWIP_UDP                        1
#define LWIP_UDPLITE                    0

/*
   ---------------------------------
   ---------- TCP options ----------
   ---------------------------------
*/
#define LWIP_TCP                        1
#define TCP_MAXRTX                      12
#define TCP_SYNMAXRTX                   4

/* TCP Maximum segment size. */
#define TCP_MSS                         1460
/* TCP sender buffer space (bytes). */
/* MCC feature TP is low, need reduce the buffer configuration*/
#define TCP_SND_BUF                     (24 * 1024)
/* TCP sender buffer space (pbufs). This must be at least = 2 *
   TCP_SND_BUF/TCP_MSS for things to work. */
#define TCP_SND_QUEUELEN                ((4 * (TCP_SND_BUF) + (TCP_MSS - 1))/(TCP_MSS))
/* TCP receive window. */
#define TCP_WND                         (24 * 1024)

#define LWIP_TCP_SACK_OUT               0
#define TCP_QUEUE_OOSEQ                 1
#define LWIP_TCP_TIMESTAMPS             0

#define LWIP_WND_SCALE                  1
#define TCP_RCV_SCALE                   1

/*
   ----------------------------------
   ---------- Pbuf options ----------
   ----------------------------------
*/
/**
 * PBUF_LINK_HLEN: the number of bytes that should be allocated for a
 * link level header. The default is 14, the standard value for
 * Ethernet.
 */
#define RESERVED_HLEN                   0    //depend on CFG_CONNSYS_TXD_PAD_SIZE
#ifdef MTK_HIF_GDMA_ENABLE
#define PBUF_LINK_ENCAPSULATION_HLEN    2 // for WiFi headroom (TX zero copy)
#else
#define PBUF_LINK_ENCAPSULATION_HLEN    0
#endif
#define PBUF_LINK_HLEN                  (RESERVED_HLEN + 14 + ETH_PAD_SIZE)

/* PBUF_POOL_SIZE: the number of buffers in the pbuf pool. */
#define PBUF_POOL_SIZE                  10

/* PBUF_POOL_BUFSIZE: the size of each pbuf in the pbuf pool. */
/* packet of MT7687 IOT has extra TXD header and packet offset */
#define PBUF_POOL_BUFSIZE               1664


/*
   ------------------------------------------------
   ---------- Network Interfaces options ----------
   ------------------------------------------------
*/
#define LWIP_NETIF_STATUS_CALLBACK      1
#define LWIP_NETIF_LINK_CALLBACK        0
#define LWIP_NETIF_REMOVE_CALLBACK      0

/*
   ------------------------------------
   ---------- LOOPIF options ----------
   ------------------------------------
*/
#define LWIP_HAVE_LOOPIF                1
#define LWIP_NETIF_LOOPBACK             1
#define LWIP_LOOPBACK_MAX_PBUFS         12


/*
   ------------------------------------
   ---------- Thread options ----------
   ------------------------------------
*/
#define TCPIP_MBOX_SIZE                 16
#define DEFAULT_RAW_RECVMBOX_SIZE       16
#define DEFAULT_UDP_RECVMBOX_SIZE       16
#define DEFAULT_TCP_RECVMBOX_SIZE       16
#define DEFAULT_ACCEPTMBOX_SIZE         16

/*
   ------------------------------------
   ---------- Socket options ----------
   ------------------------------------
*/
#define LWIP_SOCKET                     1
#define LWIP_COMPAT_SOCKETS             1

#define LWIP_TCP_KEEPALIVE              0
#define LWIP_SO_SNDTIMEO                0
#define LWIP_SO_RCVTIMEO                1
#define SO_REUSE                        1

#define LWIP_SOCKET_SELECT              1
#define LWIP_SOCKET_POLL                0

/*
   ----------------------------------------
   ---------- Statistics options ----------
   ----------------------------------------
*/
/* ---------- Statistics options ---------- */
#if defined(MTK_LWIP_STATISTICS_ENABLE)
#define LWIP_STATS                      1
#define LWIP_STATS_DISPLAY              1
#endif

/*
   ---------------------------------------
   ---------- IPv6 options ---------------
   ---------------------------------------
*/
#define LWIP_IPV6                       0

/**
 * MTK_LWIP_DYNAMIC_DEBUG_ENABLE: Support LwIP dynamic debug
 * No dependency
 */
#if defined(MTK_LWIP_DYNAMIC_DEBUG_ENABLE)

struct lwip_debug_flags {
    char *debug_flag_name;
    uint32_t debug_flag;
};

extern struct lwip_debug_flags lwip_debug_flags[];

#define LWIP_DEBUG LWIP_DBG_ON
#define LWIP_DEBUG_IDX(idx) ((idx) >> 8)

#undef ETHARP_DEBUG
#undef NETIF_DEBUG
#undef PBUF_DEBUG
#undef API_LIB_DEBUG
#undef API_MSG_DEBUG
#undef SOCKETS_DEBUG
#undef ICMP_DEBUG
#undef IGMP_DEBUG
#undef INET_DEBUG
#undef IP_DEBUG
#undef IP_REASS_DEBUG
#undef RAW_DEBUG
#undef MEM_DEBUG
#undef MEMP_DEBUG
#undef SYS_DEBUG
#undef TIMERS_DEBUG
#undef TCP_DEBUG
#undef TCP_INPUT_DEBUG
#undef TCP_FR_DEBUG
#undef TCP_RTO_DEBUG
#undef TCP_CWND_DEBUG
#undef TCP_WND_DEBUG
#undef TCP_OUTPUT_DEBUG
#undef TCP_RST_DEBUG
#undef TCP_QLEN_DEBUG
#undef UDP_DEBUG
#undef TCPIP_DEBUG
#undef PPP_DEBUG
#undef SLIP_DEBUG
#undef DHCP_DEBUG
#undef AUTOIP_DEBUG
#undef SNMP_MSG_DEBUG
#undef SNMP_MIB_DEBUG
#undef DNS_DEBUG

#define ETHARP_DEBUG                    0x0000U
#define NETIF_DEBUG                     0x0100U
#define PBUF_DEBUG                      0x0200U
#define API_LIB_DEBUG                   0x0300U
#define API_MSG_DEBUG                   0x0400U
#define SOCKETS_DEBUG                   0x0500U
#define ICMP_DEBUG                      0x0600U
#define IGMP_DEBUG                      0x0700U
#define INET_DEBUG                      0x0800U
#define IP_DEBUG                        0x0900U
#define IP_REASS_DEBUG                  0x0a00U
#define RAW_DEBUG                       0x0b00U
#define MEM_DEBUG                       0x0c00U
#define MEMP_DEBUG                      0x0d00U
#define SYS_DEBUG                       0x0e00U
#define TIMERS_DEBUG                    0x0f00U
#define TCP_DEBUG                       0x1000U
#define TCP_INPUT_DEBUG                 0x1100U
#define TCP_FR_DEBUG                    0x1200U
#define TCP_RTO_DEBUG                   0x1300U
#define TCP_CWND_DEBUG                  0x1400U
#define TCP_WND_DEBUG                   0x1500U
#define TCP_OUTPUT_DEBUG                0x1600U
#define TCP_RST_DEBUG                   0x1700U
#define TCP_QLEN_DEBUG                  0x1800U
#define UDP_DEBUG                       0x1900U
#define TCPIP_DEBUG                     0x1a00U
#define PPP_DEBUG                       0x1b00U
#define SLIP_DEBUG                      0x1c00U
#define DHCP_DEBUG                      0x1d00U
#define AUTOIP_DEBUG                    0x1e00U
#define SNMP_MSG_DEBUG                  0x1f00U
#define SNMP_MIB_DEBUG                  0x2000U
#define DNS_DEBUG                       0x2100U
#endif

#endif /* __LWIPOPTS_H__ */
