/* Copyright Statement:
 *
 * (C) 2017-2017  MediaTek Inc. All rights reserved.
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


#ifndef __HAL_BWCS_REGS_H__
#define __HAL_BWCS_REGS_H__


typedef enum {
    BWCS_1_WIRE_MODE = 0,
    BWCS_2_WIRE_MODE,
    BWCS_3_WIRE_MODE,
    BWCS_4_WIRE_MODE,
    BWCS_1_WIRE_EXTEND_MODE,
    BWCS_2_WIRE_ESI_MODE,
    BWCS_1_WIRE_ESI_MODE,
    BWCS_WIRE_NUM
} BWCS_PTA_mode;

#define OUTBAND_ARB_MODE					1
#define INBAND_ARB_MODE						0

#define DEFAULT_RW_RETRY_LIMIT                             0x0
#define DEFAULT_RW_RETRY_LIMIT_BT_ACL               0x3

#define DEFAULT_RW_QNULL_EXTRA_RSVD                   0x2                   //in 32us unit

#define DEFAULT_RW_QNULL_SILENCE_WINDOW                        0x29   //1280us + 32us in 32us unit
#define DEFAULT_RW_QNULL_SILENCE_WINDOW_LOW_RATE     0x29   //1280us + 32us in 32us unit
#define DEFAULT_RW_QNULL_SILENCE_WINDOW_BT_ACL          0x9D   //4992us + 32us in 32us unit


#define DEFAULT_RW_CTS2SELF_SILENCE_WINDOW                      0x15   //640us + 32us in 32us unit
#define DEFAULT_RW_CTS2SELF_SILENCE_WINDOW_LOW_RATE  0x15    //960us  in 32us unit

#define DEFAULT_RW_CTS2SELF_DURATION                                         0x28 //1280us
#define DEFAULT_RW_CTS2SELF_DURATION_LOWRATE                        0x28 
#define DEFAULT_RW_CTS2SELF_DURATION_ESCO                               0x4F //2528us
#define DEFAULT_RW_CTS2SELF_DURATION_LONG                               0x271       //20ms


#define WNDRV_CUST_BT_1WIRE_MODE_T6                 (8)
#define WNDRV_CUST_BT_1WIRE_MODE_T8                 (110)
#define WNDRV_CUST_BT_1WIRE_MODE_BT_DELAY           (26)

#define COEX_MSG(X)                                                 ((X) & 0xFFFF0000)
#define COEX_VALUE(X)                                             ((X) & 0x0000FFFF)
#define COEX_PROFILE(X)                                          ((X) & 0x000000FF)
#define COEX_BT_SLOT(X)                                         ((X) & 0x0000FF00)

#define VAR_WMT_CONFIG_MAX_BT_TX_PWR                0x00020000  // s_int8
#define VAR_WMT_CONFIG_BT_TPCTL                     0x00040000
#define VAR_WMT_CONFIG_MAX_WIFI_TX_PWR              0x01000000  // s_int8
#define VAR_WMT_CONFIG_WIFI_TX_LIMIT                0x02000000
#define VAR_WMT_CONFIG_WIFI_CHANNEL_INFO            0x04000000
#define VAR_WMT_CONFIG_PORT2_PTA_CTRL               0x08000000

#define VAR_WMT_CONFIG_WIFI_1WIRE_MODE              0x20000000  //bit 0 ~ 3: antsel 0, bit 7 ~ 7: antsel 1
#define VAR_1WIRE_MODE_7                            0x0700
#define VAR_1WIRE_MODE_8                            0x0800
#define VAR_1WIRE_MODE_MASK                         0x00FF
#define VAR_1WIRE_MODE_ANTSEL0_MASK                 0x0F
#define VAR_1WIRE_MODE_ANTSEL1_MASK                 0xF0
#define VAR_1WIRE_MODE                              0x1000

#define VAR_WFTX_EXTTX_OK                                    0x0010
#define VAR_WFRX_EXTRX_OK                                    0x0020
#define VAR_WFRX_EXTTX_OK                                    0x0040
#define VAR_WFTX_EXTRX_OK                                    0x0080

#define VAR_EXTTX_BTTX_OK                                     0x0100
#define VAR_EXTRX_BTRX_OK                                     0x0200
#define VAR_EXTRX_BTTX_OK                                     0x0400
#define VAR_EXTTX_BTRX_OK                                     0x0800


//For ANT_SEL
#define VAR_WMT_CONFIG_ANT_SEL                      0x00080000

//For PTA timing
#define VAR_1WIRE_MODE_T0                           0x0100
#define VAR_1WIRE_MODE_T6                           0x0200
#define VAR_1WIRE_MODE_T7                           0x0400
#define VAR_1WIRE_MODE_T8                           0x0800
#define VAR_1WIRE_MODE_BT_DELAY                     0x2000

//For PTA WIFI
#define VAR_WMT_CONFIG_ARB_MODE                     0x80000000
#define VAR_BTTX_WFTX_OK                            0x0001
#define VAR_BTRX_WFRX_OK                            0x0002
#define VAR_BTRX_WFTX_OK                            0x0004
#define VAR_BTTX_WFRX_OK                            0x0008
#define VAR_BTTX_DIS_WIFIRXPE                       0x1000
#define VAR_BTRX_DIS_WIFIRXPE                       0x2000

//For DPD cal.
#define VAR_DPD_CAL_REQ                             0x01
#define VAR_DPD_CAL_REL                             0x00
#define COEX_PORT2_PTA_CTL_VAL_GRANT                0x01
#define COEX_PORT2_PTA_CTL_VAL_REJECT               0x00

//For SCO PTA periodicity
#define VAR_WMT_CONFIG_SCO_PTA_PERIODICITY          0x40000000
#define VAR_SCO_PTA_PERIODICITY_MASK                0xFF0000
#define VAR_WIFI_PTA_ACTIVITY_MASK                  0xFF00
#define VAR_BT_PTA_ACTIVITY_MASK                    0x00FF

#define VAR_WMT_CONFIG_COEX_SETTING                 0x00010000      /* WMT send coexistence setting to BT and WIFI          */

// ---------------------------------------------------------------------------
// VAR_WMT_CONFIG_COEX_SETTING
// ---------------------------------------------------------------------------
#define WMT_COEX_CONFIG_DUAL_ANT_MODE               0x00000001
#define WMT_COEX_CONFIG_ENABLE_PTA                  0x00000002
#define WMT_COEX_CONFIG_ENABLE_RW                   0x00000004

#define WMT_COEX_CONFIG_BT_ON                       0x00000040
#define WMT_COEX_CONFIG_WIFI_ON                     0x00000080

#define VAR_WMT_REPORT_BT_PROFILE                   0x10000000      /* Report BT profile                                    */

// ---------------------------------------------------------------------------
// VAR_WMT_REPORT_BT_PROFILE
// ---------------------------------------------------------------------------
#define VAR_BT_PROF_MASK                            0x000000FF

#define VAR_BT_PROF_NONE                            0x00000000
#define VAR_BT_PROF_SCO                             0x00000001
#define VAR_BT_PROF_A2DP                            0x00000002
#define VAR_BT_PROF_LINK_CONNECTED                  0x00000004
#define VAR_BT_PROF_HID                             0x00000008
#define VAR_BT_PROF_PAGE                            0x00000010
#define VAR_BT_PROF_INQUIRY                         0x00000020
#define VAR_BT_PROF_ESCO                            0x00000040
#define VAR_BT_PROF_MULTI_HID                       0x00000080


//====================================================================================
#define TX_MODE_CCK                  0x00
#define PHY_RATE_1M                            0x0
#define RATE_CCK_1M_LONG              (TX_MODE_CCK | PHY_RATE_1M)

// ---------------------------------------------------------------------------
// Protection
// ---------------------------------------------------------------------------
// Modes
#define DEFAULT_CTS_MODE                            FALSE
#define DEFAULT_FPS_MODE                            TRUE

// Protection frame: QNULL retry limit
#define DEFAULT_RW_QNULL_RETRY_LIMIT                6               /* QNULL retry limit */

// Normal AIFS in 1st SW
#define DEFAULT_RW_AIFS_SILENCE_WINDOW              5

// Protection frame: DCF settings
#define DEFAULT_RW_CWMAX                            1
#define DEFAULT_RW_CWMIN                            1
#define DEFAULT_RW_AIFS                             1

// Protection frame: SW
#define DEFAULT_RW_QNULL_SILENCE_WINDOW             0x1388          /*  5000us in  1us unit                                 */
#define DEFAULT_RW_CTS2SELF_SILENCE_WINDOW_CRITICAL 0x0280          /*   640us in  1us unit                                 */
#define DEFAULT_RW_CTS2SELF_SILENCE_WINDOW_SUPERFRM 0x07d0          /*  2000us in  1us unit                                 */
#define DEFAULT_RW_AC_SILENCE_WINDOW                0x0060          /*    96us in  1us unit                                 */
#define DEFAULT_RW_CTS_REISSUE_EARLY_TIME           0x004F          /*  5056us in 64us unit                                 */
#define DEFAULT_RW_RX_LONG_PACKET_THRESHOLD         0x002F          /*  1504us in 32us unit   (NEED CONFIRM)                */
#define DEFAULT_RW_MAX_CR_VALUE                     0x7FFF          /* 32767us                                              */

// Priority
#define WIFI_PRIORITY_AC_DEFAULT                    14
#define WIFI_PRIORITY_BCN_DEFAULT                   14
#define WIFI_PRIORITY_BMC_DEFAULT                   14
#define WIFI_PRIORITY_PROTECT_DEFAULT               14
#define WIFI_PRIORITY_RX_SP_DEFAULT                 14
#define WIFI_PRIORITY_TX_RSP_DEFAULT                10
#define WIFI_PRIORITY_RX_CCA_DEFAULT                10
#define WIFI_PRIORITY_RX_CCA_LONG_DEFAULT           6
#define WIFI_PRIORITY_TX_RX_HIGH                    4


// ---------------------------------------------------------------------------
// QoS control
// ---------------------------------------------------------------------------
// QOS Control Field and sequence number
#define DEFAULT_RW_QOS_CTRL                         0x0
#define DEFAULT_RW_INIT_SEQ_0                       0x0
#define DEFAULT_RW_INIT_SEQ_1                       0x20
#define DEFAULT_RW_INIT_SEQ_2                       0x40
#define DEFAULT_RW_INIT_SEQ_3                       0x60
#define DEFAULT_RW_RANGE_SEQ_0                      0x04
#define DEFAULT_RW_RANGE_SEQ_1                      0x04
#define DEFAULT_RW_RANGE_SEQ_2                      0x04
#define DEFAULT_RW_RANGE_SEQ_3                      0x04

// ---------------------------------------------------------------------------
// PTA/WMT
// ---------------------------------------------------------------------------
// Specify the timeout for waiting WiFi_grant after asserting wifi_tx_req/wifi_rx_req
#define PTA_TIME_OUT_WINDOW                         0x12

/* Protection rate */

#define RXP_HT_VHT_RATE_THD_MCS0                    0
#define RXP_HT_VHT_RATE_THD_MCS1                    1
#define RXP_HT_VHT_RATE_THD_MCS2                    2
#define RXP_HT_VHT_RATE_THD_MCS3                    3
#define RXP_HT_VHT_RATE_THD_MCS4                    4
#define RXP_HT_VHT_RATE_THD_MCS5                    5
#define RXP_HT_VHT_RATE_THD_MCS6                    6
#define RXP_HT_VHT_RATE_THD_MCS7                    7
#define RXP_VHT_RATE_THD_MCS8                       8
#define RXP_VHT_RATE_THD_MCS9                       9
#define RXP_HT_RATE_THD_MCS32                       32
#define RXP_OFDM_RATE_THD_6M                        8
#define RXP_OFDM_RATE_THD_9M                        9
#define RXP_OFDM_RATE_THD_12M                       10
#define RXP_OFDM_RATE_THD_18M                       11
#define RXP_OFDM_RATE_THD_24M                       12
#define RXP_OFDM_RATE_THD_36M                       13
#define RXP_OFDM_RATE_THD_48M                       14
#define RXP_OFDM_RATE_THD_54M                       15
#define RXP_CCK_RATE_THD_1M                         0
#define RXP_CCK_RATE_THD_2M                         1
#define RXP_CCK_RATE_THD_5M                         2
#define RXP_CCK_RATE_THD_11M                        3


/* Protection threshold */
#define DURATION_THD_DEFAULT                        32     /* In unit of 32 us */
#define DURATION_THD_MAX                            0x7FF  /* In unit of 32 us */
#define RATE_THD_DATA_VHT_DEFAULT                   RXP_HT_VHT_RATE_THD_MCS6
#define RATE_THD_DATA_HT_DEFAULT                    RXP_HT_VHT_RATE_THD_MCS6
#define RATE_THD_DATA_OFDM_DEFAULT                  RXP_OFDM_RATE_THD_12M
#define RATE_THD_DATA_CCK_DEFAULT                   RXP_CCK_RATE_THD_11M
#define RATE_THD_RTS_VHT_DEFAULT                    RXP_HT_VHT_RATE_THD_MCS6
#define RATE_THD_RTS_HT_DEFAULT                     RXP_HT_VHT_RATE_THD_MCS6
#define RATE_THD_RTS_OFDM_DEFAULT                   RXP_OFDM_RATE_THD_18M
#define RATE_THD_RTS_CCK_DEFAULT                    RXP_CCK_RATE_THD_5M


#define RXP_DUR_TRIG_CNT_DEFAUT                     0
#define RXP_DUR_RET_CNT_DEFAUT                      255
#define RXP_RATE_TRIG_CNT_DEFAUT                    0
#define RXP_RATE_RET_CNT_DEFAUT                     255
#define RXP_DUR_TRIG_CNT                            20
#define RXP_DUR_RET_CNT                             10
#define RXP_RATE_TRIG_CNT                           20
#define RXP_RATE_RET_CNT                            10



#define RXP_MODE_NORMAL                             0
#define RXP_OBSERVATION_TIMEOUT                     20
#define RXP_PF_SWITCH_ON_THD                        30
#define RXP_PF_SWITCH_OFF_THD                       40
#define RXP_PF_SWITCH_ON_THD_A2DP                   0
#define RXP_PF_SWITCH_OFF_THD_A2DP                  0
#define RXP_PRI_TAG_WF_PROT_DEFAULT                 0
#define RXP_PROT_RAISE_CNT_RINGBUF_SIZE             10
#define RXP_PROT_RINGBUF_LONG_INTERVAL_OFFSET       1
#define RXP_PROT_RINGBUF_SHORT_INTERVAL_OFFSET      6
#define RXP_PROT_RINGBUF_CHG_STATE_RESET_SIZE       5

// Priority macros
#define COEX_BCM_SET_ALL_AC(priority)               do{ halPtaSetAc0TxPriTag(priority);   \
                                                        halPtaSetAc1TxPriTag(priority);   \
                                                        halPtaSetAc2TxPriTag(priority);   \
                                                        halPtaSetAc3TxPriTag(priority);   \
                                                        halPtaSetAc4TxPriTag(priority);   \
                                                        halPtaSetAc5TxPriTag(priority);   \
                                                        halPtaSetAc6TxPriTag(priority);   \
                                                        halPtaSetAc10TxPriTag(priority);  \
                                                        halPtaSetAc11TxPriTag(priority);  \
                                                        halPtaSetAc12TxPriTag(priority);  \
                                                        halPtaSetAc13TxPriTag(priority);  \
                                                        halPtaSetAc14TxPriTag(priority);  \
                                                        halPtaSetAc0RxPriTag(priority);   \
                                                        halPtaSetAc1RxPriTag(priority);   \
                                                        halPtaSetAc2RxPriTag(priority);   \
                                                        halPtaSetAc3RxPriTag(priority);   \
                                                        halPtaSetAc4RxPriTag(priority);   \
                                                        halPtaSetAc5RxPriTag(priority);   \
                                                        halPtaSetAc6RxPriTag(priority);   \
                                                        halPtaSetAc10RxPriTag(priority);  \
                                                        halPtaSetAc11RxPriTag(priority);  \
                                                        halPtaSetAc12RxPriTag(priority);  \
                                                        halPtaSetAc13RxPriTag(priority);  \
                                                        halPtaSetAc14RxPriTag(priority);  \
                                                    }while(0)


// ---------------------------------------------------------------------------
// BT
// ---------------------------------------------------------------------------
// BT SLOT TIME
#define DEFAULT_01_BT_SLOT                          0x14            /*   640us in 32us unit                                 */
#define DEFAULT_02_BT_SLOT                          0x28            /*  1280us in 32us unit                                 */
#define DEFAULT_03_BT_SLOT                          0x3B            /*  1880us in 32us unit                                 */
#define DEFAULT_04_BT_SLOT                          0x4F            /*  2528us in 32us unit                                 */
#define DEFAULT_08_BT_SLOT                          0x9D            /*  5024us in 32us unit                                 */
#define DEFAULT_32_BT_SLOT                          0x1D4           /* 14976us in 32us unit (Not exactly 32 slots)          */

// Duration to protect each BT's profile
#define DEFAULT_HID_DURATION                        DEFAULT_02_BT_SLOT
#define DEFAULT_SCO_DURATION                        DEFAULT_02_BT_SLOT
#define DEFAULT_SLAVE_BOUNDARY_DURATION             DEFAULT_03_BT_SLOT
#define DEFAULT_ESCO_DURATION                       DEFAULT_04_BT_SLOT
#define DEFAULT_MUL_CIRITICAL_DURATION              DEFAULT_08_BT_SLOT
#define DEFAULT_ACL_DURATION                        DEFAULT_32_BT_SLOT
#define DEFAULT_MAX_DURATION                        0x3E8

#define COEX_BCM_IS_BT_SCO(x)                       ((x) & VAR_BT_PROF_SCO)
#define COEX_BCM_IS_BT_ESCO(x)                      ((x) & VAR_BT_PROF_ESCO)                                                    
#define COEX_BCM_IS_CRITICAL(x)                     (COEX_BCM_IS_BT_SCO(x)||   \
                                                     COEX_BCM_IS_BT_ESCO(x))
#endif /* __HAL_BWCS_REGS_H__ */
