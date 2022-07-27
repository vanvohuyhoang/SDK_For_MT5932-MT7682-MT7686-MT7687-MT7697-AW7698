/* Copyright Statement:
 *
 * (C) 2017  Airoha Technology Corp. All rights reserved.
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

#ifndef __BWCS_API_H__
#define __BWCS_API_H__

/**@defgroup BWCS_ENUM Enumeration
* @{
*/
/** @brief This enumeration defines PTA mode.
*/
typedef enum
{
    PTA_MODE_TDD_1_WIRE,              /**<  1 wire PTA mode. */
    PTA_MODE_TDD_1_WIRE_EXTENDED,     /**<  1 wire extend PTA mode. */
    PTA_MODE_TDD_3_WIRE,              /**<  3 wire mode. */
} pta_mode_t;

/** @brief This enumeration defines coexistence mode.
*/
typedef enum
{
    PTA_CM_MODE_TDD,                  /**<  TDD(Time Division Duplex) is scheduled coexistence mode.*/
    PTA_CM_MODE_FDD,                  /**<  FDD(Frequency Division Duplex) is unschedule coexistence mode. It doesn't support */
} pta_cm_mode_t;

/** @brief This enumeration defines antenna mode in coexistence.
*/
typedef enum
{
    PTA_ANT_MODE_SINGLE,             /**<  Single antenna mode, Wi-Fi and BT share the same antenna, this mode is only used in TDD.*/
    PTA_ANT_MODE_DUAL,               /**<  Dual antenna mode, Wi-Fi and BT have the independent antenna, this mode is both used in TDD and FDD.*/
} pta_antenna_mode_t;

/** @brief This enumeration defines BWCS events indicate to up layer.
*/
typedef enum {
    BWCS_EVENT_IOT_WIFI_OFF = 0,    /**<  Indicate Wi-Fi turns off.*/
    BWCS_EVENT_IOT_WIFI_ON,         /**<  Indicate Wi-Fi turns on.*/
    BWCS_EVENT_IOT_BT_OFF,          /**<  Indicate BT turns off.*/
    BWCS_EVENT_IOT_BT_ON,           /**<  Indicate BT turns on.*/
    BWCS_EVENT_IOT_TDD,             /**<  Indicate BWCS runs with TDD mode .*/
    BWCS_EVENT_IOT_FDD,             /**<  Indicate BWCS runs with FDD mode .*/
    BWCS_EVENT_MAX_NUMBER
} bwcs_event_t;

/**
* @}
*/

/**@defgroup BWCS_STRUCT Structure
* @{
*/
/** @brief This enumeration defines BWCS intial parameters.
*/
typedef struct
{
    pta_mode_t pta_mode;             /**<  PTA mode initial. For more details, please refer to #wifi_event_t.*/
    pta_cm_mode_t pta_cm_mode;       /**<  Coexistence mode initial. For more details, please refer to #pta_cm_mode_t.*/
    pta_antenna_mode_t antenna_mode; /**<  Antenna mode initial. For more details, please refer to #pta_antenna_mode_t.*/
} bwcs_init_t;
/**
* @}
*/


/**@defgroup BWCS_TYPEDEF Typedef
* @{
*/
/**
* @brief This defines the BWCS event handler. Call #bwcs_register_event_handler() to register a handler, then the BWCS driver generates an event and sends it to the handler.
* @param[in] event  is an optional event to register. For more details, please refer to #bwcs_event_t.
*
* @param[in] payload  is the payload for the event.
* @param[in] length is the length of a packet.
*
* @return The return value is reserved and it is ignored.
*/
typedef int32_t (* bwcs_event_handler_t)(bwcs_event_t event, uint8_t *payload, uint32_t length);

/**
* @}
*/

/**
* @brief This function inits the BWCS module.
*
* @param[in] bwcs_init_param is the BWCS intial parameters that the function will operate on. For more details, please refer to #bwcs_init_t.
*
* @return  >=0 the operation completed successfully, <0 the operation failed.
*
*@note BWCS should enabled with Wi-Fi and BLE coex case to schedule the use of antenna.
*/
int32_t bwcs_init(bwcs_init_t bwcs_init_param);

/**
* @brief This function deinits the BWCS module.
*
* @return  >=0 the operation completed successfully, <0 the operation failed.
*
*@note If there is no Wi-Fi and BLE coex case, the BWCS can be deinited.
*/
int32_t bwcs_deinit(void);

/**
* @brief This function registers the BWCS event handler. 
* When Wi-Fi is turn on, BWCS_EVENT_IOT_WIFI_ON event is triggered;\n
* When Wi-Fi is turn off, BWCS_EVENT_IOT_WIFI_OFF event is triggered;\n
* When BLE is turn on, BWCS_EVENT_IOT_BT_ON event is triggered;\n
* When BLE is turn off, BWCS_EVENT_IOT_BT_OFF event is triggered;\n
* When BWCS runs with TDD mode, BWCS_EVENT_IOT_TDD event is triggered;\n
* When BWCS runs with FDD mode, BWCS_EVENT_IOT_FDD event is triggered;\n
*
* @param[in] event is the event ID. For more details, please refer to #bwcs_event_t.
*
* @param[in] handler is the event handler. For more details, please refer to #bwcs_event_handler_t.
*
* @return  >=0 the operation completed successfully, <0 the operation failed.
*
*/
int32_t bwcs_register_event_handler(bwcs_event_t event,bwcs_event_handler_t handler);

/**
* @brief This function unregisters BWCS event handler. The function #bwcs_register_event_handler() registers an event and 
* matches it with the corresponding event handler. For the event behavior, please refer to #bwcs_register_event_handler().
*
* @param[in] event is the event ID. For more details, please refer to #bwcs_event_t for more details.
*
* @param[in] handler is the event handler. For more details, please refer to #bwcs_event_handler_t.
*
* @return  >=0 the operation completed successfully, <0 the operation failed.
*/
int32_t bwcs_unregister_event_handler(bwcs_event_t event,bwcs_event_handler_t handler);

#endif

