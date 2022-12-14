/*
 ***************************************************************************
 * Ralink Tech Inc.
 * 4F, No. 2 Technology	5th	Rd.
 * Science-based Industrial	Park
 * Hsin-chu, Taiwan, R.O.C.
 *
 * (c) Copyright 2002-2004, Ralink Technology, Inc.
 *
 * All rights reserved.	Ralink's source	code is	an unpublished work	and	the
 * use of a	copyright notice does not imply	otherwise. This	source code
 * contains	confidential trade secret material of Ralink Tech. Any attemp
 * or participation	in deciphering,	decoding, reverse engineering or in	any
 * way altering	the	source code	is stricitly prohibited, unless	the	prior
 * written consent of Ralink Technology, Inc. is obtained.
 ***************************************************************************

	Module Name:
	oid.h

	Abstract:

	Revision History:
	Who			When			What
	--------	----------		----------------------------------------------
	Name		Date			Modification logs
*/
#ifndef _OID_H_
#define _OID_H_

#ifndef UNDER_CE
// OID definition, since NDIS 5.0 didn't define these, we need to define for our own
//#if _WIN32_WINNT<=0x0500

#define OID_GEN_MACHINE_NAME               0x0001021A

#ifdef RALINK_ATE
#define RT_QUERY_ATE_TXDONE_COUNT			0x0401
#endif // RALINK_ATE //
#define RT_QUERY_SIGNAL_CONTEXT				0x0402
#define RT_SET_IAPP_PID                 	0x0404

//
// IEEE 802.11 OIDs
//
#define	OID_GET_SET_TOGGLE			0x8000

#define	OID_802_11_NETWORK_TYPES_SUPPORTED			0x0103
#define	OID_802_11_NETWORK_TYPE_IN_USE				0x0104
#define	OID_802_11_RSSI_TRIGGER						0x0107
#define	RT_OID_802_11_RSSI							0x0108 //rt2860	only , kathy
#define	RT_OID_802_11_RSSI_1						0x0109 //rt2860	only , kathy
#define	RT_OID_802_11_RSSI_2						0x010A //rt2860	only , kathy
#define	OID_802_11_NUMBER_OF_ANTENNAS				0x010B
#define	OID_802_11_RX_ANTENNA_SELECTED				0x010C
#define	OID_802_11_TX_ANTENNA_SELECTED				0x010D
#define	OID_802_11_SUPPORTED_RATES					0x010E
#define	OID_802_11_ADD_WEP							0x0112
#define	OID_802_11_REMOVE_WEP						0x0113
#define	OID_802_11_DISASSOCIATE						0x0114
#define	OID_802_11_PRIVACY_FILTER					0x0118
#define	OID_802_11_ASSOCIATION_INFORMATION			0x011E
#define	OID_802_11_TEST								0x011F
#define	RT_OID_802_11_COUNTRY_REGION				0x0507
#define	OID_802_11_BSSID_LIST_SCAN					0x0508
#define	OID_802_11_SSID								0x0509
#define	OID_802_11_BSSID							0x050A
#define	RT_OID_802_11_RADIO							0x050B
#define	RT_OID_802_11_PHY_MODE						0x050C
#define	RT_OID_802_11_STA_CONFIG					0x050D
#define	OID_802_11_DESIRED_RATES					0x050E
#define	RT_OID_802_11_PREAMBLE						0x050F
#define	OID_802_11_WEP_STATUS						0x0510
#define	OID_802_11_AUTHENTICATION_MODE				0x0511
#define	OID_802_11_INFRASTRUCTURE_MODE				0x0512
#define	RT_OID_802_11_RESET_COUNTERS				0x0513
#define	OID_802_11_RTS_THRESHOLD					0x0514
#define	OID_802_11_FRAGMENTATION_THRESHOLD			0x0515
#define	OID_802_11_POWER_MODE						0x0516
#define	OID_802_11_TX_POWER_LEVEL					0x0517
#define	RT_OID_802_11_ADD_WPA						0x0518
#define	OID_802_11_REMOVE_KEY						0x0519
#define	OID_802_11_ADD_KEY							0x0520
#define	OID_802_11_CONFIGURATION					0x0521
#define	OID_802_11_TX_PACKET_BURST					0x0522
#define	RT_OID_802_11_QUERY_NOISE_LEVEL				0x0523
#define	RT_OID_802_11_EXTRA_INFO					0x0524
#ifdef	DBG
#define	RT_OID_802_11_HARDWARE_REGISTER				0x0525
#endif
#define OID_802_11_ENCRYPTION_STATUS            OID_802_11_WEP_STATUS
#define OID_802_11_DEAUTHENTICATION                 0x0526
#define OID_802_11_DROP_UNENCRYPTED                 0x0527
#define OID_802_11_MIC_FAILURE_REPORT_FRAME         0x0528

// For 802.1x daemin using to require current driver configuration
#define OID_802_11_RADIUS_QUERY_SETTING				0x0540

#define	RT_OID_DEVICE_NAME							0x0607
#define	RT_OID_VERSION_INFO							0x0608
#define	OID_802_11_BSSID_LIST						0x0609
#define	OID_802_3_CURRENT_ADDRESS					0x060A
#define	OID_GEN_MEDIA_CONNECT_STATUS				0x060B
#define	RT_OID_802_11_QUERY_LINK_STATUS				0x060C
#define	OID_802_11_RSSI								0x060D
#define	OID_802_11_STATISTICS						0x060E
#define	OID_GEN_RCV_OK								0x060F
#define	OID_GEN_RCV_NO_BUFFER						0x0610
#define	RT_OID_802_11_QUERY_EEPROM_VERSION			0x0611
#define	RT_OID_802_11_QUERY_FIRMWARE_VERSION		0x0612
#define	RT_OID_802_11_QUERY_LAST_RX_RATE			0x0613
#define	RT_OID_802_11_TX_POWER_LEVEL_1				0x0614
#define	RT_OID_802_11_QUERY_PIDVID					0x0615
//for WPA_SUPPLICANT_SUPPORT
#define OID_SET_COUNTERMEASURES                     0x0616
#define OID_802_11_SET_IEEE8021X                    0x0617
#define OID_802_11_SET_IEEE8021X_REQUIRE_KEY        0x0618
#define OID_802_11_PMKID                            0x0620
#define RT_OID_WPA_SUPPLICANT_SUPPORT               0x0621
#define RT_OID_WE_VERSION_COMPILED                  0x0622
#define RT_OID_NEW_DRIVER                           0x0623
#define RT_OID_WPS_PROBE_REQ_IE						0x0625
#ifdef UCOS
#define RT_OID_WINDOWS_ZERO_CONFIG_SUPPORT			0x062F
#endif // UCOS //

//rt2860 , kathy
#define	RT_OID_802_11_SNR_0							0x0630
#define	RT_OID_802_11_SNR_1							0x0631
#define	RT_OID_802_11_QUERY_LAST_TX_RATE			0x0632
#define	RT_OID_802_11_QUERY_HT_PHYMODE				0x0633
#define	RT_OID_802_11_SET_HT_PHYMODE				0x0634
#define	OID_802_11_RELOAD_DEFAULTS					0x0635
#define	RT_OID_802_11_QUERY_APSD_SETTING			0x0636
#define	RT_OID_802_11_SET_APSD_SETTING				0x0637
#define	RT_OID_802_11_QUERY_APSD_PSM				0x0638
#define	RT_OID_802_11_SET_APSD_PSM					0x0639
#define	RT_OID_802_11_QUERY_DLS						0x063A
#define	RT_OID_802_11_SET_DLS						0x063B
#define	RT_OID_802_11_QUERY_DLS_PARAM				0x063C
#define	RT_OID_802_11_SET_DLS_PARAM					0x063D
#define RT_OID_802_11_QUERY_WMM              		0x063E
#define RT_OID_802_11_SET_WMM      					0x063F
#define RT_OID_802_11_QUERY_IMME_BA_CAP				0x0640
#define RT_OID_802_11_SET_IMME_BA_CAP				0x0641
#define RT_OID_802_11_QUERY_BATABLE					0x0642
#define RT_OID_802_11_ADD_IMME_BA					0x0643
#define RT_OID_802_11_TEAR_IMME_BA					0x0644
#define RT_OID_DRIVER_DEVICE_NAME                   0x0645
#define RT_OID_802_11_QUERY_DAT_HT_PHYMODE          0x0646
#define RT_OID_QUERY_MULTIPLE_CARD_SUPPORT          0x0647

typedef UCHAR   NDIS_802_11_MAC_ADDRESS[6];
typedef struct _NDIS_802_11_PASSPHRASE
{
    UINT           KeyLength;          // length of key in bytes
    NDIS_802_11_MAC_ADDRESS BSSID;
    UCHAR           KeyMaterial[1];     // variable length depending on above field
} NDIS_802_11_PASSPHRASE, *PNDIS_802_11_PASSPHRASE;


/*+++ add by woody +++*/
#define OID_802_11_SET_PASSPHRASE				0x0649


#ifdef SNMP_SUPPORT
//SNMP ieee 802dot11, kathy , 2008_0220
// dot11res(3)
#define RT_OID_802_11_MANUFACTUREROUI			0x0700
#define RT_OID_802_11_MANUFACTURERNAME			0x0701
#define RT_OID_802_11_RESOURCETYPEIDNAME		0x0702

// dot11smt(1)
#define RT_OID_802_11_PRIVACYOPTIONIMPLEMENTED	0x0703
#define RT_OID_802_11_POWERMANAGEMENTMODE		0x0704
#define OID_802_11_WEPDEFAULTKEYVALUE			0x0705 // read , write
#define OID_802_11_WEPDEFAULTKEYID				0x0706
#define RT_OID_802_11_WEPKEYMAPPINGLENGTH		0x0707
#define OID_802_11_SHORTRETRYLIMIT				0x0708
#define OID_802_11_LONGRETRYLIMIT				0x0709
#define RT_OID_802_11_PRODUCTID					0x0710
#define RT_OID_802_11_MANUFACTUREID				0x0711

// //dot11Phy(4)
#define OID_802_11_CURRENTCHANNEL				0x0712

//dot11mac
#define RT_OID_802_11_MAC_ADDRESS				0x0713
#endif // SNMP_SUPPORT //

#define OID_802_11_BUILD_CHANNEL_EX				0x0714
#define OID_802_11_GET_CH_LIST					0x0715
#define OID_802_11_GET_COUNTRY_CODE				0x0716
#define OID_802_11_GET_CHANNEL_GEOGRAPHY		0x0717

//#define RT_OID_802_11_STATISTICS              (OID_GET_SET_TOGGLE | OID_802_11_STATISTICS)

#ifdef WSC_INCLUDED
#define	RT_OID_WSC_AUTO_PROVISION_WITH_BSSID		0x0737
#define	RT_OID_WSC_AUTO_PROVISION					0x0738
#ifdef WSC_LED_SUPPORT
//WPS LED MODE 10 for Dlink WPS LED
#define RT_OID_LED_WPS_MODE10						0x0739
#endif // WSC_LED_SUPPORT //
#endif // WSC_INCLUDED //
#ifdef CONFIG_STA_SUPPORT
#define RT_OID_WSC_SET_PASSPHRASE                   0x0740 // passphrase for wpa(2)-psk
#define RT_OID_WSC_DRIVER_AUTO_CONNECT              0x0741
#define RT_OID_WSC_QUERY_DEFAULT_PROFILE            0x0742
#define RT_OID_WSC_SET_CONN_BY_PROFILE_INDEX        0x0743
#define RT_OID_WSC_SET_ACTION                       0x0744
#define RT_OID_WSC_SET_SSID                         0x0745
#define RT_OID_WSC_SET_PIN_CODE                     0x0746
#define RT_OID_WSC_SET_MODE                         0x0747 // PIN or PBC
#define RT_OID_WSC_SET_CONF_MODE                    0x0748 // Enrollee or Registrar
#define RT_OID_WSC_SET_PROFILE                      0x0749
#endif // CONFIG_STA_SUPPORT //
#ifdef CONFIG_AP_SUPPORT
#ifdef APCLI_SUPPORT
#define RT_OID_APCLI_WSC_PIN_CODE					0x074A
#endif // APCLI_SUPPORT //
#endif // CONFIG_AP_SUPPORT //
#define	RT_OID_WSC_V2_SUPPORT						0x074E
#define	RT_OID_WSC_CONFIG_STATUS					0x074F
#define RT_OID_802_11_WSC_QUERY_PROFILE				0x0750
// for consistency with RT61
#define RT_OID_WSC_QUERY_STATUS						0x0751
#define RT_OID_WSC_PIN_CODE							0x0752
#define RT_OID_WSC_UUID								0x0753
#define RT_OID_WSC_SET_SELECTED_REGISTRAR			0x0754
#define RT_OID_WSC_EAPMSG							0x0755
#define RT_OID_WSC_MANUFACTURER						0x0756
#define RT_OID_WSC_MODEL_NAME						0x0757
#define RT_OID_WSC_MODEL_NO							0x0758
#define RT_OID_WSC_SERIAL_NO						0x0759
#define RT_OID_WSC_READ_UFD_FILE					0x075A
#define RT_OID_WSC_WRITE_UFD_FILE					0x075B
#define RT_OID_WSC_QUERY_PEER_INFO_ON_RUNNING		0x075C
#define RT_OID_WSC_MAC_ADDRESS						0x0760


// Ralink defined OIDs
// Dennis Lee move to platform specific	

#define	RT_OID_802_11_BSSID					  (OID_GET_SET_TOGGLE |	OID_802_11_BSSID)
#define	RT_OID_802_11_SSID					  (OID_GET_SET_TOGGLE |	OID_802_11_SSID)
#define	RT_OID_802_11_INFRASTRUCTURE_MODE	  (OID_GET_SET_TOGGLE |	OID_802_11_INFRASTRUCTURE_MODE)
#define	RT_OID_802_11_ADD_WEP				  (OID_GET_SET_TOGGLE |	OID_802_11_ADD_WEP)
#define	RT_OID_802_11_ADD_KEY				  (OID_GET_SET_TOGGLE |	OID_802_11_ADD_KEY)
#define	RT_OID_802_11_REMOVE_WEP			  (OID_GET_SET_TOGGLE |	OID_802_11_REMOVE_WEP)
#define	RT_OID_802_11_REMOVE_KEY			  (OID_GET_SET_TOGGLE |	OID_802_11_REMOVE_KEY)
#define	RT_OID_802_11_DISASSOCIATE			  (OID_GET_SET_TOGGLE |	OID_802_11_DISASSOCIATE)
#define	RT_OID_802_11_AUTHENTICATION_MODE	  (OID_GET_SET_TOGGLE |	OID_802_11_AUTHENTICATION_MODE)
#define	RT_OID_802_11_PRIVACY_FILTER		  (OID_GET_SET_TOGGLE |	OID_802_11_PRIVACY_FILTER)
#define	RT_OID_802_11_BSSID_LIST_SCAN		  (OID_GET_SET_TOGGLE |	OID_802_11_BSSID_LIST_SCAN)
#define	RT_OID_802_11_WEP_STATUS			  (OID_GET_SET_TOGGLE |	OID_802_11_WEP_STATUS)
#define	RT_OID_802_11_RELOAD_DEFAULTS		  (OID_GET_SET_TOGGLE |	OID_802_11_RELOAD_DEFAULTS)
#define	RT_OID_802_11_NETWORK_TYPE_IN_USE	  (OID_GET_SET_TOGGLE |	OID_802_11_NETWORK_TYPE_IN_USE)
#define	RT_OID_802_11_TX_POWER_LEVEL		  (OID_GET_SET_TOGGLE |	OID_802_11_TX_POWER_LEVEL)
#define	RT_OID_802_11_RSSI_TRIGGER			  (OID_GET_SET_TOGGLE |	OID_802_11_RSSI_TRIGGER)
#define	RT_OID_802_11_FRAGMENTATION_THRESHOLD (OID_GET_SET_TOGGLE |	OID_802_11_FRAGMENTATION_THRESHOLD)
#define	RT_OID_802_11_RTS_THRESHOLD			  (OID_GET_SET_TOGGLE |	OID_802_11_RTS_THRESHOLD)
#define	RT_OID_802_11_RX_ANTENNA_SELECTED	  (OID_GET_SET_TOGGLE |	OID_802_11_RX_ANTENNA_SELECTED)
#define	RT_OID_802_11_TX_ANTENNA_SELECTED	  (OID_GET_SET_TOGGLE |	OID_802_11_TX_ANTENNA_SELECTED)
#define	RT_OID_802_11_SUPPORTED_RATES		  (OID_GET_SET_TOGGLE |	OID_802_11_SUPPORTED_RATES)
#define	RT_OID_802_11_DESIRED_RATES			  (OID_GET_SET_TOGGLE |	OID_802_11_DESIRED_RATES)
#define	RT_OID_802_11_CONFIGURATION			  (OID_GET_SET_TOGGLE |	OID_802_11_CONFIGURATION)
#define	RT_OID_802_11_POWER_MODE			  (OID_GET_SET_TOGGLE |	OID_802_11_POWER_MODE)

//
// IEEE 802.11 Structures and definitions
//
// new types for Media Specific Indications

#define NDIS_802_11_LENGTH_SSID         32
#define NDIS_802_11_LENGTH_RATES        8
#define NDIS_802_11_LENGTH_RATES_EX     16

typedef enum _NDIS_802_11_STATUS_TYPE
{
    Ndis802_11StatusType_Authentication,
    Ndis802_11StatusType_MediaStreamMode,
    Ndis802_11StatusType_PMKID_CandidateList,		
    Ndis802_11StatusTypeMax    // not a real type, defined as an upper bound
} NDIS_802_11_STATUS_TYPE, *PNDIS_802_11_STATUS_TYPE;

typedef struct _NDIS_802_11_STATUS_INDICATION
{
    NDIS_802_11_STATUS_TYPE StatusType;
} NDIS_802_11_STATUS_INDICATION, *PNDIS_802_11_STATUS_INDICATION;

// mask for authentication/integrity fields
#define NDIS_802_11_AUTH_REQUEST_AUTH_FIELDS        0x0f

#define NDIS_802_11_AUTH_REQUEST_REAUTH             0x01
#define NDIS_802_11_AUTH_REQUEST_KEYUPDATE          0x02
#define NDIS_802_11_AUTH_REQUEST_PAIRWISE_ERROR     0x06
#define NDIS_802_11_AUTH_REQUEST_GROUP_ERROR        0x0E

typedef struct _NDIS_802_11_AUTHENTICATION_REQUEST
{
    ULONG Length;            // Length of structure
    NDIS_802_11_MAC_ADDRESS Bssid;
    ULONG Flags;
} NDIS_802_11_AUTHENTICATION_REQUEST, *PNDIS_802_11_AUTHENTICATION_REQUEST;

//Added new types for PMKID Candidate lists.
typedef struct _PMKID_CANDIDATE {
    NDIS_802_11_MAC_ADDRESS BSSID;
    ULONG Flags;
} PMKID_CANDIDATE, *PPMKID_CANDIDATE;

typedef struct _NDIS_802_11_PMKID_CANDIDATE_LIST
{
    ULONG Version;       // Version of the structure
    ULONG NumCandidates; // No. of pmkid candidates
    PMKID_CANDIDATE CandidateList[1];
} NDIS_802_11_PMKID_CANDIDATE_LIST, *PNDIS_802_11_PMKID_CANDIDATE_LIST;

//Flags for PMKID Candidate list structure
#define NDIS_802_11_PMKID_CANDIDATE_PREAUTH_ENABLED	0x01

// Added new types for OFDM 5G and 2.4G
typedef enum _NDIS_802_11_NETWORK_TYPE
{
   Ndis802_11FH, 
   Ndis802_11DS, 
    Ndis802_11OFDM5,
    Ndis802_11OFDM5_N,
    Ndis802_11OFDM24,
    Ndis802_11OFDM24_N,
   Ndis802_11Automode,
    Ndis802_11NetworkTypeMax    // not a real type, defined as an upper bound
} NDIS_802_11_NETWORK_TYPE, *PNDIS_802_11_NETWORK_TYPE;

typedef struct _NDIS_802_11_NETWORK_TYPE_LIST
{
    UINT                       NumberOfItems;  // in list below, at least 1
   NDIS_802_11_NETWORK_TYPE    NetworkType [1];
} NDIS_802_11_NETWORK_TYPE_LIST, *PNDIS_802_11_NETWORK_TYPE_LIST;

typedef enum _NDIS_802_11_POWER_MODE
{
   Ndis802_11PowerModeCAM,
   Ndis802_11PowerModeMAX_PSP,
   Ndis802_11PowerModeFast_PSP,
    Ndis802_11PowerModeLegacy_PSP,
    Ndis802_11PowerModeMax      // not a real mode, defined as an upper bound
} NDIS_802_11_POWER_MODE, *PNDIS_802_11_POWER_MODE;

typedef ULONG   NDIS_802_11_TX_POWER_LEVEL; // in milliwatts

//
// Received Signal Strength Indication
//
typedef LONG    NDIS_802_11_RSSI;           // in dBm
#ifdef KEIL_COMPILE	
typedef struct PACKED _NDIS_802_11_CONFIGURATION_FH
#else
typedef struct _NDIS_802_11_CONFIGURATION_FH
#endif
{
   ULONG           Length;            // Length of structure
   ULONG           HopPattern;        // As defined by 802.11, MSB set 
   ULONG           HopSet;            // to one if non-802.11
   ULONG           DwellTime;         // units are Kusec
} NDIS_802_11_CONFIGURATION_FH, *PNDIS_802_11_CONFIGURATION_FH;

#ifdef KEIL_COMPILE	
typedef struct PACKED _NDIS_802_11_CONFIGURATION
#else
typedef struct _NDIS_802_11_CONFIGURATION
#endif
{
   ULONG                           Length;             // Length of structure
   ULONG                           BeaconPeriod;       // units are Kusec
   ULONG                           ATIMWindow;         // units are Kusec
   ULONG                           DSConfig;           // Frequency, units are kHz
   NDIS_802_11_CONFIGURATION_FH    FHConfig;
} NDIS_802_11_CONFIGURATION, *PNDIS_802_11_CONFIGURATION;

#ifdef IAR_COMPILE
PACKED typedef struct _NDIS_802_11_STATISTICS  {
#else
typedef struct	PACKED _NDIS_802_11_STATISTICS {
#endif
   ULONG           Length;             // Length of structure
	/* If not PACKED, MIPS will pad 4 bytes right here in 32-bit CPU */	
   LARGE_INTEGER   TransmittedFragmentCount;
   LARGE_INTEGER   MulticastTransmittedFrameCount;
   LARGE_INTEGER   FailedCount;
   LARGE_INTEGER   RetryCount;
   LARGE_INTEGER   MultipleRetryCount;
   LARGE_INTEGER   RTSSuccessCount;
   LARGE_INTEGER   RTSFailureCount;
   LARGE_INTEGER   ACKFailureCount;
   LARGE_INTEGER   FrameDuplicateCount;
   LARGE_INTEGER   ReceivedFragmentCount;
   LARGE_INTEGER   MulticastReceivedFrameCount;
   LARGE_INTEGER   FCSErrorCount;
   LARGE_INTEGER   TKIPLocalMICFailures;
   LARGE_INTEGER   TKIPRemoteMICErrors;
   LARGE_INTEGER   TKIPICVErrors;
   LARGE_INTEGER   TKIPCounterMeasuresInvoked;
   LARGE_INTEGER   TKIPReplays;
   LARGE_INTEGER   CCMPFormatErrors;
   LARGE_INTEGER   CCMPReplays;
   LARGE_INTEGER   CCMPDecryptErrors;
   LARGE_INTEGER   FourWayHandshakeFailures;   
} NDIS_802_11_STATISTICS, *PNDIS_802_11_STATISTICS;

typedef  ULONG  NDIS_802_11_KEY_INDEX;
typedef ULONGLONG   NDIS_802_11_KEY_RSC;
#if 0
#define MAX_RADIUS_SRV_NUM			2	  // 802.1x failover number

typedef struct PACKED _RADIUS_SRV_INFO {
	UINT32			radius_ip;
	UINT32			radius_port;
	UCHAR			radius_key[64];
	UCHAR			radius_key_len;
} RADIUS_SRV_INFO, *PRADIUS_SRV_INFO;

typedef struct PACKED _RADIUS_KEY_INFO
{
	UCHAR			radius_srv_num;			
	RADIUS_SRV_INFO	radius_srv_info[MAX_RADIUS_SRV_NUM];
	UCHAR			ieee8021xWEP;		 // dynamic WEP
    UCHAR           key_index;           
    UCHAR           key_length;          // length of key in bytes
    UCHAR           key_material[13];    
} RADIUS_KEY_INFO, *PRADIUS_KEY_INFO;

// It's used by 802.1x daemon to require relative configuration
typedef struct PACKED _RADIUS_CONF
{
    UINT32          Length;             // Length of this structure    
    UCHAR			mbss_num;			// indicate multiple BSS number 
	UINT32			own_ip_addr;	
	UINT32			retry_interval;
	UINT32			session_timeout_interval;
	UCHAR			EAPifname[IFNAMSIZ];
	UCHAR			EAPifname_len;
	UCHAR 			PreAuthifname[IFNAMSIZ];
	UCHAR			PreAuthifname_len;
	RADIUS_KEY_INFO	RadiusInfo[MAX_MBSSID_NUM];
} RADIUS_CONF, *PRADIUS_CONF;
#endif
#ifdef CONFIG_AP_SUPPORT
typedef struct _NDIS_AP_802_11_KEY
{
    UINT            Length;             // Length of this structure
    UCHAR			addr[6];
    UINT            KeyIndex;           
    UINT            KeyLength;          // length of key in bytes
    UCHAR           KeyMaterial[1];     // variable length depending on above field
} NDIS_AP_802_11_KEY, *PNDIS_AP_802_11_KEY;
#endif // CONFIG_AP_SUPPORT //

#ifdef APCLI_SUPPORT
#ifdef WPA_SUPPLICANT_SUPPORT
typedef struct _NDIS_APCLI_802_11_KEY
{
    UINT           Length;
    UINT           KeyIndex;
    UINT           KeyLength;
    NDIS_802_11_MAC_ADDRESS BSSID;
    NDIS_802_11_KEY_RSC KeyRSC;
    UCHAR           KeyMaterial[1];
} NDIS_APCLI_802_11_KEY, *PNDIS_APCLI_802_11_KEY;
#endif/* WPA_SUPPLICANT_SUPPORT */
#endif /* APCLI_SUPPORT */



#ifdef CONFIG_STA_SUPPORT
// Key mapping keys require a BSSID
#ifdef IAR_COMPILE
PACKED typedef struct _NDIS_802_11_KEY  {
#else
typedef struct	PACKED _NDIS_802_11_KEY {
#endif
    UINT           Length;             // Length of this structure
    UINT           KeyIndex;           
    UINT           KeyLength;          // length of key in bytes
    NDIS_802_11_MAC_ADDRESS BSSID;
    NDIS_802_11_KEY_RSC KeyRSC;
    UCHAR           KeyMaterial[1];     // variable length depending on above field
} NDIS_802_11_KEY, *PNDIS_802_11_KEY;
#endif // CONFIG_STA_SUPPORT //

typedef struct _NDIS_802_11_REMOVE_KEY
{
    UINT           Length;             // Length of this structure
    UINT           KeyIndex;           
    NDIS_802_11_MAC_ADDRESS BSSID;      
} NDIS_802_11_REMOVE_KEY, *PNDIS_802_11_REMOVE_KEY;

typedef struct _NDIS_802_11_WEP
{
   UINT     Length;        // Length of this structure
   UINT     KeyIndex;           // 0 is the per-client key, 1-N are the
                                        // global keys
   UINT     KeyLength;     // length of key in bytes
   UCHAR     KeyMaterial[1];// variable length depending on above field
} NDIS_802_11_WEP, *PNDIS_802_11_WEP;


typedef enum _NDIS_802_11_NETWORK_INFRASTRUCTURE
{
   Ndis802_11IBSS,
   Ndis802_11Infrastructure,
   Ndis802_11AutoUnknown,
   Ndis802_11Monitor,
   Ndis802_11InfrastructureMax     // Not a real value, defined as upper bound
} NDIS_802_11_NETWORK_INFRASTRUCTURE, *PNDIS_802_11_NETWORK_INFRASTRUCTURE;

// Add new authentication modes
typedef enum _NDIS_802_11_AUTHENTICATION_MODE
{
   Ndis802_11AuthModeOpen,
   Ndis802_11AuthModeShared,
   Ndis802_11AuthModeAutoSwitch,
    Ndis802_11AuthModeWPA,
    Ndis802_11AuthModeWPAPSK,
    Ndis802_11AuthModeWPANone,
   Ndis802_11AuthModeWPA2,
   Ndis802_11AuthModeWPA2PSK,    
   	Ndis802_11AuthModeWPA1WPA2,
	Ndis802_11AuthModeWPA1PSKWPA2PSK,
   Ndis802_11AuthModeMax           // Not a real mode, defined as upper bound
} NDIS_802_11_AUTHENTICATION_MODE, *PNDIS_802_11_AUTHENTICATION_MODE;

typedef UCHAR   NDIS_802_11_RATES[NDIS_802_11_LENGTH_RATES];        // Set of 8 data rates
typedef UCHAR   NDIS_802_11_RATES_EX[NDIS_802_11_LENGTH_RATES_EX];  // Set of 16 data rates

#ifdef IAR_COMPILE
PACKED typedef struct _NDIS_802_11_SSID  {
#else
typedef struct	PACKED _NDIS_802_11_SSID {
#endif
    UINT   SsidLength;         // length of SSID field below, in bytes;
                                // this can be zero.
    UCHAR   Ssid[NDIS_802_11_LENGTH_SSID];           // SSID information field
} NDIS_802_11_SSID, *PNDIS_802_11_SSID;

#ifdef IAR_COMPILE
PACKED typedef struct _NDIS_WLAN_BSSID  {
#else
typedef struct	PACKED _NDIS_WLAN_BSSID {
#endif
   ULONG                               Length;     // Length of this structure
   NDIS_802_11_MAC_ADDRESS             MacAddress; // BSSID
   UCHAR                               Reserved[2];
   NDIS_802_11_SSID                    Ssid;       // SSID
   ULONG                               Privacy;    // WEP encryption requirement
   NDIS_802_11_RSSI                    Rssi;       // receive signal strength in dBm
   NDIS_802_11_NETWORK_TYPE            NetworkTypeInUse;
   NDIS_802_11_CONFIGURATION           Configuration;
   NDIS_802_11_NETWORK_INFRASTRUCTURE  InfrastructureMode;
   NDIS_802_11_RATES                   SupportedRates;
} NDIS_WLAN_BSSID, *PNDIS_WLAN_BSSID;

#ifdef IAR_COMPILE
PACKED typedef struct _NDIS_802_11_BSSID_LIST  {
#else
typedef struct	PACKED _NDIS_802_11_BSSID_LIST {
#endif
   UINT           NumberOfItems;      // in list below, at least 1
   NDIS_WLAN_BSSID Bssid[1];
} NDIS_802_11_BSSID_LIST, *PNDIS_802_11_BSSID_LIST;

// Added Capabilities, IELength and IEs for each BSSID
#ifdef IAR_COMPILE
PACKED typedef struct _NDIS_WLAN_BSSID_EX  {
#else
typedef struct	PACKED _NDIS_WLAN_BSSID_EX {
#endif
    ULONG                               Length;             // Length of this structure
    NDIS_802_11_MAC_ADDRESS             MacAddress;         // BSSID
    UCHAR                               Reserved[2];
    NDIS_802_11_SSID                    Ssid;               // SSID
    UINT                                Privacy;            // WEP encryption requirement
    NDIS_802_11_RSSI                    Rssi;               // receive signal
                                                            // strength in dBm
    NDIS_802_11_NETWORK_TYPE            NetworkTypeInUse;
    NDIS_802_11_CONFIGURATION           Configuration;
    NDIS_802_11_NETWORK_INFRASTRUCTURE  InfrastructureMode;
    NDIS_802_11_RATES_EX                SupportedRates;
    ULONG                               IELength;
    UCHAR                               IEs[1];
} NDIS_WLAN_BSSID_EX, *PNDIS_WLAN_BSSID_EX;

#ifdef IAR_COMPILE
PACKED typedef struct _NDIS_802_11_BSSID_LIST_EX  {
#else
typedef struct	PACKED _NDIS_802_11_BSSID_LIST_EX {
#endif
    UINT                   NumberOfItems;      // in list below, at least 1
    NDIS_WLAN_BSSID_EX      Bssid[1];
} NDIS_802_11_BSSID_LIST_EX, *PNDIS_802_11_BSSID_LIST_EX;

#ifdef IAR_COMPILE
PACKED typedef struct _NDIS_802_11_FIXED_IEs  {
#else
typedef struct	PACKED _NDIS_802_11_FIXED_IEs {
#endif
    UCHAR Timestamp[8];
    USHORT BeaconInterval;
    USHORT Capabilities;
} NDIS_802_11_FIXED_IEs, *PNDIS_802_11_FIXED_IEs;

typedef struct _NDIS_802_11_VARIABLE_IEs 
{
    UCHAR ElementID;
    UCHAR Length;    // Number of bytes in data field
    UCHAR data[1];
} NDIS_802_11_VARIABLE_IEs, *PNDIS_802_11_VARIABLE_IEs;

typedef  ULONG   NDIS_802_11_FRAGMENTATION_THRESHOLD;

typedef  ULONG   NDIS_802_11_RTS_THRESHOLD;

typedef  ULONG   NDIS_802_11_ANTENNA;

typedef enum _NDIS_802_11_PRIVACY_FILTER
{
   Ndis802_11PrivFilterAcceptAll,
   Ndis802_11PrivFilter8021xWEP
} NDIS_802_11_PRIVACY_FILTER, *PNDIS_802_11_PRIVACY_FILTER;

// Added new encryption types
// Also aliased typedef to new name
typedef enum _NDIS_802_11_WEP_STATUS {
   Ndis802_11WEPEnabled,
    Ndis802_11Encryption1Enabled = Ndis802_11WEPEnabled,
   Ndis802_11WEPDisabled,
    Ndis802_11EncryptionDisabled = Ndis802_11WEPDisabled,
   Ndis802_11WEPKeyAbsent,
    Ndis802_11Encryption1KeyAbsent = Ndis802_11WEPKeyAbsent,
   Ndis802_11WEPNotSupported,
    Ndis802_11EncryptionNotSupported = Ndis802_11WEPNotSupported,
	Ndis802_11TKIPEnable,
	Ndis802_11Encryption2Enabled = Ndis802_11TKIPEnable,
    Ndis802_11Encryption2KeyAbsent,
	Ndis802_11AESEnable,
	Ndis802_11Encryption3Enabled = Ndis802_11AESEnable,
    Ndis802_11Encryption3KeyAbsent,
	Ndis802_11TKIPAESMix,
	Ndis802_11Encryption4Enabled = Ndis802_11TKIPAESMix,	/* TKIP or AES mix */
    Ndis802_11Encryption4KeyAbsent,
	Ndis802_11GroupWEP40Enabled,
	Ndis802_11GroupWEP104Enabled,
#ifdef WAPI_SUPPORT
	Ndis802_11EncryptionSMS4Enabled,	/* WPI SMS4 support */
#endif /* WAPI_SUPPORT */
} NDIS_802_11_WEP_STATUS, *PNDIS_802_11_WEP_STATUS, NDIS_802_11_ENCRYPTION_STATUS, *PNDIS_802_11_ENCRYPTION_STATUS;

typedef enum _NDIS_802_11_RELOAD_DEFAULTS
{
   Ndis802_11ReloadWEPKeys
} NDIS_802_11_RELOAD_DEFAULTS, *PNDIS_802_11_RELOAD_DEFAULTS;

#define NDIS_802_11_AI_REQFI_CAPABILITIES      1
#define NDIS_802_11_AI_REQFI_LISTENINTERVAL    2
#define NDIS_802_11_AI_REQFI_CURRENTAPADDRESS  4

#define NDIS_802_11_AI_RESFI_CAPABILITIES      1
#define NDIS_802_11_AI_RESFI_STATUSCODE        2
#define NDIS_802_11_AI_RESFI_ASSOCIATIONID     4

typedef struct _NDIS_802_11_AI_REQFI
{
    USHORT Capabilities;
    USHORT ListenInterval;
    NDIS_802_11_MAC_ADDRESS  CurrentAPAddress;
} NDIS_802_11_AI_REQFI, *PNDIS_802_11_AI_REQFI;

typedef struct _NDIS_802_11_AI_RESFI
{
    USHORT Capabilities;
    USHORT StatusCode;
    USHORT AssociationId;
} NDIS_802_11_AI_RESFI, *PNDIS_802_11_AI_RESFI;

typedef struct _NDIS_802_11_ASSOCIATION_INFORMATION
{
    ULONG                   Length;
    USHORT                  AvailableRequestFixedIEs;
    NDIS_802_11_AI_REQFI    RequestFixedIEs;
    ULONG                   RequestIELength;
    ULONG                   OffsetRequestIEs;
    USHORT                  AvailableResponseFixedIEs;
    NDIS_802_11_AI_RESFI    ResponseFixedIEs;
    ULONG                   ResponseIELength;
    ULONG                   OffsetResponseIEs;
} NDIS_802_11_ASSOCIATION_INFORMATION, *PNDIS_802_11_ASSOCIATION_INFORMATION;

typedef struct _NDIS_802_11_AUTHENTICATION_EVENT
{
    NDIS_802_11_STATUS_INDICATION       Status;
    NDIS_802_11_AUTHENTICATION_REQUEST  Request[1];
} NDIS_802_11_AUTHENTICATION_EVENT, *PNDIS_802_11_AUTHENTICATION_EVENT;
        
typedef struct _NDIS_802_11_TEST
{
    ULONG Length;
    ULONG Type;
#ifndef KEIL_COMPILE		
    union
    {
#endif	
        NDIS_802_11_AUTHENTICATION_EVENT AuthenticationEvent;
        NDIS_802_11_RSSI RssiTrigger;
#ifndef KEIL_COMPILE			
    };
#endif	
} NDIS_802_11_TEST, *PNDIS_802_11_TEST;

// 802.11 Media stream constraints, associated with OID_802_11_MEDIA_STREAM_MODE
typedef enum _NDIS_802_11_MEDIA_STREAM_MODE
{
    Ndis802_11MediaStreamOff,
    Ndis802_11MediaStreamOn,
} NDIS_802_11_MEDIA_STREAM_MODE, *PNDIS_802_11_MEDIA_STREAM_MODE;

// PMKID Structures
typedef UCHAR   NDIS_802_11_PMKID_VALUE[16];

#if defined(CONFIG_STA_SUPPORT) || defined(WPA_SUPPLICANT_SUPPORT)
typedef struct _BSSID_INFO {
    NDIS_802_11_MAC_ADDRESS BSSID;
    NDIS_802_11_PMKID_VALUE PMKID;
} BSSID_INFO, *PBSSID_INFO;

typedef struct _NDIS_802_11_PMKID {
    UINT    Length;
    UINT    BSSIDInfoCount;
    BSSID_INFO BSSIDInfo[1];
} NDIS_802_11_PMKID, *PNDIS_802_11_PMKID;
#endif /* defined(CONFIG_STA_SUPPORT) || defined(WPA_SUPPLICANT_SUPPORT) */

#ifdef CONFIG_AP_SUPPORT
#ifdef APCLI_SUPPORT
#ifdef WPA_SUPPLICANT_SUPPORT
typedef struct _NDIS_APCLI_802_11_PMKID
{
    UINT    Length;
    UINT    BSSIDInfoCount;
    BSSID_INFO BSSIDInfo[1];
} NDIS_APCLI_802_11_PMKID, *PNDIS_APCLI_802_11_PMKID;
#endif/*WPA_SUPPLICANT_SUPPORT*/
#endif /* APCLI_SUPPORT */

typedef struct _AP_BSSID_INFO {
	NDIS_802_11_MAC_ADDRESS		MAC;
	NDIS_802_11_PMKID_VALUE		PMKID;
	UCHAR                       PMK[32];
	ULONG						RefreshTime;
	BOOLEAN						Valid;
} AP_BSSID_INFO, *PAP_BSSID_INFO;

#define MAX_PMKID_COUNT		8
typedef struct _NDIS_AP_802_11_PMKID {
	AP_BSSID_INFO BSSIDInfo[MAX_PMKID_COUNT];
} NDIS_AP_802_11_PMKID, *PNDIS_AP_802_11_PMKID;
#endif /* CONFIG_AP_SUPPORT */


typedef struct _NDIS_802_11_AUTHENTICATION_ENCRYPTION
{
    NDIS_802_11_AUTHENTICATION_MODE AuthModeSupported;
    NDIS_802_11_ENCRYPTION_STATUS EncryptStatusSupported;
} NDIS_802_11_AUTHENTICATION_ENCRYPTION, *PNDIS_802_11_AUTHENTICATION_ENCRYPTION;

typedef struct _NDIS_802_11_CAPABILITY
{
     ULONG Length;
     ULONG Version;
     ULONG NoOfPMKIDs;
     ULONG NoOfAuthEncryptPairsSupported;
     NDIS_802_11_AUTHENTICATION_ENCRYPTION AuthenticationEncryptionSupported[1];
} NDIS_802_11_CAPABILITY, *PNDIS_802_11_CAPABILITY;

//#endif //of WIN 2k
#endif //UNDER_CE


//#define RT_OID_802_11_STATISTICS              (OID_GET_SET_TOGGLE | OID_802_11_STATISTICS)

#ifdef CONFIG_STA_SUPPORT
#define RT_OID_WSC_SET_PASSPHRASE                   0x0740 // passphrase for wpa(2)-psk
#define RT_OID_WSC_DRIVER_AUTO_CONNECT              0x0741
#define RT_OID_WSC_QUERY_DEFAULT_PROFILE            0x0742
#define RT_OID_WSC_SET_CONN_BY_PROFILE_INDEX        0x0743
#define RT_OID_WSC_SET_ACTION                       0x0744
#define RT_OID_WSC_SET_SSID                         0x0745
#define RT_OID_WSC_SET_PIN_CODE                     0x0746
#define RT_OID_WSC_SET_MODE                         0x0747 // PIN or PBC
#define RT_OID_WSC_SET_CONF_MODE                    0x0748 // Enrollee or Registrar
#define RT_OID_WSC_SET_PROFILE                      0x0749
#endif // CONFIG_STA_SUPPORT //

#define RT_OID_802_11_WSC_QUERY_PROFILE				0x0750
// for consistency with RT61
#define RT_OID_WSC_QUERY_STATUS						0x0751
#define RT_OID_WSC_PIN_CODE							0x0752
#define RT_OID_WSC_UUID								0x0753
#define RT_OID_WSC_SET_SELECTED_REGISTRAR			0x0754
#define RT_OID_WSC_EAPMSG							0x0755
#define RT_OID_WSC_MANUFACTURER						0x0756
#define RT_OID_WSC_MODEL_NAME						0x0757
#define RT_OID_WSC_MODEL_NO							0x0758
#define RT_OID_WSC_SERIAL_NO						0x0759
#define RT_OID_WSC_MAC_ADDRESS						0x0760

#ifdef LLTD_SUPPORT
// for consistency with RT61
#define RT_OID_GET_PHY_MODE                         0x761
#ifdef CONFIG_AP_SUPPORT
#define RT_OID_GET_LLTD_ASSO_TABLE                  0x762
#ifdef APCLI_SUPPORT
#define RT_OID_GET_REPEATER_AP_LINEAGE				0x763
#endif // APCLI_SUPPORT //
#endif // CONFIG_AP_SUPPORT //
#endif // LLTD_SUPPORT //

#define MAX_NUMBER_OF_MAC				14 //TODO: Raghav: check the correct value

#ifdef NINTENDO_AP
//#define RT_OID_NINTENDO                             0x0D010770 
#define RT_OID_802_11_NINTENDO_GET_TABLE			0x0771 //((RT_OID_NINTENDO + 0x01) & 0xffff)
#define RT_OID_802_11_NINTENDO_SET_TABLE			0x0772 //((RT_OID_NINTENDO + 0x02) & 0xffff)
#define RT_OID_802_11_NINTENDO_CAPABLE				0x0773 //((RT_OID_NINTENDO + 0x03) & 0xffff)
#endif // NINTENDO_AP //

//Add Paul Chen for Accton
//#define RT_OID_TX_POWER_LEVEL                 0xFF020010
//#define RT_OID_SET_TX_POWER_LEVEL	          (OID_GET_SET_TOGGLE | RT_OID_TX_POWER_LEVEL)

// New for MeetingHouse Api support
#define OID_MH_802_1X_SUPPORTED               0xFFEDC100


typedef enum _RT_802_11_PREAMBLE {
    Rt802_11PreambleLong,
    Rt802_11PreambleShort,
    Rt802_11PreambleAuto
} RT_802_11_PREAMBLE, *PRT_802_11_PREAMBLE;

// Only for STA, need to sync with AP
#if 0
// 2005-03-08 match current ApConfig
typedef enum _RT_802_11_PHY_MODE {
    PHY_11BG_MIXED,
    PHY_11B,
    PHY_11G,        // current STA has no G-only mode. that's the problem.
    PHY_11A,
    PHY_11ABG_MIXED
} RT_802_11_PHY_MODE;
#else
// 2005-03-08 match current RaConfig. 
typedef enum _RT_802_11_PHY_MODE {
	PHY_11BG_MIXED = 0,
	PHY_11B,
	PHY_11A,
	PHY_11ABG_MIXED,
	PHY_11G,
	PHY_11ABGN_MIXED,	// both band   5
	PHY_11N_2_4G,		// 11n-only with 2.4G band   	6
	PHY_11GN_MIXED,	// 2.4G band      7
	PHY_11AN_MIXED,	// 5G  band       8
	PHY_11BGN_MIXED,	// if check 802.11b.      9
	PHY_11AGN_MIXED,	// if check 802.11b.      10
	PHY_11N_5G,			// 11n-only with 5G band		11
	PHY_MODE_MAX
} RT_802_11_PHY_MODE;
#endif

typedef enum _RT_802_11_ADHOC_MODE {
	ADHOC_11B,
	ADHOC_11BG_MIXED,
	ADHOC_11G,
	ADHOC_11A,
	ADHOC_11ABG_MIXED
} RT_802_11_ADHOC_MODE;

// put all proprietery for-query objects here to reduce # of Query_OID
typedef struct _RT_802_11_LINK_STATUS {
    ULONG   CurrTxRate;         // in units of 0.5Mbps
    ULONG   ChannelQuality;     // 0..100 %
    ULONG   TxByteCount;        // both ok and fail
    ULONG   RxByteCount;        // both ok and fail
    ULONG	CentralChannel;		// 40MHz central channel number
} RT_802_11_LINK_STATUS, *PRT_802_11_LINK_STATUS;

typedef struct _RT_802_11_EVENT_LOG {
    LARGE_INTEGER   SystemTime;  // timestammp via NdisGetCurrentSystemTime()
    UCHAR           Addr[MAC_ADDR_LEN];
    USHORT          Event;       // EVENT_xxx
} RT_802_11_EVENT_LOG, *PRT_802_11_EVENT_LOG;

typedef struct _RT_802_11_EVENT_TABLE {
    ULONG       Num;
    ULONG       Rsv;     // to align Log[] at LARGE_INEGER boundary
    RT_802_11_EVENT_LOG   Log[MAX_NUM_OF_EVENT];
} RT_802_11_EVENT_TABLE, PRT_802_11_EVENT_TABLE;

// MIMO Tx parameter, ShortGI, MCS, STBC, etc.  these are fields in TXWI. Don't change this definition!!!
#ifdef IAR_COMPILE
PACKED typedef union   _MACHTTRANSMIT_SETTING {
#else
typedef union  PACKED _MACHTTRANSMIT_SETTING {
#endif 
#ifdef KEIL_COMPILE
	struct	PACKED{
#else
	struct	{
#endif	
	USHORT   	MCS:7;                 // MCS
	USHORT		BW:1;	//channel bandwidth 20MHz or 40 MHz
	USHORT		ShortGI:1;
	USHORT		STBC:2;	//SPACE 
	USHORT		rsv:3;	 
	USHORT		MODE:2;	// Use definition MODE_xxx.  
	}	field;
	USHORT		word;
 } MACHTTRANSMIT_SETTING, *PMACHTTRANSMIT_SETTING;

 
#ifdef IAR_COMPILE
PACKED typedef struct _RT_802_11_MAC_ENTRY  {
#else
typedef struct	PACKED _RT_802_11_MAC_ENTRY {
#endif
    UCHAR       Addr[MAC_ADDR_LEN];
    UCHAR       Aid;
    UCHAR       Psm;     // 0:PWR_ACTIVE, 1:PWR_SAVE
    UCHAR		MimoPs;  // 0:MMPS_STATIC, 1:MMPS_DYNAMIC, 3:MMPS_Enabled
	UCHAR		apidx;
    MACHTTRANSMIT_SETTING	TxRate;
} RT_802_11_MAC_ENTRY, *PRT_802_11_MAC_ENTRY;
#if LATER
typedef struct PACKED _RT_802_11_MAC_TABLE {
    ULONG       Num;
    RT_802_11_MAC_ENTRY Entry[MAX_LEN_OF_MAC_TABLE(pAd)];
} RT_802_11_MAC_TABLE, *PRT_802_11_MAC_TABLE;
#endif

#ifdef CONFIG_AP_SUPPORT

#ifdef IAR_COMPILE
PACKED typedef struct __RT_MBSS_STAT_ENTRY  {
#else
typedef struct	PACKED __RT_MBSS_STAT_ENTRY {
#endif
	UINT32 RxCount;
	UINT32 TxCount;
	UINT32 ReceivedByteCount;
	UINT32 TransmittedByteCount;
	UINT32 RxErrorCount;
	UINT32 RxDropCount;
} RT_MBSS_STAT_ENTRY;

#ifdef IAR_COMPILE
PACKED typedef struct __RT_MBSS_STATISTICS_TABLE  {
#else
typedef struct	PACKED __RT_MBSS_STATISTICS_TABLE {
#endif
	int Num;
	RT_MBSS_STAT_ENTRY	MbssEntry[1];
} RT_MBSS_STATISTICS_TABLE;
#endif // #ifdef CONFIG_AP_SUPPORT //



// structure for query/set hardware register - MAC, BBP, RF register
typedef struct _RT_802_11_HARDWARE_REGISTER {
    ULONG   HardwareType;       // 0:MAC, 1:BBP, 2:RF register, 3:EEPROM
    ULONG   Offset;             // Q/S register offset addr
    ULONG   Data;               // R/W data buffer
} RT_802_11_HARDWARE_REGISTER, *PRT_802_11_HARDWARE_REGISTER;

// structure to tune BBP R17 "RX AGC VGC init"
//typedef struct _RT_802_11_RX_AGC_VGC_TUNING {
//    UCHAR   FalseCcaLowerThreshold;  // 0-255, def 10
//    UCHAR   FalseCcaUpperThreshold;  // 0-255, def 100
//    UCHAR   VgcDelta;                // R17 +-= VgcDelta whenever flase CCA over UpprThreshold
//                                     // or lower than LowerThresholdupper threshold
//    UCHAR   VgcUpperBound;           // max value of R17
//} RT_802_11_RX_AGC_VGC_TUNING, *PRT_802_11_RX_AGC_VGC_TUNING;

typedef struct _RT_802_11_AP_CONFIG {
    ULONG   EnableTxBurst;      // 0-disable, 1-enable
    ULONG   EnableTurboRate;    // 0-disable, 1-enable 72/100mbps turbo rate
    ULONG   IsolateInterStaTraffic;     // 0-disable, 1-enable isolation
    ULONG   HideSsid;           // 0-disable, 1-enable hiding
    ULONG   UseBGProtection;    // 0-AUTO, 1-always ON, 2-always OFF
    ULONG   UseShortSlotTime;   // 0-no use, 1-use 9-us short slot time
    ULONG   Rsv1;               // must be 0
    ULONG   SystemErrorBitmap;  // ignore upon SET, return system error upon QUERY
} RT_802_11_AP_CONFIG, *PRT_802_11_AP_CONFIG;

// structure to query/set STA_CONFIG
typedef struct _RT_802_11_STA_CONFIG {
    ULONG   EnableTxBurst;      // 0-disable, 1-enable
    ULONG   EnableTurboRate;    // 0-disable, 1-enable 72/100mbps turbo rate
    ULONG   UseBGProtection;    // 0-AUTO, 1-always ON, 2-always OFF
    ULONG   UseShortSlotTime;   // 0-no use, 1-use 9-us short slot time when applicable
    ULONG   AdhocMode; 			// 0-11b rates only (WIFI spec), 1 - b/g mixed, 2 - g only
    ULONG   HwRadioStatus;      // 0-OFF, 1-ON, default is 1, Read-Only
    ULONG   Rsv1;               // must be 0
    ULONG   SystemErrorBitmap;  // ignore upon SET, return system error upon QUERY
} RT_802_11_STA_CONFIG, *PRT_802_11_STA_CONFIG;

#ifdef KEIL_COMPILE
typedef struct PACKED _RT_802_11_ACL_ENTRY {
#else
typedef struct _RT_802_11_ACL_ENTRY {
#endif
    UCHAR   Addr[MAC_ADDR_LEN];
    USHORT  Rsv;
} RT_802_11_ACL_ENTRY, *PRT_802_11_ACL_ENTRY;

#ifdef IAR_COMPILE
PACKED typedef struct _RT_802_11_ACL  {
#else
typedef struct	PACKED _RT_802_11_ACL {
#endif
    ULONG   Policy;             // 0-disable, 1-positive list, 2-negative list
    ULONG   Num;
    RT_802_11_ACL_ENTRY Entry[MAX_NUM_OF_ACL_LIST];
} RT_802_11_ACL, *PRT_802_11_ACL;
#if 0
typedef struct _RT_802_11_WDS {
    ULONG						Num;
    NDIS_802_11_MAC_ADDRESS		Entry[MAX_NUM_OF_WDS_LINK];
	ULONG						KeyLength;
	UCHAR						KeyMaterial[32];
} RT_802_11_WDS, *PRT_802_11_WDS;
#endif
typedef struct _RT_802_11_TX_RATES_ {
    UCHAR       SupRateLen;	
    UCHAR       SupRate[MAX_LEN_OF_SUPPORTED_RATES];
    UCHAR       ExtRateLen;	
    UCHAR       ExtRate[MAX_LEN_OF_SUPPORTED_RATES];
} RT_802_11_TX_RATES, *PRT_802_11_TX_RATES;


// Definition of extra information code
#define	GENERAL_LINK_UP			0x0			// Link is Up
#define	GENERAL_LINK_DOWN		0x1			// Link is Down
#define	HW_RADIO_OFF			0x2			// Hardware radio off
#define	SW_RADIO_OFF			0x3			// Software radio off
#define	AUTH_FAIL				0x4			// Open authentication fail
#define	AUTH_FAIL_KEYS			0x5			// Shared authentication fail
#define	ASSOC_FAIL				0x6			// Association failed
#define	EAP_MIC_FAILURE			0x7			// Deauthencation because MIC failure
#define	EAP_4WAY_TIMEOUT		0x8			// Deauthencation on 4-way handshake timeout
#define	EAP_GROUP_KEY_TIMEOUT	0x9			// Deauthencation on group key handshake timeout
#define	EAP_SUCCESS				0xa			// EAP succeed
#define	DETECT_RADAR_SIGNAL		0xb         // Radar signal occur in current channel

#define EXTRA_INFO_CLEAR		0xffffffff

#ifdef NINTENDO_AP
#define NINTENDO_MAX_ENTRY 16
#define NINTENDO_SSID_NAME_LN 8
#define NINTENDO_SSID_NAME "NWCUSBAP"
#define NINTENDO_PROBE_REQ_FLAG_MASK 0x03
#define NINTENDO_PROBE_REQ_ON 0x01
#define NINTENDO_PROBE_REQ_SIGNAL 0x02
#define NINTENDO_PROBE_RSP_ON 0x01
#define NINTENDO_SSID_NICKNAME_LN 20

#define NINTENDO_WEPKEY_LN 13

typedef struct _NINTENDO_SSID 
{
	UCHAR	NINTENDOFixChar[NINTENDO_SSID_NAME_LN];
	UCHAR	zero1;
	UCHAR	registe;
	UCHAR	ID;
	UCHAR	zero2;
	UCHAR	NICKname[NINTENDO_SSID_NICKNAME_LN];
} RT_NINTENDO_SSID, *PRT_NINTENDO_SSID;

typedef struct _NINTENDO_ENTRY 
{
	UCHAR	NICKname[NINTENDO_SSID_NICKNAME_LN];
    UCHAR   DS_Addr[ETH_LENGTH_OF_ADDRESS];
	UCHAR	registe;
	UCHAR	UserSpaceAck;
} RT_NINTENDO_ENTRY, *PRT_NINTENDO_ENTRY;

//RTPRIV_IOCTL_NINTENDO_GET_TABLE
//RTPRIV_IOCTL_NINTENDO_SET_TABLE
typedef struct _NINTENDO_TABLE 
{
	UINT				number;
	RT_NINTENDO_ENTRY	entry[NINTENDO_MAX_ENTRY];
} RT_NINTENDO_TABLE, *PRT_NINTENDO_TABLE;

//RTPRIV_IOCTL_NINTENDO_SEED_WEPKEY
typedef struct _NINTENDO_SEED_WEPKEY 
{
	UCHAR	seed[NINTENDO_SSID_NICKNAME_LN];
	UCHAR	wepkey[16];//use 13 for 104 bits wep key
} RT_NINTENDO_SEED_WEPKEY, *PRT_NINTENDO_SEED_WEPKEY;
#endif // NINTENDO_AP //

#ifdef LLTD_SUPPORT
typedef struct _RT_LLTD_ASSOICATION_ENTRY {
    UCHAR           Addr[ETH_LENGTH_OF_ADDRESS];
    unsigned short  MOR;        // maximum operational rate
    UCHAR           phyMode;
} RT_LLTD_ASSOICATION_ENTRY, *PRT_LLTD_ASSOICATION_ENTRY;
#ifdef LATER
typedef struct _RT_LLTD_ASSOICATION_TABLE {
    unsigned int                Num;
    RT_LLTD_ASSOICATION_ENTRY   Entry[MAX_LEN_OF_MAC_TABLE];
} RT_LLTD_ASSOICATION_TABLE, *PRT_LLTD_ASSOICATION_TABLE;
#endif
#endif // LLTD_SUPPORT //

#ifdef CONFIG_STA_SUPPORT
#if 0
//rt2860, kathy 2007-0118
// structure for DLS
typedef struct _RT_802_11_DLS_UI {
	USHORT						TimeOut;		// unit: second , set by UI
	USHORT						CountDownTimer;	// unit: second , used by driver only
	NDIS_802_11_MAC_ADDRESS		MacAddr;		// set by UI
	UCHAR						Status;			// 0: none , 1: wait STAkey, 2: finish DLS setup , set by driver only
	BOOLEAN						Valid;			// 1: valid , 0: invalid , set by UI, use to setup or tear down DLS link
} RT_802_11_DLS_UI, *PRT_802_11_DLS_UI;

// structure for DLS
typedef struct _RT_802_11_DLS {
	USHORT						TimeOut;		// Use to time out while slience, unit: second , set by UI
	USHORT						CountDownTimer;	// Use to time out while slience,unit: second , used by driver only
	NDIS_802_11_MAC_ADDRESS		MacAddr;		// set by UI
	UCHAR						Status;			// 0: none , 1: wait STAkey, 2: finish DLS setup , set by driver only
	BOOLEAN						Valid;			// 1: valid , 0: invalid , set by UI, use to setup or tear down DLS link
	RALINK_TIMER_STRUCT			Timer;			// Use to time out while handshake
	PVOID						pAd;
} RT_802_11_DLS, *PRT_802_11_DLS;

typedef enum _RT_802_11_DLS_MODE {
    DLS_NONE,
    DLS_WAIT_KEY,
    DLS_FINISH
} RT_802_11_DLS_MODE;
#endif

#endif // CONFIG_STA_SUPPORT //

//#ifdef WPA_SUPPLICANT_SUPPORT
//#ifndef NATIVE_WPA_SUPPLICANT_SUPPORT
#define	RT_ASSOC_EVENT_FLAG                         0x0101
#define	RT_DISASSOC_EVENT_FLAG                      0x0102
#define	RT_REQIE_EVENT_FLAG                         0x0103
#define	RT_RESPIE_EVENT_FLAG                        0x0104
#define	RT_ASSOCINFO_EVENT_FLAG                     0x0105
#define RT_PMKIDCAND_FLAG                           0x0106
#define RT_INTERFACE_DOWN                           0x0107
#define RT_INTERFACE_UP                             0x0108

//#endif // NATIVE_WPA_SUPPLICANT_SUPPORT //
//#endif // WPA_SUPPLICANT_SUPPORT //


#define MAX_CUSTOM_LEN 64 

#endif // _OID_H_

