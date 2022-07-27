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


#ifndef __HAL_PTA_H__
#define __HAL_PTA_H__

/**
 * \file hal_pta.h.
 *
 * This file defined the API of PTA (packet transport arbitration) module.
 * The PTA module is a hardware that arbitrate between overlapping radio
 * frequency hardwares to decide which to transmit/receive.
 *
 * Before PTA hardware can be used, the corresponding pins of the chip has
 * to be configured to the correct mode.
 */


/*****************************************************************************
 * module header file include
 *****************************************************************************/

#include "rt_config.h"

//#include <stdint.h>
#include <stdbool.h>


#ifdef __cplusplus
extern "C" {
#endif


/****************************************************************************
 *
 * Constants.
 *
 ****************************************************************************/

/****************************************************************************
 *
 * Enums
 *
 ****************************************************************************/


typedef enum
{
    HAL_PTA_PRIORITY_TAG_BT_RX_HIGH_DEFAULT             = 0x7,
    HAL_PTA_PRIORITY_TAG_BT_RX_LOW_DEFAULT              = 0xD,
    HAL_PTA_PRIORITY_TAG_BT_TX_HIGH_DEFAULT             = 0x4,
    HAL_PTA_PRIORITY_TAG_BT_TX_LOW_DEFAULT              = 0xA,
    HAL_PTA_PRIORITY_TAG_BT_SCO_DEFAULT                 = 0x1,
    HAL_PTA_PRIORITY_TAG_WIFI_MAN_DEFAULT               = 0x8,
    HAL_PTA_PRIORITY_TAG_WIFI_MAN_ESCALATED             = 0x2,
    HAL_PTA_PRIORITY_TAG_WIFI_TX_RX_HIGH                = 0x4,
    HAL_PTA_PRIORITY_TAG_WIFI_AC_DEFAULT                = 0xE,
    HAL_PTA_PRIORITY_TAG_WIFI_BMC_DEFAULT               = 0xE,
    HAL_PTA_PRIORITY_TAG_WIFI_RX_SP_DEFAULT             = 0xE,
    HAL_PTA_PRIORITY_TAG_WIFI_TX_RSP_DEFAULT            = 0xA,
    HAL_PTA_PRIORITY_TAG_WIFI_RX_CCA_DEFAULT            = 0xA,
    HAL_PTA_PRIORITY_TAG_WIFI_RX_CCA_LONG_DEFAULT       = 0x6,
    HAL_PTA_PRIORITY_TAG_WIFI_TX_AFTER_LONG_RX_DEFAULT  = 0x0,

} hal_pta_priority_value_t;


typedef enum
{
    HAL_PTA_PRIORITY_TAG_BT_RX_HIGH,            ///< 0~15
    HAL_PTA_PRIORITY_TAG_BT_RX_LOW,             ///< 0~15
    HAL_PTA_PRIORITY_TAG_BT_TX_HIGH,            ///< 0~15
    HAL_PTA_PRIORITY_TAG_BT_TX_LOW,             ///< 0~15
    HAL_PTA_PRIORITY_TAG_BT_SCO,                ///< 0~15
    HAL_PTA_PRIORITY_TAG_WIFI_MAN,              ///< 0~15
    HAL_PTA_PRIORITY_TAG_WIFI_TX_AC0,           ///< 0~15
    HAL_PTA_PRIORITY_TAG_WIFI_TX_AC1,           ///< 0~15
    HAL_PTA_PRIORITY_TAG_WIFI_TX_AC2,           ///< 0~15
    HAL_PTA_PRIORITY_TAG_WIFI_TX_AC3,           ///< 0~15
    HAL_PTA_PRIORITY_TAG_WIFI_TX_AC4,           ///< 0~15
    HAL_PTA_PRIORITY_TAG_WIFI_TX_AC5,           ///< 0~15
    HAL_PTA_PRIORITY_TAG_WIFI_TX_AC6,           ///< 0~15
    HAL_PTA_PRIORITY_TAG_WIFI_TX_BMC,           ///< 0~15
    HAL_PTA_PRIORITY_TAG_WIFI_RX_ACK_AC0,       ///< 0~15
    HAL_PTA_PRIORITY_TAG_WIFI_RX_ACK_AC1,       ///< 0~15
    HAL_PTA_PRIORITY_TAG_WIFI_RX_ACK_AC2,       ///< 0~15
    HAL_PTA_PRIORITY_TAG_WIFI_RX_ACK_AC3,       ///< 0~15
    HAL_PTA_PRIORITY_TAG_WIFI_RX_ACK_AC4,       ///< 0~15
    HAL_PTA_PRIORITY_TAG_WIFI_RX_ACK_AC5,       ///< 0~15
    HAL_PTA_PRIORITY_TAG_WIFI_RX_ACK_AC6,       ///< 0~15
    HAL_PTA_PRIORITY_TAG_WIFI_TX_BFEE,          ///< 0~15
    HAL_PTA_PRIORITY_TAG_WIFI_TX_RESP,          ///< 0~15
    HAL_PTA_PRIORITY_TAG_WIFI_SVC_PRD,          ///< 0~15
    HAL_PTA_PRIORITY_TAG_WIFI_RX_NSW,           ///< 0~15
    HAL_PTA_PRIORITY_TAG_WIFI_RX_NSW_LONG,      ///< 0~15
    HAL_PTA_PRIORITY_TAG_WIFI_TX_AFTER_LONG_RX, ///< 0~15

} hal_pta_priority_tag_t;


typedef enum
{
    HAL_PTA_PERIOD_MODE_START_END,
    HAL_PTA_PERIOD_MODE_END
} hal_pta_period_mode_t;


typedef enum
{
    HAL_PTA_ANTENNA_SELECT_MODE_SINGLE,
    HAL_PTA_ANTENNA_SELECT_MODE_SINGLE_3_WIRE,
    HAL_PTA_ANTENNA_SELECT_MODE_DUAL
} hal_pta_antenna_select_mode_t;


/****************************************************************************
 *
 * Structures
 *
 ****************************************************************************/


typedef struct _hal_pta_status_t
{
    uint16_t    bt_tx_abt_cnt;
    uint16_t    bt_rx_abt_cnt;
    uint16_t    wf_tx_abt_cnt;
    uint16_t    wf_rx_abt_cnt;
    uint16_t    bt_tx_req_cnt;
    uint16_t    bt_tx_gnt_cnt;
    uint16_t    bt_rx_req_cnt;
    uint16_t    wf_tx_req_cnt;
    uint16_t    wf_tx_gnt_cnt;
    uint16_t    wf_rx_req_cnt;
    uint32_t    observe;
} hal_pta_status_t;


/****************************************************************************
 *
 * API functions.
 *
 ****************************************************************************/


bool hal_pta_tdd_1_wire_init    (uint8_t t6, uint8_t t8);


bool hal_pta_tdd_1_wire_ext_init(uint8_t t6, uint8_t t8);


bool hal_pta_tdd_3_wire_init    (void);


/**
 * Get the priority tag of the specified type of TRX request.
 *
 * @param tag   the type of TRX request.
 * @return      0xFF when the tag is invalid. The actual value of priority
 *              otherwise.
 */
uint8_t hal_pta_priority_get(hal_pta_priority_tag_t tag);


bool hal_pta_period_get(hal_pta_period_mode_t mode, uint8_t *periods);


bool hal_pta_priority_set(hal_pta_priority_tag_t tag, uint8_t priority);


bool hal_pta_period_set(hal_pta_period_mode_t mode, const uint8_t *periods);


/**
 * Set the antenna select mode.
 *
 * @param  mode     The mode of antenna select pins. See
 *                  hal_pta_antenna_select_mode_t for more information.
 *
 * @retval true     If the antenna select mode is applied.
 * @retval false    If the antenna select mode is not supported.
 */
bool hal_pta_antenna_select_set(hal_pta_antenna_select_mode_t mode);


void hal_pta_deinit(void);


/**
 * Retrieve the counters from PTA module.
 *
 * @param status    the counters are listed in the structure pointed by
 *                  status.
 * @param reset     reset the counters after fetched.
 *                  -1 for maintaining current setting (must be used alone).
 *                  0 to disable counters (can be ORed with 2 or/and 4).
 *                  1 to enable counters (can be ORed with 2 or/and 4).
 *                  2 to clear BT counters.
 *                  4 to clear Wi-Fi counters.
 */
void hal_pta_dump_status(hal_pta_status_t *status, int8_t reset);


/**
 * debug flag register, MCU dependent.
 */
void hal_pta_debug_flag_set(uint32_t val);


bool hal_pta_bt_ext_pin_enable(bool fgEnable);
bool hal_pta_bt_trx_disable_wifi_rx(bool fgBtTxDisWiFiRxPe, bool fgBtRxDisWiFiRxPe);
bool hal_pta_arb_mode(uint8_t type, uint8_t arb_mode);
bool hal_pta_arb_function_enable(bool fgPtaSysEnable, bool fgWfPtaEnable, bool fgBtPtaEnable);

#ifdef __cplusplus
}
#endif

#endif /* __HAL_PTA_H__ */
