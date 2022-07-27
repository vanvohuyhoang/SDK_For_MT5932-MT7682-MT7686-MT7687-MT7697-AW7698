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


#ifndef __PTA_API_H__
#define __PTA_API_H__

/**
 * \file pta_api.h.
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


//#include <stdint.h>
#include <stdbool.h>


#ifdef __cplusplus
extern "C" {
#endif


/*****************************************************************************
 * enums
 *****************************************************************************/


#define PTA_PERIOD_COUNT            (8)


/**
 * Supported PTA modes.
 */
typedef enum
{
    PTA_MODE_TDD_1_WIRE,
    PTA_MODE_TDD_1_WIRE_EXTENDED,
    PTA_MODE_TDD_3_WIRE,
} pta_mode_t;


typedef enum
{
    PTA_PRIORITY_TAG_BT_RX_HIGH,            ///< 0~15
    PTA_PRIORITY_TAG_BT_RX_LOW,             ///< 0~15
    PTA_PRIORITY_TAG_BT_TX_HIGH,            ///< 0~15
    PTA_PRIORITY_TAG_BT_TX_LOW,             ///< 0~15
    PTA_PRIORITY_TAG_BT_SCO,                ///< 0~15
    PTA_PRIORITY_TAG_WIFI_MAN,              ///< 0~15
    PTA_PRIORITY_TAG_WIFI_TX_AC0,           ///< 0~15
    PTA_PRIORITY_TAG_WIFI_TX_AC1,           ///< 0~15
    PTA_PRIORITY_TAG_WIFI_TX_AC2,           ///< 0~15
    PTA_PRIORITY_TAG_WIFI_TX_AC3,           ///< 0~15
    PTA_PRIORITY_TAG_WIFI_TX_AC4,           ///< 0~15
    PTA_PRIORITY_TAG_WIFI_TX_AC5,           ///< 0~15
    PTA_PRIORITY_TAG_WIFI_TX_AC6,           ///< 0~15
    PTA_PRIORITY_TAG_WIFI_TX_BMC,           ///< 0~15
    PTA_PRIORITY_TAG_WIFI_RX_ACK_AC0,       ///< 0~15
    PTA_PRIORITY_TAG_WIFI_RX_ACK_AC1,       ///< 0~15
    PTA_PRIORITY_TAG_WIFI_RX_ACK_AC2,       ///< 0~15
    PTA_PRIORITY_TAG_WIFI_RX_ACK_AC3,       ///< 0~15
    PTA_PRIORITY_TAG_WIFI_RX_ACK_AC4,       ///< 0~15
    PTA_PRIORITY_TAG_WIFI_RX_ACK_AC5,       ///< 0~15
    PTA_PRIORITY_TAG_WIFI_RX_ACK_AC6,       ///< 0~15
    PTA_PRIORITY_TAG_WIFI_TX_BFEE,          ///< 0~15
    PTA_PRIORITY_TAG_WIFI_TX_RESP,          ///< 0~15
    PTA_PRIORITY_TAG_WIFI_SVC_PRD,          ///< 0~15
    PTA_PRIORITY_TAG_WIFI_RX_NSW,           ///< 0~15
    PTA_PRIORITY_TAG_WIFI_RX_NSW_LONG,      ///< 0~15
    PTA_PRIORITY_TAG_WIFI_TX_AFTER_LONG_RX, ///< 0~15
} pta_priority_tag_t;


typedef enum
{
    PTA_PRIORITY_SCO_BT_WIFI,
    PTA_PRIORITY_SCO_WIFI_BT,
    PTA_PRIORITY_ADVANCED
} pta_priority_type_t;


typedef enum
{
    PTA_ANTENNA_SELECT_SINGLE,
    PTA_ANTENNA_SELECT_SINGLE_3_WIRE,
    PTA_ANTENNA_SELECT_DUAL
} pta_antenna_select_t;

typedef enum
{
    PTA_PERIOD_MODE_START_END,
    PTA_PERIOD_MODE_END
} pta_period_mode_t;


/****************************************************************************
 *
 * Structures
 *
 ****************************************************************************/


typedef struct _pta_priority_tag_config_t
{
    uint8_t     tag;        ///< See pta_priority_tag_t.
    uint8_t     priority;   ///< See pta_priority_tag_t.
} pta_priority_tag_config_t;


typedef struct _pta_1_wire_param_t
{
    uint8_t         t6;         ///< Guard time in us. The time for ramping
                                ///< up/down grant or no grant.

    uint8_t         t8;         ///< Turnaround time. If Wi-Fi is forced to
                                ///< give up current transmission, this time
                                ///< is used to guarantee Wi-Fi hardware is
                                ///< fully stopped and there will be no energy
                                ///< emission from Wi-Fi antenna that could
                                ///< affect BlueTooth.

    uint8_t         rsvd0[2];   ///< Reserved.
} pta_1_wire_param_t;


/**
 * The configuration of PTA hardware.
 */
typedef struct _pta_config_t
{
    uint32_t        mode;       ///< The mode of PTA. See pta_mode_t for
                                ///< more information. Mode is declared as
                                ///< uint32_t to ensure storage space is fixed
                                ///< since enum may change by compiler
                                ///< optimization setting.
    union {
        pta_1_wire_param_t     one_wire;   ///< the configuration for 1-wire.
    };

} pta_config_t;


typedef struct _pta_priority_config_t
{
    /*
     * The priority of packets PTA clients.
     *
     * There are two defaults to be chosen as references.
     *
     * 1. PTA_PRIORITY_SCO_BT_WIFI (0)
     * 2. PTA_PRIORITY_SCO_WIFI_BT (1)
     *
     * See pta_priority_type_t for more details about them.
     * A third option, PTA_PRIORITY_ADVANCED, allows fine-grained
     * tuning of priorities among different sort of packets. When
     * PTA_PRIORITY_ADVANCED is used, set the tags array and provide the
     * number of elements in tags_count.
     */
    uint32_t                    priority;

    /*
     * specifies the actual number of elements in tags.
     */
    uint8_t                     tags_count;


    /*
     * Reserved for future extension and 32-bits alignment.
     */
    uint8_t                     rsvd0[3];


    /*
     * the array of pairs (of tag and its priority).
     */
    pta_priority_tag_config_t   tags[0];

} pta_priority_config_t;


typedef struct _pta_period_config_t
{
    /*
     * The mode of this config. See pta_period_mode_t for supported values.
     */
    uint8_t                     mode;


    /*
     * Reserved for future extension and 32-bits alignment.
     */
    uint8_t                     rsvd0[3];


    uint8_t                     periods[PTA_PERIOD_COUNT];
} pta_period_config_t;


/****************************************************************************
 *
 * API functions.
 *
 ****************************************************************************/


/**
 * Start PTA hardware to the specified mode.
 *
 * @param cfg   The configuration of PTA.
 *              See wifi_pta_config_t, pta_mode_t, and
 *              pta_1_wire_param_t for more details.
 *
 * @retval true     if the configuration is valid and accepted.
 * @retval false    if the configuration is not valid and not accepted.
 */
bool pta_init(const pta_config_t *cfg);

/**
 * Read the priority of the specified types of TRX request.
 */
uint8_t pta_priority_get(pta_priority_tag_t tag);


/**
 * Config the priorities of different types of TRX request.
 */
bool pta_priority_set(const pta_priority_config_t *cfg);


/**
 * Read the periods of the specified mode.
 *
 * @param  cfg      The buffer to read the setting to. However, the mode
 *                  field should be specified tell this API to read
 *                  corresponding settings.
 *
 * @retval true     if succeeded
 * @retval false    if invalid pointer or mode is not supported.
 */
bool pta_period_get(pta_period_config_t *cfg);


/**
 * Config the periods 'start-end mode' or 'end mode'.
 *
 * @param  cfg      the setting to be applied.
 *
 * @retval true     if succeeded
 * @retval false    if invalid pointer or mode is not supported.
 */
bool pta_period_set(const pta_period_config_t *cfg);


/**
 * Configure the antenna select mode.
 *
 * @param  select   The antenna select mode. See pta_antenna_select_t for more
 *                  details.
 *
 * @retval true     if succeeded.
 * @retval false    if mode is not supported.
 */
bool pta_antenna_select_set(const pta_antenna_select_t select);


/**
 * Stop PTA hardware.
 */
void pta_deinit(void);


#ifdef __cplusplus
}
#endif

#endif /* __PTA_API_H__ */
