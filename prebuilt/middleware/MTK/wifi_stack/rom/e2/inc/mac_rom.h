/*
 ***************************************************************************
 * Ralink Tech Inc.
 * 4F, No. 2 Technology	5th Rd.
 * Science-based Industrial Park
 * Hsin-chu, Taiwan, R.O.C.
 *
 * (c) Copyright 2002-2004, Ralink Technology, Inc.
 *
 * All rights reserved.	Ralink's source	code is	an unpublished work and the
 * use of a copyright notice does not imply otherwise. This source code
 * contains confidential trade secret material of Ralink Tech. Any attemp
 * or participation in deciphering, decoding, reverse engineering or in any
 * way altering	the source code	is stricitly prohibited, unless	the prior
 * written consent of Ralink Technology, Inc. is obtained.
 ***************************************************************************

	Module Name:
	mac.h

	Abstract:
	Ralink Wireless Chip MAC related definition & structures

	Revision History:
	Who			When		  What
	--------	----------	  ----------------------------------------------
*/


#ifndef __MAC_H__
#define __MAC_H__
#ifndef MT_MAC
#define MT_MAC
#endif
#ifdef BIG_ENDIAN
#undef BIG_ENDIAN
#endif
#ifdef MT_MAC
//#include "mt_mac.h"
//#include "mac/mac_mt/mt_mac_ctrl.h"
#endif /* MT_MAC */



#define WMM_QUE_NUM		4 /* each sta has 4 Queues to mapping to each WMM AC */

/* value domain of pTxD->HostQId (4-bit: 0~15) */
#define QID_AC_BK               1   /* meet ACI definition in 802.11e */
#define QID_AC_BE               0   /* meet ACI definition in 802.11e */
#define QID_AC_VI               2
#define QID_AC_VO               3

#define ETH_TYPE_VLAN	0x8100
#define ETH_TYPE_IPv4	0x0800
#define ETH_TYPE_IPv6	0x86dd
#define ETH_TYPE_ARP	0x0806
#define ETH_TYPE_EAPOL	0x888e
#define ETH_TYPE_WAI	0x88b4
#define ETH_TYPE_FASTROAMING	0x890d

#define IP_VER_CODE_V4	0x40
#define IP_VER_CODE_V6	0x60
#define IP_PROTO_UDP	0x11
#define IP_HDR_LEN		20
#define ETH_HDR_LEN		14

#define DMA_SCH_LMAC		0
#define DMA_SCH_BYPASS		1
#define DMA_SCH_HYBRID		2

//#define TXINFO_SIZE			4
#ifdef IAR_COMPILE
GNU_PACKED typedef union  _TXINFO_STRUC{
#else
typedef union GNU_PACKED _TXINFO_STRUC{
#endif
#ifdef RTMP_MAC
	struct _TXINFO_OMAC txinfo_omac;
#endif /* RTMP_MAC */
#ifdef RLT_MAC
	struct _TXINFO_NMAC_PKT txinfo_nmac_pkt;
	struct _TXINFO_NMAC_CMD txinfo_nmac_cmd;
#endif /* RLT_MAC */
	UINT32 word;
}TXINFO_STRUC;

#define SHORT_PREAMBLE 0
#define LONG_PREAMBLE 1

#define TIMESTAMP_FIELD_LEN 8
/* 7.3.1.3 Beacon Interval field */
#define BEACON_INTERVAL_FIELD_LEN                   2
#define CAP_INFO_FIELD_LEN                          2
#define MAX_IE_LENGTH       (600)

#if 0
/*
	bit31 =>802.3 if set 1, implay you hav did header translation
	bit30 => put VLAN field
	
*/
#define RAL_RXINFO_SIZE			4
#ifdef RT_BIG_ENDIAN
typedef	struct GNU_PACKED _RXINFO_STRUC {
	UINT32		hdr_trans_ip_sum_err:1;		/* IP checksum error */
	UINT32		vlan_taged_tcp_sum_err:1;	/* TCP checksum error */
	UINT32		rsv:1;
	UINT32		action_wanted:1;
	UINT32		deauth:1;
	UINT32		disasso:1;
	UINT32		beacon:1;
	UINT32		probe_rsp:1;
	UINT32		sw_fc_type1:1;
	UINT32		sw_fc_type0:1;
	UINT32		pn_len:3;
	UINT32		wapi_kidx:1;
	UINT32		BssIdx3:1;
	UINT32		Decrypted:1;
	UINT32		AMPDU:1;
	UINT32		L2PAD:1;
	UINT32		RSSI:1;
	UINT32		HTC:1;
	UINT32		AMSDU:1;		/* rx with 802.3 header, not 802.11 header. obsolete. */
	UINT32		CipherErr:2;        /* 0: decryption okay, 1:ICV error, 2:MIC error, 3:KEY not valid */
	UINT32		Crc:1;			/* 1: CRC error */
	UINT32		MyBss:1;		/* 1: this frame belongs to the same BSSID */
	UINT32		Bcast:1;			/* 1: this is a broadcast frame */
	UINT32		Mcast:1;			/* 1: this is a multicast frame */
	UINT32		U2M:1;			/* 1: this RX frame is unicast to me */
	UINT32		FRAG:1;
	UINT32		NULLDATA:1;
	UINT32		DATA:1;
	UINT32		BA:1;
}	RXINFO_STRUC;
#else
typedef	struct GNU_PACKED _RXINFO_STRUC {
	UINT32		BA:1;
	UINT32		DATA:1;
	UINT32		NULLDATA:1;
	UINT32		FRAG:1;
	UINT32		U2M:1;
	UINT32		Mcast:1;
	UINT32		Bcast:1;
	UINT32		MyBss:1;
	UINT32		Crc:1;
	UINT32		CipherErr:2;
	UINT32		AMSDU:1;
	UINT32		HTC:1;
	UINT32		RSSI:1;
	UINT32		L2PAD:1;
	UINT32		AMPDU:1;
	UINT32		Decrypted:1;
	UINT32		BssIdx3:1;
	UINT32		wapi_kidx:1;
	UINT32		pn_len:3;
	UINT32		sw_fc_type0:1;
	UINT32      sw_fc_type1:1;
	UINT32      probe_rsp:1;
	UINT32		beacon:1;
	UINT32		disasso:1;
	UINT32      deauth:1;
	UINT32      action_wanted:1;
	UINT32      rsv:1;
	UINT32		vlan_taged_tcp_sum_err:1;
	UINT32		hdr_trans_ip_sum_err:1;
}RXINFO_STRUC;
#endif
#endif

typedef struct _MAC_TX_INFO{
	UCHAR WCID;
	BOOLEAN FRAG;
	BOOLEAN InsTimestamp;
	BOOLEAN NSeq;
	BOOLEAN Ack;
	BOOLEAN BM;
	BOOLEAN CFACK;
	BOOLEAN AMPDU;
	UCHAR BASize;
	UCHAR PID;
	UCHAR TID;
	UCHAR TxRate;
	UCHAR Txopmode;
	ULONG Length;
	UCHAR hdr_len;
	UCHAR hdr_pad;
	UCHAR eth_type_offset;
	UCHAR bss_idx;
	UCHAR q_idx;
	UCHAR prot;
	UCHAR AntPri;
	UCHAR SpeEn;
	UCHAR Preamble;
#ifdef MT_MAC
	UCHAR Type;
	UCHAR SubType;
	UINT32 TxSPriv;
	UCHAR PsmBySw; /* PSM bit controlled by SW */
	UINT8 OmacIdx;
	UINT8 wmm_set; // TODO: shiang-MT7615, replace band by bss_idx?
    BOOLEAN IsTmr;
    BOOLEAN IsOffloadPkt;//host gen pkt template, make pkt enqued by fw.
#endif /* MT_MAC */
}MAC_TX_INFO;
#if 0
typedef enum _PID {
	PID_DATA_NORMALUCAST = 0,
	PID_DATA_NOT_NORM_ACK,
	PID_DATA_AMPDU,
	PID_MGMT,
	PID_DATA_NO_ACK,
	PID_CTL_BAR,
	PID_PS_DATA,
	PID_TDLS,
	PID_P2P_ACTION,
	PID_NULL_FRAME_PWR_ACTIVE,
	PID_NULL_FRAME_PWR_SAVE,
	PID_BEACON = 0x20,
	PID_MAX = 0x40,
}PID;
#endif
#if LATER

struct _RTMP_ADAPTER;
struct _TXD_STRUC;
struct _RXD_STRUC;



VOID dump_txinfo(struct _RTMP_ADAPTER *pAd, TXINFO_STRUC *pTxInfo);
VOID dump_rxinfo(struct _RTMP_ADAPTER *pAd, RXINFO_STRUC *pRxInfo);

VOID dump_tmac_info(struct _RTMP_ADAPTER *pAd, UCHAR *tmac_info);
VOID dump_rmac_info(struct _RTMP_ADAPTER *pAd, UCHAR *rmac_info);

#ifdef MT_MAC
VOID DumpTxSFormat(struct _RTMP_ADAPTER *pAd, UINT8 Format, CHAR *Data);
#endif /* MT_MAC */

#ifdef RTMP_PCI_SUPPORT
VOID dump_txd(struct _RTMP_ADAPTER *pAd, struct _TXD_STRUC *pTxD);
VOID dump_rxd(struct _RTMP_ADAPTER *pAd, struct _RXD_STRUC *pRxD);
#endif
#endif //LATER
#endif /* __MAC_H__ */


