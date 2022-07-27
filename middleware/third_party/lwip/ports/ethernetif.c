/**
 * @file
 * Ethernet Interface Skeleton
 *
 */

/*
 * Copyright (c) 2001-2004 Swedish Institute of Computer Science.
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

/*
 * This file is a skeleton for developing Ethernet network interface
 * drivers for lwIP. Add code to the low_level functions and do a
 * search-and-replace for the word "ethernetif" to replace it with
 * something that better describes your network interface.
 */
#include <stdio.h>
#include <string.h>

#include "lwip/opt.h"
#include <stdint.h>
#include "lwip/def.h"
#include "lwip/mem.h"
#include "lwip/pbuf.h"
#include "lwip/sys.h"
#include <lwip/stats.h>
#include <lwip/snmp.h>
#include <lwip/sockets.h>
#include <lwip/tcpip.h>
#include <lwip/dhcp.h>
#include "netif/etharp.h"
//#include "netif/ppp_oe.h"   //2015-7-21 bing.luo delete on@132599
#include "ethernetif.h"
#include "net_task.h"
#include "type_def.h"
#include "connsys_util.h"
#include "connsys_driver.h"
#include "inband_queue.h"
#include "mt_cmd_fmt.h"
#include "lwip/ethip6.h"
#if defined(MTK_HAL_LOWPOWER_ENABLE)
#include "hal_lp.h"
#endif
#include "syslog.h"
#include "wifi_scan.h"
#include "wifi_private_api.h"
#if (CFG_WIFI_HIF_GDMA_EN == 1)
#include "hal_gdma.h"
#endif
#include "lwip/netif.h"
#include "os_util.h"

#ifdef MTK_WIFI_ROM_ENABLE
#include "mlme_interface.h"
#endif

#if !defined(TCPIP_FOR_MT5931_ENABLE) && !defined(TCPIP_FOR_MT5932_ENABLE) && !defined(MTK_TCPIP_FOR_EXTERNAL_MODULE_ENABLE)
#include "wifi_api.h"
#endif

int32_t pkt_lock_init(struct os_time *timeout_value);


#ifdef DATA_PATH_87
extern unsigned char IoT_Init_Done;

void wifi_firmware_rxdata_msg_send(void *pbuffer);

#endif/*DATA_PATH_87*/
/* Sanity check the configuration. */
#define ALIGN_4BYTE(size)       (((size+3)/4) * 4)

#ifndef IS_ALIGN_4
#define IS_ALIGN_4(_value)      (((_value) & 0x3) ? FALSE : TRUE)
#define IS_NOT_ALIGN_4(_value)  (((_value) & 0x3) ? TRUE : FALSE)
#endif /* IS_ALIGN_4 */

#ifndef IS_NOT_ALIGN_4
#define IS_NOT_ALIGN_4(_value)  (((_value) & 0x3) ? TRUE : FALSE)
#endif /* IS_NOT_ALIGN_4 */



#if defined(MTK_BSP_LOOPBACK_ENABLE)
#include "lwip/debug.h"
unsigned int g_loopback_start = 0;
unsigned int loopback_start() { return g_loopback_start;}
void loopback_start_set(unsigned int start) { g_loopback_start = start; }
#endif // MTK_BSP_LOOPBACK_ENABLE

/* Define those to better describe your network interface. Note: only 2-byte allowed */
#define IFNAME00 's'
#define IFNAME01 't'
#define IFNAME10 'a'
#define IFNAME11 'p'
#define IFNAME20 'l'
#define IFNAME21 'o'

static int g_lwip_eapol_rx_socket = -1;
static int g_lwip_eapol_rx_socket_second = -1;

struct netif sta_if, ap_if;

#if AIROHA_LWIP_COM_NETIF
    struct netif com_if;

    // In order to prevent conflict with STA/AP Netif Addr,
    // Use ClassA IPv4 Addr, not A/B/C Private IP Address
    #define COM_NETIF_IP_ADDR       "8.0.0.2"
    #define COM_NETIF_GW_ADDR       "8.0.0.1"
    #define COM_NETIF_MASK_ADDR     "255.0.0.0"
    ip4_addr_t com_if_ip_addr;
    ip4_addr_t com_if_gw_addr;
    ip4_addr_t com_if_mask_addr;

    const u8_t COM_NETIF_LOCAL_MAC[6] = {0x76, 0x82, 'C', 'O', 'M', 'P'};
    const u8_t COM_NETIF_REMOTE_MAC[6] = {0x63, 0x66, 'C', 'O', 'M', 'P'};

    #define COM_NETIF_QUEUE_SIZE                16
    #define COM_NETIF_QUEUE_TAXK_TX_NAME        "com_tx"
    #define COM_NETIF_QUEUE_TAXK_RX_NAME        "com_rx"
    #define COM_NETIF_QUEUE_TASK_STACKSIZE      (1024 * 2)
    // The Queue_Task priority should higher than LwIP TCPIP Thread (TASK_PRIORITY_HIGH)
    #define COM_NETIF_QUEUE_TASK_PRIO           TASK_PRIORITY_SOFT_REALTIME

    QueueHandle_t g_com_pbuf_tx_queue;
    QueueHandle_t g_com_pbuf_rx_queue;
    com_netif_send_cb g_com_send_cb;
#endif



/**
 * Helper struct to hold private data used to operate your ethernet interface.
 * Keeping the ethernet address of the MAC in this struct is not necessary
 * as it is already kept in the struct netif.
 * But this is only an example, anyway...
 */
struct ethernetif {
  struct eth_addr *ethaddr;
  /* Add whatever per-interface state that is needed here. */
};

/* Forward declarations. */
void  ethernetif_input(struct netif *netif, int port);
void ethernetif_intr_enhance_mode_allocate_pkt_callback(
                            unsigned int allocate_len,
                            void** pkt_ptr,
                            unsigned char** payload_ptr);
void ethernetif_free_pkt(void *pkt_ptr);
uint8_t enqueue_bottom_half_from_isr(int32_t port, PNETFUNC func);

void inband_handle_func(void* pkt_ptr, unsigned char *payload, unsigned int len);

#if defined(MTK_WIFI_STUB_CONF_ENABLE)
uint8_t wfc_send_to_host(uint8_t *pbuf, uint32_t bufSize, uint8_t isEvt);
#endif

#if 0
static void InterruptSimulator( void *pvParameters )
{
   /*
    *  There is no real way of simulating an interrupt.
    *  Make sure other tasks can run.
    */

   do
   {
      vTaskDelay(3*configTICK_RATE_HZ);
   }
   while (1);
}
#endif

void
ethernetif_intr_enhance_mode_dispatch(struct pbuf *p, struct netif *netif);

void enqueue_rx_pkt_lwip(struct pbuf *p, struct netif *netif)
{
ethernetif_intr_enhance_mode_dispatch(p, netif);
   //NetJobAddFromISR((PNETFUNC)ethernetif_intr_enhance_mode_dispatch, (uint32_t) p, (uint32_t)netif);
}

void register_eapol_rx_socket(int eapol_rx_socket)
{
	if (eapol_rx_socket >= 0)
	g_lwip_eapol_rx_socket = eapol_rx_socket;
	g_lwip_eapol_rx_socket_second = -1;
}

void unregister_eapol_rx_socket()
{
    if (g_lwip_eapol_rx_socket >= 0)
        close(g_lwip_eapol_rx_socket);
    g_lwip_eapol_rx_socket = -1;
}


void register_eapol_rx_socket_dual_intf(int eapol_rx_socket, int eapol_rx_socket_second)
{
    if (eapol_rx_socket >= 0)
        g_lwip_eapol_rx_socket = eapol_rx_socket;

    if (eapol_rx_socket_second >= 0)
        g_lwip_eapol_rx_socket_second = eapol_rx_socket_second;
}

void unregister_eapol_rx_socket_dual_intf(int eapol_rx_socket, int eapol_rx_socket_second)
{
    if (eapol_rx_socket >= 0)
        close(eapol_rx_socket);
    g_lwip_eapol_rx_socket = -1;

    if (eapol_rx_socket_second >= 0)
        close(eapol_rx_socket_second);
    g_lwip_eapol_rx_socket_second = -1;
}


void low_level_set_mac_addr(struct netif *netif, uint8_t *mac_addr)
{
    /* set MAC hardware address length */
    netif->hwaddr_len = ETHARP_HWADDR_LEN;

    /* set MAC hardware address */
    memcpy(netif->hwaddr, mac_addr, ETHARP_HWADDR_LEN);
}

/**
 * Find a network interface by searching for its name
 *
 * @param name the name of the netif (like netif->name) NOT plus concatenated number
 * in ascii representation (e.g. 'en0')
 */
struct netif *
netif_find_name(char *name)
{
  struct netif *netif;

  if (name == NULL) {
    return NULL;
  }

  for(netif = netif_list; netif != NULL; netif = netif->next) {
    if (name[0] == netif->name[0] &&
       name[1] == netif->name[1]) {
      LWIP_DEBUGF(NETIF_DEBUG, ("netif_find: found %c%c\n", name[0], name[1]));
      return netif;
    }
  }
  LWIP_DEBUGF(NETIF_DEBUG, ("netif_find: didn't find %c%c\n", name[0], name[1]));
  return NULL;
}



int low_level_get_sta_mac_addr(unsigned char *mac_addr)
{
    char name[3];
    struct netif *netif = NULL;

    name[0] = IFNAME00;
    name[1] = IFNAME01;

    netif = netif_find_name(name);
    if (netif)
    {
        memcpy(mac_addr, netif->hwaddr, ETHARP_HWADDR_LEN);
    }
    else
    {
        return -1;
    }

    return 0;
}

int low_level_get_ap_mac_addr(unsigned char *mac_addr)
{
    char name[3];
    struct netif *netif = NULL;

    name[0] = IFNAME10;
    name[1] = IFNAME11;

    netif = netif_find_name(name);
    if (netif)
    {
        memcpy(mac_addr, netif->hwaddr, ETHARP_HWADDR_LEN);
    }
    else
    {
        return -1;
    }

    return 0;
}



/**
 * In this function, the hardware should be initialized.
 * Called from ethernetif_init().
 *
 * @param netif the already initialized lwip network interface structure
 *        for this ethernetif
 */
static void
low_level_init1(struct netif *netif)
{
#if 0 // init MAC address in upper layer
  /* set MAC hardware address length */
  netif->hwaddr_len = ETHARP_HWADDR_LEN;

  /* set MAC hardware address */
  netif->hwaddr[0] = 0x00;
  netif->hwaddr[1] = 0x0c;
  netif->hwaddr[2] = 0x43;
  netif->hwaddr[3] = 0x76;
  netif->hwaddr[4] = 0x62;
  netif->hwaddr[5] = 0x02;
#endif

  /* maximum transfer unit */
  netif->mtu = 1500;

  /* device capabilities */
  /* don't set NETIF_FLAG_ETHARP if this device is not an ethernet one */
  netif->flags = NETIF_FLAG_IGMP | NETIF_FLAG_BROADCAST | NETIF_FLAG_ETHARP;

  /* Do whatever else is needed to initialize interface. */
}

static void
low_level_init2(struct netif *netif)
{
#if 0 // init MAC address in upper layer
  /* set MAC hardware address length */
  netif->hwaddr_len = ETHARP_HWADDR_LEN;

  /* set MAC hardware address */
  netif->hwaddr[0] = 0x00;
  netif->hwaddr[1] = 0x0c;
  netif->hwaddr[2] = 0x43;
  netif->hwaddr[3] = 0x76;
  netif->hwaddr[4] = 0x62;
  netif->hwaddr[5] = 0x04;
#endif

  /* maximum transfer unit */
  netif->mtu = 1500;

  /* device capabilities */
  /* don't set NETIF_FLAG_ETHARP if this device is not an ethernet one */
  netif->flags = NETIF_FLAG_IGMP | NETIF_FLAG_BROADCAST | NETIF_FLAG_ETHARP | NETIF_FLAG_LINK_UP;

  /* Do whatever else is needed to initialize interface. */
}


/**
 * This function should do the actual transmission of the packet. The packet is
 * contained in the pbuf that is passed to the function. This pbuf
 * might be chained.
 *
 * @param netif the lwip network interface structure for this ethernetif
 * @param p the MAC packet to send (e.g. IP packet including MAC addresses and type)
 * @return ERR_OK if the packet could be sent
 *         an err_t value if the packet couldn't be sent
 *
 * @note Returning ERR_MEM here if a DMA queue of your MAC is full can lead to
 *       strange results. You might consider waiting for space in the DMA queue
 *       to become availale since the stack doesn't retry to send a packet
 *       dropped because of memory failure (except for the TCP timers).
 */
#if 0
static int low_level_output(struct netif *netif, struct pbuf *p)
{
    struct pbuf *q;
    u32_t total_len = 0;
    int ret = ERR_OK;
    int connsys_ret = CONNSYS_STATUS_SUCCESS;
    u32_t pbuf_num = 0;
    unsigned char *pBuffer = NULL;
    unsigned char *pBuffer_4byte_align = NULL;
    unsigned char *pTmp = NULL;
    taskENTER_CRITICAL();


    for(q = p; q != NULL; q = q->next) {
        total_len = total_len + (q->len);
        pbuf_num = pbuf_num + 1;
        DBG_CONNSYS(CONNSYS_DBG_TX_Q1,("==>low_level_output, q->len = %u\n",
            (unsigned int)q->len));
    }
    DBG_CONNSYS(CONNSYS_DBG_TX_Q1,("==>low_level_output, pbuf_num = %u, total_len = %u\n",
        (unsigned int)pbuf_num, (unsigned int)total_len));

    if (pbuf_num > 1)
    {
        /* concate pbuf_chain into single buffer */
        u32_t allocate_len = total_len +
            CFG_CONNSYS_IOT_TX_ZERO_COPY_PAD_LEN +
            WIFI_HIF_TX_BYTE_CNT_LEN;

        u32_t payload_offset = sizeof(INIT_HIF_TX_HEADER_T);

#if (CFG_CONNSYS_IOT_TX_ZERO_COPY_EN == 1)
        payload_offset += CFG_CONNSYS_IOT_TX_ZERO_COPY_PAD_LEN;
#endif

        if (IS_NOT_ALIGN_4(total_len))
            allocate_len += 4;

        pBuffer = os_malloc(allocate_len);
        if (NULL == pBuffer)
        {
            LWIP_DEBUGF(NETIF_DEBUG, ("ethernetif_init: out of memory\n"));
            DBG_CONNSYS(CONNSYS_DBG_TX_Q1,("==>low_level_output, out of memory\n"));
            taskEXIT_CRITICAL();
            return ERR_MEM;
        }
        pBuffer_4byte_align = (unsigned char *)((unsigned int)pBuffer & (~0x3UL));
        pTmp = pBuffer_4byte_align + payload_offset;

        for(q = p; q != NULL; q = q->next) {
            memcpy(pTmp, q->payload, q->len);
            pTmp += (q->len);
        }

        if (netif == &ap_if)
        {
            connsys_ret = connsys_util_low_level_output(pBuffer_4byte_align, total_len, IOT_PACKET_TYPE_INF_1_IDX);
        }
        else
        {
            connsys_ret = connsys_util_low_level_output(pBuffer_4byte_align, total_len, IOT_PACKET_TYPE_INF_0_IDX);
        }
        os_free(pBuffer);
    }
    else
    {
#if CFG_CONNSYS_TXD_PAD_SIZE
        pbuf_header(p, CFG_CONNSYS_TXD_PAD_SIZE);
#endif

        if (netif == &ap_if)
        {
            connsys_ret = connsys_util_low_level_output(p->payload, total_len, IOT_PACKET_TYPE_INF_1_IDX);
        }
        else
        {
            connsys_ret = connsys_util_low_level_output(p->payload, total_len, IOT_PACKET_TYPE_INF_0_IDX);
        }
    }

    if (connsys_ret == CONNSYS_STATUS_FAIL)
        ret = ERR_BUF;

  LINK_STATS_INC(link.xmit);
  taskEXIT_CRITICAL();
  return ret;
}
#endif
extern unsigned char IoT_Init_Done;
UCHAR send_txdata_to_wifi_thread(void *pbuffer, struct netif * netif);

#if defined(MTK_WIFI_STUB_CONF_ENABLE) && (MTK_WIFI_STUB_CONF_SPIM_ENABLE || MTK_WIFI_STUB_CONF_SDIO_MSDC_ENABLE)

static int32_t low_level_output_scatter(struct netif *netif, struct pbuf *p)
{
    struct pbuf *q;
    uint32_t total_len = 0;

    for(q = p; q != NULL; q = q->next) {
        total_len = total_len + (q->len);
    }
    //printf("S(%u) -> Host lwip\r\n",total_len);
    wfc_send_to_host((void *)p, total_len, 0);

    return CONNSYS_STATUS_SUCCESS;
}

#else

static int32_t low_level_output_scatter(struct netif *netif, struct pbuf *p)
{
    P_HIF_TX_HEADER_PORT1_T p_txd;
    int32_t ret = CONNSYS_STATUS_SUCCESS;
    int connsys_ret = CONNSYS_STATUS_SUCCESS;
    struct pbuf *q;
    int32_t inf_num;
    uint32_t total_len = 0;
    connsys_tx_scatter_info_t tx_info;
    signed int flow_control_status;
#if (CONNSYS_DEBUG_MODE_EN == 1)
    static uint32_t tx_sequence = 0;
#endif

#ifdef MTK_CM4_WIFI_TASK_ENABLE
    if(IoT_Init_Done)
    {
#ifdef MT5932_SINGLE_CONTEXT
	    pbuf_ref(p);
#endif

#ifdef MTK_WIFI_REPEATER_ENABLE

        int    i = 0;
        struct eth_hdr *ethhdr;
        struct etharp_hdr *hdr;

        int      sndAllInf = 0;
        unsigned char op_mode = WIFI_MODE_STA_ONLY;

        ethhdr = (struct eth_hdr *)p->payload;

        fw_mlme_get_opmode(&op_mode);
        if (op_mode == WIFI_MODE_REPEATER)
        {
           sndAllInf = (memcmp(&(ethhdr->dest.addr), "\xff\xff\xff\xff\xff\xff", 6) == 0)?1:0;
           if(sndAllInf == 1)
                pbuf_ref(p);
        }

        for (i=0; i<=sndAllInf; i++)
        {
            if (op_mode == WIFI_MODE_REPEATER)
            {
                if ( sndAllInf && (i==sndAllInf) )
                {
                    if (   (memcmp(&ethhdr->src, (struct eth_addr*)(sta_if.hwaddr), 6)==0)
                        || (memcmp(&ethhdr->src, (struct eth_addr*)(ap_if.hwaddr), 6)==0)
                       )
                    {
                       //LOG_I(lwip, "src - %2x:%2x:%2x:%2x:%2x:%2x, dest - %2x:%2x:%2x:%2x:%2x:%2x, sndAllInf(%d)\n", ethhdr->src.addr[0], ethhdr->src.addr[1], ethhdr->src.addr[2], ethhdr->src.addr[3], ethhdr->src.addr[4], ethhdr->src.addr[5], ethhdr->dest.addr[0], ethhdr->dest.addr[1], ethhdr->dest.addr[2], ethhdr->dest.addr[3], ethhdr->dest.addr[4], ethhdr->dest.addr[5], sndAllInf);
                        netif = (netif == &ap_if)?&sta_if:&ap_if;
                        /**
                         * Airoha Change
                         * ETHADDR16_COPY macro has been removed
                         * Original implementation : #define ETHADDR16_COPY(dst, src)  SMEMCPY(dst, src, ETH_HWADDR_LEN)
                         * Use memcpy to replace
                         */
                        //ETHADDR16_COPY(&ethhdr->src, (struct eth_addr*)(netif->hwaddr));
                        memcpy(&ethhdr->src, (struct eth_addr*)(netif->hwaddr), ETH_HWADDR_LEN);
                        /** Airoha Change End */
                        if ( ethhdr->type == PP_HTONS(ETHTYPE_ARP) )
                        {
                           hdr = (struct etharp_hdr *)((u8_t*)ethhdr + SIZEOF_ETH_HDR);
                           #if ETHARP_SUPPORT_VLAN
                             if (ethhdr->type == PP_HTONS(ETHTYPE_VLAN)) {
                               hdr = (struct etharp_hdr *)(((u8_t*)ethhdr) + SIZEOF_ETH_HDR + SIZEOF_VLAN_HDR);
                             }
                           #endif /* ETHARP_SUPPORT_VLAN */
                           /**
                            * Airoha Change
                            * Use memcpy to replace
                            */
                           //ETHADDR16_COPY(&(hdr->shwaddr), (struct eth_addr*)(netif->hwaddr));
                           memcpy(&(hdr->shwaddr), (struct eth_addr*)(netif->hwaddr), ETH_HWADDR_LEN);
                           /** Airoha Change End */
                        }
                    }
                }
                else if ( fw_mlme_find_connected_sta_by_mac(ethhdr->dest.addr))
                {
                    netif = &ap_if;
                    if (memcmp(&ethhdr->src, (struct eth_addr*)(sta_if.hwaddr), 6)==0)
                    {
                        /**
                         * Airoha Change
                         * Use memcpy to replace
                         */
                        //ETHADDR16_COPY(&ethhdr->src, (struct eth_addr*)(netif->hwaddr));
                        memcpy(&ethhdr->src, (struct eth_addr*)(netif->hwaddr), ETH_HWADDR_LEN);
                        /** Airoha Change End */
                    }
                    //LOG_I(lwip,"Change netif to ap_if : src - %2x:%2x:%2x:%2x:%2x:%2x, dest - %2x:%2x:%2x:%2x:%2x:%2x\n", ethhdr->src.addr[0], ethhdr->src.addr[1], ethhdr->src.addr[2], ethhdr->src.addr[3], ethhdr->src.addr[4], ethhdr->src.addr[5],ethhdr->dest.addr[0], ethhdr->dest.addr[1], ethhdr->dest.addr[2], ethhdr->dest.addr[3], ethhdr->dest.addr[4], ethhdr->dest.addr[5]);
                }
            }
#endif
            if(!send_txdata_to_wifi_thread((void *)p, netif))
				return CONNSYS_STATUS_FAIL;
#ifdef MTK_WIFI_REPEATER_ENABLE
        }
#endif
        return CONNSYS_STATUS_SUCCESS;
    }
    else
    {
#ifndef MT5932_SINGLE_CONTEXT
        pbuf_free(p);
        p = NULL;
#else
	return CONNSYS_STATUS_SUCCESS;
#endif
    }
#endif

#ifdef MTK_MINISUPP_ENABLE
#ifdef MTK_WIFI_REPEATER_ENABLE
    int    i = 0;
    struct eth_hdr *ethhdr;
    struct etharp_hdr *hdr;

    int      sndAllInf = 0;
    unsigned char op_mode = WIFI_MODE_STA_ONLY;

    ethhdr = (struct eth_hdr *)p->payload;
    // Check if OpMode is Repeater Mode. Broadcast to all interfaces only at Repeater Mode.
    if (__g_wpa_supplicant_api.wpa_supplicant_entry_op_mode_get)
    {
        __g_wpa_supplicant_api.wpa_supplicant_entry_op_mode_get(&op_mode);
        if (op_mode == WIFI_MODE_REPEATER)
        {
           sndAllInf = (memcmp(&(ethhdr->dest.addr), "\xff\xff\xff\xff\xff\xff", 6) == 0)?1:0;
        }
    }

    for (i=0; i<=sndAllInf; i++)
    {
        total_len = 0;
        // Change Source Mac for Ether/Arp packets if it is need
        if (op_mode == WIFI_MODE_REPEATER)
        {
            if ( sndAllInf && (i==sndAllInf) )
            {
                if (   (memcmp(&ethhdr->src, (struct eth_addr*)(sta_if.hwaddr), 6)==0)
                    || (memcmp(&ethhdr->src, (struct eth_addr*)(ap_if.hwaddr), 6)==0)
                   )
                {
                   //LOG_I(lwip, "src - %2x:%2x:%2x:%2x:%2x:%2x, dest - %2x:%2x:%2x:%2x:%2x:%2x, sndAllInf(%d)\n", ethhdr->src.addr[0], ethhdr->src.addr[1], ethhdr->src.addr[2], ethhdr->src.addr[3], ethhdr->src.addr[4], ethhdr->src.addr[5], ethhdr->dest.addr[0], ethhdr->dest.addr[1], ethhdr->dest.addr[2], ethhdr->dest.addr[3], ethhdr->dest.addr[4], ethhdr->dest.addr[5], sndAllInf);
                    netif = (netif == &ap_if)?&sta_if:&ap_if;
                    /**
                     * Airoha Change
                     * Use memcpy to replace
                     */
                    //ETHADDR16_COPY(&ethhdr->src, (struct eth_addr*)(netif->hwaddr));
                    memcpy(&ethhdr->src, (struct eth_addr*)(netif->hwaddr), ETH_HWADDR_LEN);
                    /** Airoha Change End */
                    if ( ethhdr->type == PP_HTONS(ETHTYPE_ARP) )
                   {
                       hdr = (struct etharp_hdr *)((u8_t*)ethhdr + SIZEOF_ETH_HDR);
                       #if ETHARP_SUPPORT_VLAN
                         if (ethhdr->type == PP_HTONS(ETHTYPE_VLAN)) {
                           hdr = (struct etharp_hdr *)(((u8_t*)ethhdr) + SIZEOF_ETH_HDR + SIZEOF_VLAN_HDR);
                         }
                       #endif /* ETHARP_SUPPORT_VLAN */
                       /**
                        * Airoha Change
                        * Use memcpy to replace
                        */
                       //ETHADDR16_COPY(&(hdr->shwaddr), (struct eth_addr*)(netif->hwaddr));
                       memcpy(&(hdr->shwaddr), (struct eth_addr*)(netif->hwaddr), ETH_HWADDR_LEN);
                       /** Airoha Change End */
                   }
                }
            }
            // Check if this remote STA mac belong to this AP
            else if ( (__g_wpa_supplicant_api.get_ap_sta) && (__g_wpa_supplicant_api.get_ap_sta((char *)&(ethhdr->dest.addr))) )
            {
                netif = &ap_if;
                if (memcmp(&ethhdr->src, (struct eth_addr*)(sta_if.hwaddr), 6)==0)
                {
                    /**
                     * Airoha Change
                     * Use memcpy to replace
                     */
                    //ETHADDR16_COPY(&ethhdr->src, (struct eth_addr*)(netif->hwaddr));
                    memcpy(&ethhdr->src, (struct eth_addr*)(netif->hwaddr), ETH_HWADDR_LEN);
                    /** Airoha Change End */
                }
                //LOG_I(lwip,"Change netif to ap_if : src - %2x:%2x:%2x:%2x:%2x:%2x, dest - %2x:%2x:%2x:%2x:%2x:%2x\n", ethhdr->src.addr[0], ethhdr->src.addr[1], ethhdr->src.addr[2], ethhdr->src.addr[3], ethhdr->src.addr[4], ethhdr->src.addr[5],ethhdr->dest.addr[0], ethhdr->dest.addr[1], ethhdr->dest.addr[2], ethhdr->dest.addr[3], ethhdr->dest.addr[4], ethhdr->dest.addr[5]);
            }
        }
#endif
#endif
#if (CONNSYS_DEBUG_MODE_EN == 1)
    //if (CONNSYS_TEST_DEBUG(CONNSYS_DBG_TX_Q1))
    //{
    //    LOG_I(lwip, "== TX Start ==  tx_sequence: %u\n", (unsigned int)tx_sequence);
    //}
    tx_sequence++;
#endif

    memset(&tx_info, 0, sizeof(connsys_tx_scatter_info_t));
    if (netif == &ap_if)
    {
        inf_num = IOT_PACKET_TYPE_INF_1_IDX;
    }
    else
    {
        inf_num = IOT_PACKET_TYPE_INF_0_IDX;
    }
    //LOG_I(lwip, "sendto inf_num(%d)...%s-%d\n",inf_num,__FUNCTION__,__LINE__);
    for(q = p; q != NULL; q = q->next) {
        if (tx_info.buf_num >= MAX_TX_BUF)
        {
            LOG_E(lwip, "ERROR! ==> low_level_output_scatter, buf_num exceed MAX_TX_BUF\n");
            LOG_E(lwip, "tx_info.buf_num = %u, MAX_TX_BUF = %u\n",
                (unsigned int)tx_info.buf_num,
                (unsigned int)MAX_TX_BUF);
            connsys_dump_tx_scatter_info(&tx_info);
            return ERR_BUF;
        }

        total_len = total_len + (q->len);
        tx_info.buff[tx_info.buf_num].buf_ptr = q->payload;
        tx_info.buff[tx_info.buf_num].buf_len= q->len;

#if (CONNSYS_DEBUG_MODE_EN == 1)
        DBG_CONNSYS(CONNSYS_DBG_TX_Q1,("pbuf chain len[%u]: %u\n",
            (unsigned int)tx_info.buf_num,
            (unsigned int)q->len));
#endif

        tx_info.buf_num ++;
    }
#if (CFG_CONNSYS_IOT_TX_ZERO_COPY_EN == 1)
#ifdef MTK_MINISUPP_ENABLE
    if (__g_wpa_supplicant_api.get_sta_qos_bit) {
        tx_info.fg_wmm = __g_wpa_supplicant_api.get_sta_qos_bit(inf_num, (char *)p->payload);
    }
#endif    //Castro+
    if (tx_info.fg_wmm == 1)
        tx_info.reserved_headroom_len = connsys_get_headroom_offset(CONNSYS_HEADROOM_OFFSET_QOS); //EXTRA_HEADROOM_LEN_FOR_QOS_ENABLE;
    else
        tx_info.reserved_headroom_len = connsys_get_headroom_offset(CONNSYS_HEADROOM_OFFSET_NON_QOS); // EXTRA_HEADROOM_LEN_FOR_NON_QOS;
//#endif //Castro-
#else
    tx_info.reserved_headroom_len = 0;
#endif
    tx_info.tx_len = sizeof(INIT_HIF_TX_HEADER_T)
                        + tx_info.reserved_headroom_len
                        + total_len;

    tx_info.real_tx_len = ALIGN_4BYTE(tx_info.tx_len);

    p_txd = (P_HIF_TX_HEADER_PORT1_T)&tx_info.txd;
    p_txd->u2TxByteCount = tx_info.tx_len;
    p_txd->u2PQ_ID = P1_Q1;
    p_txd->ucPktTypeID = PKT_ID_CMD;
    p_txd->ucPacketOffset = tx_info.reserved_headroom_len;

    if (inf_num == IOT_PACKET_TYPE_INF_1_IDX)
        p_txd->ucReserved = IOT_PACKET_TYPE_INF_1;
    else
        p_txd->ucReserved = IOT_PACKET_TYPE_INF_0;


    if (CONNSYS_TEST_DEBUG(CONNSYS_DBG_TX_Q1))
    {
        connsys_dump_tx_scatter_info(&tx_info);
    }

#if (CFG_CONNSYS_TRX_BALANCE_EN == 1)
    if (g_balance_ctr.balance_en == TRUE)
    {
        if (connsys_balance_check_tx_yield())
        {
            vTaskPrioritySet(NULL, (TCPIP_THREAD_PRIO - 1));
            vTaskPrioritySet((TaskHandle_t)g_balance_ctr.rx_handle, TCPIP_THREAD_PRIO);
        }
        else
        {
            vTaskPrioritySet(NULL, TCPIP_THREAD_PRIO);
            vTaskPrioritySet((TaskHandle_t)g_balance_ctr.rx_handle, (TCPIP_THREAD_PRIO - 1));
        }
    }
#endif

    flow_control_status =
        connsys_tx_flow_control_check_and_update_tx(WIFI_HIF_TX_PORT_IDX, tx_info.real_tx_len);

    if (flow_control_status == CONNSYS_STATUS_SUCCESS)
    {
        connsys_ret = connsys_write_data(&tx_info);
    }
    else
    {
        int query_limit = 1;
        while(query_limit--){
            connsys_tx_query_whisr();
            flow_control_status =
                connsys_tx_flow_control_check_and_update_tx(WIFI_HIF_TX_PORT_IDX, tx_info.real_tx_len);
            if(flow_control_status == CONNSYS_STATUS_SUCCESS)
                break;
        }

        if(CONNSYS_STATUS_SUCCESS == flow_control_status){
            connsys_ret = connsys_write_data(&tx_info);
        }else{
            connsys_ret = CONNSYS_STATUS_FAIL;
        }
    }

    if (connsys_ret == CONNSYS_STATUS_FAIL)
        ret = ERR_BUF;
    else
        LINK_STATS_INC(link.xmit);

#if (CFG_CONNSYS_TRX_BALANCE_EN == 1)
    connsys_balance_clear_tx_wait();
#endif

#if (CONNSYS_DEBUG_MODE_EN == 1)
    DBG_CONNSYS(CONNSYS_DBG_TX_Q1,("== TX End ==\n"));
#endif

#ifdef MTK_MINISUPP_ENABLE
#ifdef MTK_WIFI_REPEATER_ENABLE
}
#endif
#endif
    return ret;
}
#endif

void ethernetif_intr_enhance_mode_allocate_pkt_callback(
                            unsigned int allocate_len,
                            void** pkt_ptr,
                            unsigned char** payload_ptr);
#if 0
typedef void (*connsys_alloc_pkt_t)(uint32_t allocate_len,
                                    void **pkt_ptr,
                                    uint8_t **payload_ptr);
#endif
uint8_t lwip_advance_pkt_hdr(void *pkt, int16_t len)
{
    return pbuf_header((struct pbuf *)pkt, -(s16_t)len);
}

struct mt76x7_connsys_ops lwip_connsys_ops = {
    .enqueue_bottom_half    = &enqueue_bottom_half_from_isr,
    .alloc                  = (connsys_alloc_pkt_t) &ethernetif_intr_enhance_mode_allocate_pkt_callback,
    .free                   = &ethernetif_free_pkt,
    .advance_pkt_hdr        = &lwip_advance_pkt_hdr,
    .deliver_inband         = &connsys_deliver_inband,
    .deliver_tcpip          = &lwip_deliver_tcpip,
};

void ethernetif_init_callback(void)
{
    inband_queue_register_callback(inband_handle_func);
    connsys_ops = &lwip_connsys_ops;
    connsys_enable_interrupt();
    //pkt_lock_init(NULL);
}

/**
 * Airoha Change - Feature
 * Set MLD6 flag
 * Set the IPv6 link-local address
 * Set the IPv6 auto configure to be enabled
 * */
#if LWIP_IPV6
static void ip6_netif_configure(struct netif *netif)
{
    /**
     * Enable IPv6 MLD (Multicast Listener Discovery Protocol)
     * */
#if LWIP_IPV6_MLD
    netif->flags |= NETIF_FLAG_MLD6;
#endif

    netif_create_ip6_linklocal_address(netif, 1);

#if LWIP_IPV6_AUTOCONFIG
    netif_set_ip6_autoconfig_enabled(netif, 1);
#endif /** LWIP_IPV6_AUTOCONFIG */
    ip6_addr_t mld_address;
    ip6_addr_set_solicitednode(&mld_address, netif_ip6_addr(netif, 0)->addr[3]);
#if LWIP_IPV6_MLD
#include "mld6.h"
    int ret = mld6_joingroup(netif_ip6_addr(netif, 0), &mld_address);
    LWIP_DEBUGF(NETIF_DEBUG, ("netif configure, join group result : %d\n", ret));
#endif
}
#endif /** LWIP_IPV6 */
/** Airoha Change End */

/**
 * Should be called at the beginning of the program to set up the
 * network interface. It calls the function low_level_init() to do the
 * actual setup of the hardware.
 *
 * This function should be passed as a parameter to netif_add().
 *
 * @param netif the lwip network interface structure for this ethernetif
 * @return ERR_OK if the loopif is initialized
 *         ERR_MEM if private data couldn't be allocated
 *         any other err_t on error
 */
err_t
ethernetif_init1(struct netif *netif)
{
  struct ethernetif *ethernetif;

  LWIP_ASSERT("netif != NULL", (netif != NULL));

  ethernetif = mem_malloc(sizeof(struct ethernetif));
  if (ethernetif == NULL) {
    LWIP_DEBUGF(NETIF_DEBUG, ("ethernetif_init: out of memory\n"));
    return ERR_MEM;
  }

#if LWIP_NETIF_HOSTNAME
  /* Initialize interface hostname */
  netif->hostname = "lwip";
#endif /* LWIP_NETIF_HOSTNAME */

  /*
   * Initialize the snmp variables and counters inside the struct netif.
   * The last argument should be replaced with your link speed, in units
   * of bits per second.
   */
  NETIF_INIT_SNMP(netif, snmp_ifType_ethernet_csmacd, LINK_SPEED_OF_YOUR_NETIF_IN_BPS);

  netif->state = ethernetif;
  netif->name[0] = IFNAME00;
  netif->name[1] = IFNAME01;
  /* We directly use etharp_output() here to save a function call.
   * You can instead declare your own function an call etharp_output()
   * from it if you have to do some checks before sending (e.g. if link
   * is available...) */
  netif->output = etharp_output;
  #if LWIP_IPV6
  netif->output_ip6 = ethip6_output;
  #endif /* LWIP_IPV6 */
  netif->linkoutput = (netif_linkoutput_fn)low_level_output_scatter;

  ethernetif->ethaddr = (struct eth_addr *)&(netif->hwaddr[0]);

  /* initialize the hardware */
  low_level_init1(netif);

#if 0
  /* Create a task that simulates an interrupt in a real system.  This will
  block waiting for packets, then send a message to the uIP task when data
  is available. */

  LOG_I(lwip, "Create RX task\n\r");
  xTaskCreate(InterruptSimulator, "RX", 400, (void *)netif, 3, NULL );
#endif
  return ERR_OK;
}

err_t
ethernetif_init2(struct netif *netif)
{
  struct ethernetif *ethernetif;

  LWIP_ASSERT("netif != NULL", (netif != NULL));

  ethernetif = mem_malloc(sizeof(struct ethernetif));
  if (ethernetif == NULL) {
    LWIP_DEBUGF(NETIF_DEBUG, ("ethernetif_init: out of memory\n"));
    return ERR_MEM;
  }

#if LWIP_NETIF_HOSTNAME
  /* Initialize interface hostname */
  netif->hostname = "lwip";
#endif /* LWIP_NETIF_HOSTNAME */

  /*
   * Initialize the snmp variables and counters inside the struct netif.
   * The last argument should be replaced with your link speed, in units
   * of bits per second.
   */
  NETIF_INIT_SNMP(netif, snmp_ifType_ethernet_csmacd, LINK_SPEED_OF_YOUR_NETIF_IN_BPS);

  netif->state = ethernetif;
  netif->name[0] = IFNAME10;
  netif->name[1] = IFNAME11;
  /* We directly use etharp_output() here to save a function call.
   * You can instead declare your own function an call etharp_output()
   * from it if you have to do some checks before sending (e.g. if link
   * is available...) */
  netif->output = etharp_output;
  #if LWIP_IPV6
  netif->output_ip6 = ethip6_output;
  #endif /* LWIP_IPV6 */
  netif->linkoutput = (netif_linkoutput_fn)low_level_output_scatter;

  ethernetif->ethaddr = (struct eth_addr *)&(netif->hwaddr[0]);

  /* initialize the hardware */
  low_level_init2(netif);

#if 0
  /* Create a task that simulates an interrupt in a real system.  This will
  block waiting for packets, then send a message to the uIP task when data
  is available. */

  LOG_I(lwip, "Create RX task\n\r");
  xTaskCreate(InterruptSimulator, "RX", 400, (void *)netif, 3, NULL );
#endif
  return ERR_OK;
}

#if AIROHA_LWIP_COM_NETIF

#define IFNAME30 'c'
#define IFNAME31 'p'

static const unsigned int crc32tab[] = {
 0x00000000L, 0x77073096L, 0xee0e612cL, 0x990951baL,
 0x076dc419L, 0x706af48fL, 0xe963a535L, 0x9e6495a3L,
 0x0edb8832L, 0x79dcb8a4L, 0xe0d5e91eL, 0x97d2d988L,
 0x09b64c2bL, 0x7eb17cbdL, 0xe7b82d07L, 0x90bf1d91L,
 0x1db71064L, 0x6ab020f2L, 0xf3b97148L, 0x84be41deL,
 0x1adad47dL, 0x6ddde4ebL, 0xf4d4b551L, 0x83d385c7L,
 0x136c9856L, 0x646ba8c0L, 0xfd62f97aL, 0x8a65c9ecL,
 0x14015c4fL, 0x63066cd9L, 0xfa0f3d63L, 0x8d080df5L,
 0x3b6e20c8L, 0x4c69105eL, 0xd56041e4L, 0xa2677172L,
 0x3c03e4d1L, 0x4b04d447L, 0xd20d85fdL, 0xa50ab56bL,
 0x35b5a8faL, 0x42b2986cL, 0xdbbbc9d6L, 0xacbcf940L,
 0x32d86ce3L, 0x45df5c75L, 0xdcd60dcfL, 0xabd13d59L,
 0x26d930acL, 0x51de003aL, 0xc8d75180L, 0xbfd06116L,
 0x21b4f4b5L, 0x56b3c423L, 0xcfba9599L, 0xb8bda50fL,
 0x2802b89eL, 0x5f058808L, 0xc60cd9b2L, 0xb10be924L,
 0x2f6f7c87L, 0x58684c11L, 0xc1611dabL, 0xb6662d3dL,
 0x76dc4190L, 0x01db7106L, 0x98d220bcL, 0xefd5102aL,
 0x71b18589L, 0x06b6b51fL, 0x9fbfe4a5L, 0xe8b8d433L,
 0x7807c9a2L, 0x0f00f934L, 0x9609a88eL, 0xe10e9818L,
 0x7f6a0dbbL, 0x086d3d2dL, 0x91646c97L, 0xe6635c01L,
 0x6b6b51f4L, 0x1c6c6162L, 0x856530d8L, 0xf262004eL,
 0x6c0695edL, 0x1b01a57bL, 0x8208f4c1L, 0xf50fc457L,
 0x65b0d9c6L, 0x12b7e950L, 0x8bbeb8eaL, 0xfcb9887cL,
 0x62dd1ddfL, 0x15da2d49L, 0x8cd37cf3L, 0xfbd44c65L,
 0x4db26158L, 0x3ab551ceL, 0xa3bc0074L, 0xd4bb30e2L,
 0x4adfa541L, 0x3dd895d7L, 0xa4d1c46dL, 0xd3d6f4fbL,
 0x4369e96aL, 0x346ed9fcL, 0xad678846L, 0xda60b8d0L,
 0x44042d73L, 0x33031de5L, 0xaa0a4c5fL, 0xdd0d7cc9L,
 0x5005713cL, 0x270241aaL, 0xbe0b1010L, 0xc90c2086L,
 0x5768b525L, 0x206f85b3L, 0xb966d409L, 0xce61e49fL,
 0x5edef90eL, 0x29d9c998L, 0xb0d09822L, 0xc7d7a8b4L,
 0x59b33d17L, 0x2eb40d81L, 0xb7bd5c3bL, 0xc0ba6cadL,
 0xedb88320L, 0x9abfb3b6L, 0x03b6e20cL, 0x74b1d29aL,
 0xead54739L, 0x9dd277afL, 0x04db2615L, 0x73dc1683L,
 0xe3630b12L, 0x94643b84L, 0x0d6d6a3eL, 0x7a6a5aa8L,
 0xe40ecf0bL, 0x9309ff9dL, 0x0a00ae27L, 0x7d079eb1L,
 0xf00f9344L, 0x8708a3d2L, 0x1e01f268L, 0x6906c2feL,
 0xf762575dL, 0x806567cbL, 0x196c3671L, 0x6e6b06e7L,
 0xfed41b76L, 0x89d32be0L, 0x10da7a5aL, 0x67dd4accL,
 0xf9b9df6fL, 0x8ebeeff9L, 0x17b7be43L, 0x60b08ed5L,
 0xd6d6a3e8L, 0xa1d1937eL, 0x38d8c2c4L, 0x4fdff252L,
 0xd1bb67f1L, 0xa6bc5767L, 0x3fb506ddL, 0x48b2364bL,
 0xd80d2bdaL, 0xaf0a1b4cL, 0x36034af6L, 0x41047a60L,
 0xdf60efc3L, 0xa867df55L, 0x316e8eefL, 0x4669be79L,
 0xcb61b38cL, 0xbc66831aL, 0x256fd2a0L, 0x5268e236L,
 0xcc0c7795L, 0xbb0b4703L, 0x220216b9L, 0x5505262fL,
 0xc5ba3bbeL, 0xb2bd0b28L, 0x2bb45a92L, 0x5cb36a04L,
 0xc2d7ffa7L, 0xb5d0cf31L, 0x2cd99e8bL, 0x5bdeae1dL,
 0x9b64c2b0L, 0xec63f226L, 0x756aa39cL, 0x026d930aL,
 0x9c0906a9L, 0xeb0e363fL, 0x72076785L, 0x05005713L,
 0x95bf4a82L, 0xe2b87a14L, 0x7bb12baeL, 0x0cb61b38L,
 0x92d28e9bL, 0xe5d5be0dL, 0x7cdcefb7L, 0x0bdbdf21L,
 0x86d3d2d4L, 0xf1d4e242L, 0x68ddb3f8L, 0x1fda836eL,
 0x81be16cdL, 0xf6b9265bL, 0x6fb077e1L, 0x18b74777L,
 0x88085ae6L, 0xff0f6a70L, 0x66063bcaL, 0x11010b5cL,
 0x8f659effL, 0xf862ae69L, 0x616bffd3L, 0x166ccf45L,
 0xa00ae278L, 0xd70dd2eeL, 0x4e048354L, 0x3903b3c2L,
 0xa7672661L, 0xd06016f7L, 0x4969474dL, 0x3e6e77dbL,
 0xaed16a4aL, 0xd9d65adcL, 0x40df0b66L, 0x37d83bf0L,
 0xa9bcae53L, 0xdebb9ec5L, 0x47b2cf7fL, 0x30b5ffe9L,
 0xbdbdf21cL, 0xcabac28aL, 0x53b39330L, 0x24b4a3a6L,
 0xbad03605L, 0xcdd70693L, 0x54de5729L, 0x23d967bfL,
 0xb3667a2eL, 0xc4614ab8L, 0x5d681b02L, 0x2a6f2b94L,
 0xb40bbe37L, 0xc30c8ea1L, 0x5a05df1bL, 0x2d02ef8dL
};

unsigned int com_crc32(const unsigned char *buf, unsigned int size)
{
    unsigned int i, crc;
    crc = 0xFFFFFFFF;
    for (i = 0; i < size; i++) {
        crc = crc32tab[(crc ^ buf[i]) & 0xff] ^ (crc >> 8);
    }
    return crc ^ 0xFFFFFFFF;
}

static void com_netif_tx_queue_task(void *arg)
{
    // COM Send Task, Always Wait g_com_pbuf_tx_queue Ready
    uint32_t payload_addr = 0;
    while (1) {
        if (xQueueReceive(g_com_pbuf_tx_queue, &payload_addr, portMAX_DELAY) != pdPASS) {
            LWIP_DEBUGF(NETIF_DEBUG, ("com_netif_tx_queue_task xQueueReceive fail\n"));
        } else {
            struct pbuf *p = (struct pbuf *)payload_addr;
            if (g_com_send_cb != NULL) {
                uint8_t *payload = p->payload;
                g_com_send_cb(payload, p->tot_len);
                pbuf_free(p);
            }
        }
    }
    vTaskDelete(NULL);
}

static void com_netif_rx_queue_task(void *arg)
{
    // COM Receive Task, Always Wait g_com_pbuf_rx_queue Ready
    while (1) {
        uint32_t payload_addr = 0;
        if (xQueueReceive(g_com_pbuf_rx_queue, &payload_addr, portMAX_DELAY) != pdPASS) {
            LWIP_DEBUGF(NETIF_DEBUG, ("com_netif_rx_queue_task xQueueReceive fail\n"));
        } else {
            struct pbuf *p = (struct pbuf *)payload_addr;
            com_if.input(p, &com_if);
        }
    }
    vTaskDelete(NULL);
}

bool lwip_com_netif_init(com_netif_send_cb send_cb)
{
    if (send_cb == NULL) {
        LWIP_DEBUGF(NETIF_DEBUG, ("NULL com_netif_send_cb\n"));
        return FALSE;
    }
    g_com_send_cb = send_cb;

    g_com_pbuf_tx_queue = xQueueCreate(COM_NETIF_QUEUE_SIZE, sizeof(uint32_t));
    g_com_pbuf_rx_queue = xQueueCreate(COM_NETIF_QUEUE_SIZE, sizeof(uint32_t));
    if (g_com_pbuf_tx_queue == NULL || g_com_pbuf_rx_queue == NULL) {
        LWIP_DEBUGF(NETIF_DEBUG, ("Cannot create g_com_pbuf queue\n"));
        return FALSE;
    }

    if (pdPASS != xTaskCreate(com_netif_tx_queue_task,
                              COM_NETIF_QUEUE_TAXK_TX_NAME,
                              COM_NETIF_QUEUE_TASK_STACKSIZE / sizeof(portSTACK_TYPE),
                              NULL,
                              COM_NETIF_QUEUE_TASK_PRIO,
                              NULL)) {
        LWIP_DEBUGF(NETIF_DEBUG, ("Cannot create com_netif_tx_queue_task\n"));
        return FALSE;
    }

    if (pdPASS != xTaskCreate(com_netif_rx_queue_task,
                              COM_NETIF_QUEUE_TAXK_RX_NAME,
                              COM_NETIF_QUEUE_TASK_STACKSIZE / sizeof(portSTACK_TYPE),
                              NULL,
                              COM_NETIF_QUEUE_TASK_PRIO,
                              NULL)) {
        LWIP_DEBUGF(NETIF_DEBUG, ("Cannot create com_netif_rx_queue_task\n"));
        return FALSE;
    }

    return TRUE;
}

void lwip_com_netif_dispatch_payload(const uint8_t *buf, size_t len)
{
    // 14 Ethernet (6+6+2) + 20 IP + 4 CRC
    if (buf == NULL || len < 38 || len > 1500) {
        LWIP_DEBUGF(NETIF_DEBUG, ("lwip_com_netif_dispatch_payload invalid parameter\n"));
        return;
    }

    // Check CRC
#if AIROHA_LWIP_COM_NETIF_CRC_ENABLE
    uint32_t buf_crc = *(uint32_t *)(buf + len - 4);
    buf_crc = ntohl(buf_crc);
    int crc = com_crc32(buf, len - 4);
    if (crc != buf_crc) {
        LWIP_DEBUGF(NETIF_DEBUG, ("lwip_com_netif_dispatch_payload crc fail: 0x%08X\n", crc));
        return;
    }
    // Remove CRC 4Bytes
    len = len - 4;
#endif

    struct pbuf *p = pbuf_alloc(PBUF_RAW, len, PBUF_RAM);
    if (p == NULL) {
        LWIP_DEBUGF(NETIF_DEBUG, ("lwip_com_netif_dispatch_payload pbuf_alloc fail\n"));
        return;
    }
    memcpy(p->payload, buf, len);
    p->if_idx = netif_get_index(&com_if);

    uint32_t pbuf_addr = (uint32_t)p;
    if (g_com_pbuf_rx_queue != NULL &&
            xQueueSend(g_com_pbuf_rx_queue, &pbuf_addr, portMAX_DELAY) != pdPASS) {
        LWIP_DEBUGF(NETIF_DEBUG, ("lwip_com_netif_dispatch_payload xQueueSend fail\n"));
        return;
    }
}

err_t com_linkoutput_fn(struct netif *netif, struct pbuf *p)
{
    if (p == NULL || netif == NULL || p->tot_len > 1500) {
        LWIP_DEBUGF(NETIF_DEBUG, ("com_linkoutput_fn paload_len > 1500 fail\n"));
        return ERR_MEM;
    }
    uint8_t *dest_mac = NULL;
    dest_mac = p->payload; // p->payload - eth_hdr - eth_addr dest
    //if (memcmp(dest_mac, COM_NETIF_REMOTE_MAC, 6) == 0) {
        // Note: Ethernet com_payload data at least 46 Bytes and 4 Bytes CRC
    #if AIROHA_LWIP_COM_NETIF_CRC_ENABLE
        #define ETHERNET_FRAME_MIN_SIZE 64
        #define ETHERNET_PAYLOAD_MIN_SIZE 46
        uint32_t payload_len = (p->tot_len < ETHERNET_PAYLOAD_MIN_SIZE ? ETHERNET_FRAME_MIN_SIZE : p->tot_len + 4);
    #else
        uint32_t payload_len = p->tot_len;
    #endif
        // Must Copy pbuf since top-layer will free p after "com_linkoutput_fn return".
        struct pbuf *r = pbuf_alloc(PBUF_RAW, payload_len, PBUF_RAM);
        if (r == NULL) {
            LWIP_DEBUGF(NETIF_DEBUG, ("com_linkoutput_fn pbuf_alloc fail\n"));
            return ERR_MEM;
        }

        // Create com_payload buffer
        uint8_t *payload = r->payload;
        memset(payload, 0, payload_len);
        // Copy p->payload data
        uint32_t cpy_len = 0;
        struct pbuf *q = p;
        while (q != NULL) {
            memcpy(payload + cpy_len, q->payload, q->len);
            cpy_len += q->len;
            q = q->next;
        }
    #if AIROHA_LWIP_COM_NETIF_CRC_ENABLE
        // Add CRC32 to com_payload
        int crc = com_crc32(payload, payload_len - 4);
        LWIP_DEBUGF(NETIF_DEBUG, ("com_linkoutput_fn: crc=0x%08X\n", crc));
        crc = htonl(crc);
        memcpy(payload + payload_len - 4, (uint8_t *)&crc, 4);
    #endif

        uint32_t pbuf_addr = (uint32_t)r;
     // hex_dump("com_linkoutput_fn", r->payload, r->tot_len);
        if (g_com_pbuf_tx_queue != NULL &&
                xQueueSend(g_com_pbuf_tx_queue, &pbuf_addr, portMAX_DELAY) != pdPASS) {
            LWIP_DEBUGF(NETIF_DEBUG, ("com_linkoutput_fn xQueueSend fail\n"));
            return ERR_BUF;
        }
    //} else {
    //    LWIP_DEBUGF(NETIF_DEBUG, ("not for COM_NETIF\n"));
    //}
    return ERR_OK;
}

err_t com_ethernetif_init(struct netif *netif)
{
  struct ethernetif *ethernetif;

  LWIP_ASSERT("netif != NULL", (netif != NULL));

  ethernetif = mem_malloc(sizeof(struct ethernetif));
  if (ethernetif == NULL) {
    LWIP_DEBUGF(NETIF_DEBUG, ("ethernetif_init: out of memory\n"));
    return ERR_MEM;
  }

#if LWIP_NETIF_HOSTNAME
  /* Initialize interface hostname */
  netif->hostname = "com";
#endif /* LWIP_NETIF_HOSTNAME */

  /*
   * Initialize the snmp variables and counters inside the struct netif.
   * The last argument should be replaced with your link speed, in units
   * of bits per second.
   */
  NETIF_INIT_SNMP(netif, snmp_ifType_ethernet_csmacd, LINK_SPEED_OF_YOUR_NETIF_IN_BPS);

  netif->state = ethernetif;
  netif->name[0] = IFNAME30;
  netif->name[1] = IFNAME31;

  netif->output = etharp_output;
  #if LWIP_IPV6
  netif->output_ip6 = NULL;
  #endif /* LWIP_IPV6 */
  netif->linkoutput = (netif_linkoutput_fn)com_linkoutput_fn;

  ethernetif->ethaddr = (struct eth_addr *)&(netif->hwaddr[0]);

  /* initialize the hardware */
  netif->hwaddr_len = ETHARP_HWADDR_LEN;

  /* set MAC hardware address */
  netif->hwaddr[0] = COM_NETIF_LOCAL_MAC[0]; // 76
  netif->hwaddr[1] = COM_NETIF_LOCAL_MAC[1]; // 82
  netif->hwaddr[2] = COM_NETIF_LOCAL_MAC[2]; // 'C'
  netif->hwaddr[3] = COM_NETIF_LOCAL_MAC[3]; // 'O'
  netif->hwaddr[4] = COM_NETIF_LOCAL_MAC[4]; // 'M'
  netif->hwaddr[5] = COM_NETIF_LOCAL_MAC[5]; // 'P'

  /* maximum transfer unit */
  netif->mtu = 1500;

  /* device capabilities */
  /* don't set NETIF_FLAG_ETHARP if this device is not an ethernet one */
  netif->flags = NETIF_FLAG_ETHARP | NETIF_FLAG_LINK_UP;
  return ERR_OK;
}
#endif

int ethernet_raw_pkt_sender(unsigned char *buf, unsigned int len, struct netif *netif)
{
    struct pbuf *p;
    int ret = 0;

    p = pbuf_alloc(PBUF_RAW_TX, len, PBUF_POOL);
    if (p == NULL)
    {
        LOG_E(lwip, "%s pbuf_alloc fail\n\r", __FUNCTION__);
        return -1;
    }
    memcpy(p->payload, buf, len);

    ret = low_level_output_scatter(netif, p);
    pbuf_free(p);
    return ret;
}

void ethernetif_intr_enhance_mode_allocate_pkt_callback(
                            unsigned int allocate_len,
                            void** pkt_ptr,
                            unsigned char** payload_ptr)
{
    struct pbuf *p;

    /* We allocate a pbuf chain of pbufs from the pool. */
#if (CFG_CONNSYS_TRX_BALANCE_EN == 1)

    if (g_balance_ctr.balance_en == TRUE)
    {
        if (connsys_balance_check_rx_yield())
        {
            vTaskPrioritySet(NULL, (TCPIP_THREAD_PRIO - 1));
            if (g_balance_ctr.tx_handle)
                vTaskPrioritySet((TaskHandle_t)g_balance_ctr.tx_handle, TCPIP_THREAD_PRIO);
        }
        else
        {
            vTaskPrioritySet(NULL, TCPIP_THREAD_PRIO);
        }
        p = pbuf_alloc(PBUF_RAW, (ssize_t)allocate_len, PBUF_RAM);

        if (p)
        {
            (*pkt_ptr) = p;
            (*payload_ptr) = p->payload;
        }
        else
        {
            (*pkt_ptr) = NULL;
            (*payload_ptr) = NULL;
            vTaskPrioritySet(NULL, (TCPIP_THREAD_PRIO - 1));
            if (g_balance_ctr.tx_handle)
                vTaskPrioritySet((TaskHandle_t)g_balance_ctr.tx_handle, TCPIP_THREAD_PRIO);
        }
    }
    else
    {
        p = pbuf_alloc(PBUF_RAW, (ssize_t)allocate_len, PBUF_RAM);
        if (p)
        {
            (*pkt_ptr) = p;
            (*payload_ptr) = p->payload;
            vTaskPrioritySet(NULL, TCPIP_THREAD_PRIO);
        }
        else
        {
            (*pkt_ptr) = NULL;
            (*payload_ptr) = NULL;
            vTaskPrioritySet(NULL, TCPIP_THREAD_PRIO - 1);
        }
    }
#else
    p = pbuf_alloc(PBUF_RAW, (ssize_t)allocate_len, PBUF_RAM);
    if (p)
    {
        (*pkt_ptr) = p;
        (*payload_ptr) = p->payload;
        //vTaskPrioritySet(NULL, TCPIP_THREAD_PRIO);
    }
    else
    {
        (*pkt_ptr) = NULL;
        (*payload_ptr) = NULL;
        //vTaskPrioritySet(NULL, (TCPIP_THREAD_PRIO - 1));
    }
#endif /* (CFG_CONNSYS_TRX_BALANCE_EN == 1) */
}


#if defined(MTK_WIFI_STUB_CONF_ENABLE) && (!MTK_WIFI_STUB_CONF_SPIM_ENABLE && !MTK_WIFI_STUB_CONF_SDIO_MSDC_ENABLE)
void ethernetif_intr_enhance_mode_dispatch(struct pbuf *p, struct netif *netif)
{
    struct pbuf *q;
    uint32_t total_len = 0;

    for(q = p; q != NULL; q = q->next) {
        total_len = total_len + (q->len);
    }

    //printf("[HRX]:lwip <- S(%d)\r\n",total_len);
    wfc_send_to_host((void *)p, total_len, 0);
}

#else

void
ethernetif_intr_enhance_mode_dispatch(struct pbuf *p, struct netif *netif)
{
  //struct ethernetif *ethernetif;

  struct eth_hdr *ethhdr;

  //ethernetif = netif->state;
  /* move received packet into a new pbuf */

  /* no packet could be read, silently ignore this */
  if (p == NULL) return;
  /* points to packet payload, which starts with an Ethernet header */
  ethhdr = p->payload;

  switch (htons(ethhdr->type)) {
  /* IP or ARP packet? */
  case ETHTYPE_IP:
#if LWIP_IPV6
  case ETHTYPE_IPV6:
#endif
  case ETHTYPE_ARP:
#if PPPOE_SUPPORT
  /* PPPoE packet? */
  case ETHTYPE_PPPOEDISC:
  case ETHTYPE_PPPOE:
#endif /* PPPOE_SUPPORT */
    /* full packet send to tcpip_thread to process */
    if (netif->input(p, netif) != ERR_OK)
     { LWIP_DEBUGF(NETIF_DEBUG, ("ethernetif_input: IP input error\n"));
       pbuf_free(p);
       p = NULL;
     }
    break;
#if 1 // for wpa_supplicant eapol packets
  case 0x888E:
#ifndef RELAY
    if((p->len > 12) && (g_lwip_eapol_rx_socket >= 0) && (g_lwip_eapol_rx_socket_second >= 0)) {
            struct sockaddr_in    to;
            ssize_t               len;

            //LOG_I(lwip,"<<Dual interface RX EAPOL (Len=%d)>>\n", p->len);

            to.sin_family      = PF_INET;
            to.sin_addr.s_addr = htonl((127 << 24) | 1);

            if ((IFNAME10 == netif->name[0]) && (IFNAME11 == netif->name[1])) {
                to.sin_port = htons(66);
                //LOG_I(lwip,"send to AP socket[%d]...\n", g_lwip_eapol_rx_socket);
                LWIP_DEBUGF(NETIF_DEBUG, ("send to AP socket: %d\n",  g_lwip_eapol_rx_socket));
                len = lwip_sendto(g_lwip_eapol_rx_socket, p->payload, p->len, 0, (struct sockaddr *)&to, sizeof(to));
            } else {
                to.sin_port = htons(76);
                //LOG_I(lwip,"send to STA socket[%d]...\n", g_lwip_eapol_rx_socket_second);
                LWIP_DEBUGF(NETIF_DEBUG, ("send to STA socket: %d\n", g_lwip_eapol_rx_socket_second));
                len = lwip_sendto(g_lwip_eapol_rx_socket_second, p->payload, p->len, 0, (struct sockaddr *)&to, sizeof(to));
            }

            if (len != p->len)
                LOG_E(lwip,"Dual interface eapol-rx relay sendto failed!\n");
    }//dual interface
    else if ((p->len > 12) && (g_lwip_eapol_rx_socket >= 0))
    {
        struct sockaddr_in    to;
        ssize_t               len;

        //LOG_I(lwip, "<<RX EAPOL (Len=%d)>>\n", p->len);

        to.sin_family      = PF_INET;
        to.sin_addr.s_addr = htonl((127 << 24) | 1);

        to.sin_port        = htons(66);
        //LOG_I(lwip, "send to socket[%d]...\n", g_lwip_eapol_rx_socket);
        LWIP_DEBUGF(NETIF_DEBUG, ("send to socket: %d%d\n",  p->len, g_lwip_eapol_rx_socket));
        len = lwip_sendto(g_lwip_eapol_rx_socket, p->payload, p->len, 0, (struct sockaddr *)&to, sizeof(to));

        if (len != p->len)
            LOG_E(lwip, "Single interface eapol-rx relay sendto failed!\n");
    }

    pbuf_free(p);
    p = NULL;
#else
    if (p->len > 0) {
        // TODO: relay
    }
#endif
    break;
#endif
  default:
    pbuf_free(p);
    p = NULL;
    break;
  }
}
#endif

#define MAX_CONNSYS_RX_ZERO_CP_PKT_CONTENT 128

PKT_HANDLE_RESULT_T lwip_deliver_tcpip(void* pkt, uint8_t *payload, uint32_t len, int inf)
{
    struct pbuf *p = (struct pbuf *) pkt;
    struct netif *netif = &sta_if;

    if(p != NULL){
        //int i;

        if (inf == 1)
            netif = &ap_if;

        LINK_STATS_INC(link.recv);
        ethernetif_intr_enhance_mode_dispatch(p, netif);

        return PKT_HANDLE_NON_COMPLETE;
    }else{
        LOG_E(lwip, "%s pbuf NULL\n\r", __FUNCTION__);
        LINK_STATS_INC(link.memerr);
        LINK_STATS_INC(link.drop);
        return PKT_HANDLE_COMPLETE;
    }
}


void inband_handle_func(void* pkt_ptr, unsigned char *payload, unsigned int len)
{
    struct pbuf *p = (struct pbuf *) pkt_ptr;

    inband_queue_handler(pkt_ptr, payload, len);
    pbuf_free(p);
    pkt_ptr = NULL;
}

void ethernetif_free_pkt(void *pkt_ptr)
{
    pbuf_free((struct pbuf*)pkt_ptr);
}

uint8_t enqueue_bottom_half_from_isr(int32_t port, PNETFUNC func)
{
    return NetJobAddFromISR(func, 0, 0);
}

void inform_ip_ready_callback(struct netif *netif)
{
    /*
     * This is a private API , which used to inform IP is ready to wifi driver
     * In present, WiFi Driver will do some operation when this API is invoked, such as:
     * Do WiFi&BLE Coexstence relative behavior if BLE is enabled and do Power Saving Status change.
     * This API will be improved, user may need to use new API to replace it in future
     */
    LOG_I(lwip, "inform_ip_ready_callback");
    wifi_connection_inform_ip_ready();
}

void ethernetif_init(sta_ip_mode_t sta_ip_mode,
                     uint8_t *sta_mac_addr,
                     uint8_t *ap_mac_addr,
                     ip4_addr_t *sta_ip_addr, ip4_addr_t *sta_net_mask, ip4_addr_t *sta_gw,
                     ip4_addr_t *ap_ip_addr, ip4_addr_t *ap_net_mask, ip4_addr_t *ap_gw,
                     uint8_t opmode)
{
    uint8_t sta_mac_address[6];
    uint8_t ap_mac_address[6];

    /* for patch and fw download */
    ethernetif_init_callback();

    memset(&sta_if, 0, sizeof(sta_if));
    memset(&ap_if,  0, sizeof(ap_if));

    // Note: *MUST* first add AP, then STA interface, to make STA the first
    //       interface in the link-list: STA -> AP -> NULL.
    if (0 > wifi_config_get_mac_address(WIFI_PORT_STA, (uint8_t *)&sta_mac_address) ||
        0 > wifi_config_get_mac_address(WIFI_PORT_AP, (uint8_t *)&ap_mac_address)) {
        LOG_E(lwip, "get mac fail\n\r");
        return;
    }

    netif_add(&ap_if, ap_ip_addr, ap_net_mask, ap_gw, NULL,
              ethernetif_init2, tcpip_input);
    netif_add(&sta_if, sta_ip_addr, sta_net_mask, sta_gw,
              NULL, ethernetif_init1, tcpip_input);

#if AIROHA_LWIP_COM_NETIF
    ipaddr_aton(COM_NETIF_IP_ADDR, &com_if_ip_addr);
    ipaddr_aton(COM_NETIF_GW_ADDR, &com_if_gw_addr);
    ipaddr_aton(COM_NETIF_MASK_ADDR, &com_if_mask_addr);
    netif_add(&com_if, &com_if_ip_addr, &com_if_mask_addr, &com_if_gw_addr,
              NULL, com_ethernetif_init, tcpip_input);
    netif_set_up(&com_if);
#endif

    low_level_set_mac_addr(&ap_if,  ap_mac_address);
    low_level_set_mac_addr(&sta_if, sta_mac_address);
    /**
     * Airoha Change - Feature
     * Must set the link local address after set the hwaddr for netif
     * */
    #if LWIP_IPV6
    ip6_netif_configure(&sta_if);
    #endif /** LWIP_IPV6 */
    /** Airoha Change End */

    //netif_set_default(&sta_if);
    netif_set_up(&sta_if);
    netif_set_up(&ap_if);

    //install default route
    switch (opmode) {
        case WIFI_MODE_AP_ONLY:
            netif_set_default(&ap_if);
            netif_set_link_down(&sta_if);
            break;
        case WIFI_MODE_STA_ONLY:
            netif_set_default(&sta_if);
            netif_set_link_down(&ap_if);
            wifi_config_set_ip_mode((uint8_t)sta_ip_mode);
            if (sta_ip_mode == STA_IP_MODE_DHCP) {
                netif_set_wifi_ip_ready_callback(&sta_if, inform_ip_ready_callback);
            }
            break;
        case WIFI_MODE_REPEATER:
            netif_set_default(&sta_if);
            break;
    }
}

void lwip_tcpip_init(lwip_tcpip_config_t *tcpip_config, uint8_t opmode)
{
    lwip_socket_init();
    tcpip_init(NULL, NULL);
    ethernetif_init((sta_ip_mode_t)tcpip_config->ip_mode,
                    NULL,
                    NULL,
                    &tcpip_config->sta_addr,
                    &tcpip_config->sta_mask,
                    &tcpip_config->sta_gateway,
                    &tcpip_config->ap_addr,
                    &tcpip_config->ap_mask,
                    &tcpip_config->ap_gateway,
                    opmode);
}


int lwip_get_netif_name(netif_type_t netif_type, char *name)
{
    struct netif *netif = NULL;

    if (name == NULL) {
        return 0;
    }

    for (netif = netif_list; netif != NULL; netif = netif->next) {
        if (netif_type == NETIF_TYPE_AP &&
                IFNAME10 == netif->name[0] &&
                IFNAME11 == netif->name[1]) {
            name[0] = IFNAME10;
            name[1] = IFNAME11;
            name[2] = '0' + netif->num;
            return 1;
        } else if (netif_type == NETIF_TYPE_STA &&
                   IFNAME00 == netif->name[0] &&
                   IFNAME01 == netif->name[1]) {
            name[0] = IFNAME00;
            name[1] = IFNAME01;
            name[2] = '0' + netif->num;
            return 1;
        } else if (netif_type == NETIF_TYPE_LOOPBACK &&
                   IFNAME20 == netif->name[0] &&
                   IFNAME21 == netif->name[1]) {
            name[0] = IFNAME20;
            name[1] = IFNAME21;
            name[2] = '0' + netif->num;
            return 1;
        }
#if AIROHA_LWIP_COM_NETIF
        else if (netif_type == NETIF_TYPE_COM &&
                 IFNAME30 == netif->name[0] &&
                 IFNAME31 == netif->name[1]) {
            name[0] = IFNAME30;
            name[1] = IFNAME31;
            name[2] = '0' + netif->num;
            return 1;
        }
#endif
    }

    return 0;
}

struct netif *netif_find_by_type(netif_type_t netif_type)
{
    char name[4] = {0};

    if (lwip_get_netif_name(netif_type, (char *)name) == 1) {
        LWIP_DEBUGF(NETIF_DEBUG, ("netif_find_by_type: %c%c\n", name[0], name[1]));
        return netif_find(name);
    } else {
        return NULL;
    }
}

void tcpip_stack_init(tcpip_config_t *tcpip_config, uint8_t opmode)
{
    lwip_socket_init();
    tcpip_init(NULL, NULL);
    ethernetif_init(tcpip_config->sta_ip_mode,
                    tcpip_config->sta_mac_addr,
                    tcpip_config->ap_mac_addr,
                    &tcpip_config->sta_addr,
                    &tcpip_config->sta_mask,
                    &tcpip_config->sta_gateway,
                    &tcpip_config->ap_addr,
                    &tcpip_config->ap_mask,
                    &tcpip_config->ap_gateway,
                    opmode);
}

bool lwip_wifi_is_repeater()
{
#if !MTK_WIFI_REPEATER_ENABLE || defined(TCPIP_FOR_MT5931_ENABLE) || defined(TCPIP_FOR_MT5932_ENABLE) || defined(MTK_TCPIP_FOR_EXTERNAL_MODULE_ENABLE)
    return FALSE;
#else
    uint8_t wifi_mode = 0;
    int32_t wifi_result = wifi_config_get_opmode(&wifi_mode);
    LWIP_DEBUGF(NETIF_DEBUG, ("lwip_wifi_is_repeater: wifi_result=%d wifi_mode=%d", wifi_result, wifi_mode));
    return (wifi_mode == WIFI_MODE_REPEATER);
#endif
}
