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


#ifndef __BT_DEVICE_MANAGER_LE_H__
#define __BT_DEVICE_MANAGER_LE_H__

/**
 * @addtogroup Bluetooth_Services_Group Bluetooth Services
 * @{
 * @addtogroup BluetoothServices_DM BLE Device Manager
 * @{
 * This section provides API to manage the bonded peers and API to get and set security and connection between the bonded peers in LE. The bonded peer management includes the bookkeeping of contextual information, such as the security keys and any application specific information.
 *
 * Terms and Acronyms
 * ======
 * |Terms                         |Details                                                                  |
 * |------------------------------|-------------------------------------------------------------------------|
 * |\b GAP                        | Generic Access Profile. This profile defines the generic procedures related to discovery of Bluetooth enabled devices and link management aspects of connecting to the Bluetooth enabled devices. It also defines procedures related to the use of different security levels. |
 *
 * @section device_manager_le_api_usage How to use this module
 * - Step 1. Initialize the LE Device Manager.
 *  - Sample Code:
 *     @code
 *                bt_device_manager_le_init();
 *     @endcode
 * - Step 2. Use LE Device Manager APIs to set security information, such as local key, pairing configuration.
 *  - Sample code:
 *     @code 
 *                bt_gap_le_smp_pairing_config_t pairing_config = {// MITM, Bond, Out-of-Band
 *                                    .maximum_encryption_key_size = 16,
 *                                    .io_capability = BT_GAP_LE_SMP_NO_INPUT_NO_OUTPUT,
 *                                    .auth_req = BT_GAP_LE_SMP_AUTH_REQ_BONDING,
 *                                    .oob_data_flag = BT_GAP_LE_SMP_OOB_DATA_NOT_PRESENTED,
 *                                    .initiator_key_distribution = BT_GAP_LE_SMP_KEY_DISTRIBUTE_ENCKEY | BT_GAP_LE_SMP_KEY_DISTRIBUTE_IDKEY | BT_GAP_LE_SMP_KEY_DISTRIBUTE_SIGN,
 *                                    .responder_key_distribution = BT_GAP_LE_SMP_KEY_DISTRIBUTE_ENCKEY | BT_GAP_LE_SMP_KEY_DISTRIBUTE_IDKEY | BT_GAP_LE_SMP_KEY_DISTRIBUTE_SIGN,
 *                                    };
 *                bt_gap_le_local_key_t local_key = {
 *                                     .encryption_info.ltk = { 0xc0, 0xc1, 0xc2, 0xc3, 0xc4, 0xc5, 0xc8, 0xc7, 0xc8, 0xc9, 0xca, 0xcb, 0xcc, 0xcd, 0xce, 0xcf },
 *                                     .master_id.ediv = 0x1005,
 *                                     .master_id.rand = { 0xc0, 0xc1, 0xc2, 0xc3, 0xc4, 0xc5, 0xc6, 0xc7 },
 *                                     .identity_info.irk = { 0xc0, 0xc1, 0xc2, 0xc3, 0xc4, 0xc5, 0xc6, 0xc7, 0xc8, 0xc9, 0xca, 0xcb, 0xcc, 0xcd, 0xce, 0xcf },
 *                                     .signing_info.csrk = { 0xa0, 0xa1, 0xa2, 0xa3, 0xa4, 0xa5, 0xa6, 0xa7, 0xa8, 0xa9, 0xaa, 0xab, 0xac, 0xad, 0xae, 0xaf }
 *                                     };
 *                
 *                bool sc_only = false;
 *                bt_device_manager_le_gap_set_pairing_configuration(&pairing_config); 
 *                bt_device_manager_le_gap_set_local_configuration(&local_key, sc_only);
 *     @endcode
 */

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include "bt_gap_le.h"
#include "bt_type.h"


BT_EXTERN_C_BEGIN

/**
 * @defgroup Device_Manager_LE_define Define
 * @{
 * Define Bluetooth device manager LE service data types and values.
 */

/**
 * @brief The Max number of callbacks that Bluetooth device manager LE service support.
 */
#define BT_DEVICE_MANAGER_LE_CB_MAX_NUM (1)

/**
 * @brief Bluetooth device manager LE service bond change event.
 */

#define BT_DEVICE_MANAGER_LE_BONDED_ADD            0x0          /**< This event is generated after adding a new bonded device. */
#define BT_DEVICE_MANAGER_LE_BONDED_REMOVE         0x1          /**< This event is generated after removing a bonded device through #bt_device_manager_le_remove_bonded_device() or remove the oldest boned info because of list full. */
#define BT_DEVICE_MANAGER_LE_BONDED_CLEAR          0x2          /**<This event is generated after removing all bonded devices through #bt_device_manager_le_clear_all_bonded_info(). */
typedef uint8_t bt_device_manager_le_bonded_event_t;            /**< The event type of bonded information has changed. */

/**
 * @}
 */

/**
 * @defgroup Device_Manager_LE_struct Struct
 * @{
 * This section defines structures for the LE Device Manager.
 */

/**
 *  @brief Bongding infomation structure, internal use.
 */
BT_PACKED(
typedef struct {
    bt_addr_t bt_addr;             /**< The address of a remote device to be bonded. */
    bt_gap_le_bonding_info_t info; /**< The connection bonding information. */
}) bt_device_manager_le_bonded_info_t;

/**
 *  @brief Connection parameter structure.
 */
typedef struct {
    uint16_t          conn_interval;           /**< Connection Interval in 1.25 ms units, value range: 0x0006 to 0x0C80.*/
    uint16_t          slave_latency;            /**< Slave Latency in number of connection events, value range: 0x0000 to 0x03E8.*/
    uint16_t          supervision_timeout;     /**< Connection Supervision Timeout in 10 ms units, value range: 0x000A to 0x0C80.*/
} bt_device_manager_le_connection_param_t;


/**
 *  @brief Bluetooth device manager LE service bond change event handler type. 
 */
typedef void(* bt_device_manager_le_bonded_event_callback)(bt_device_manager_le_bonded_event_t event, bt_addr_t *address);

/**
 * @}
 */
 

/**
 * @brief   This function initializes the LE Device Manager service. It is recommended to call this API once during the bootup.
 * @return    None.           
 */
void bt_device_manager_le_init(void);

/**
 * @brief   This function removes the bonded device information specified by the peer_addr from this device.
 * @param[in] peer_addr    is the address of the bonded device.
 * @return    None.
 */
void bt_device_manager_le_remove_bonded_device(bt_addr_t *peer_addr);

/**
 * @brief   This function clears all bonded information stored in this device.
 * @return    None.          
 */
void bt_device_manager_le_clear_all_bonded_info(void);

/**
 * @brief   This function gets the connection parameter of the current link specified by the connection handle.
 * @param[in] connection_handle      is the connection handle.
 * @return    connection_param_t     is a pointer to the current connection parameter.         
 */
bt_device_manager_le_connection_param_t *bt_device_manager_le_get_current_connection_param(bt_handle_t connection_handle);

/**
 * @brief   This function sets the local configuration of this device.
 * @param[in] local_key              is a pointer to the @ref bt_gap_le_local_config_req_ind_t structure.
 * @param[in] sc_only_mode           is a flag to enable or disable the secure connection pairing mode.
 * @return                           BT_STATUS_SUCCESS, the local configuration is successfully set.
 *                                   BT_STATUS_FAIL, the local configuration failed to set. Please call @ref bt_device_manager_le_init() first.
 */
bt_status_t bt_device_manager_le_gap_set_local_configuration(bt_gap_le_local_key_t *local_key, bool sc_only_mode);

/**
 * @brief   This function sets the preferred pairing configuration of this device.
 * @param[in] pairing_config         is a pointer to the @ref bt_gap_le_smp_pairing_config_t structure.
 * @return                           BT_STATUS_SUCCESS, the pairing configuration is successfully set.
 *                                   BT_STATUS_FAIL, the pairing configuration failed to set. Please call @ref bt_device_manager_le_init() first.
 */
bt_status_t bt_device_manager_le_gap_set_pairing_configuration(bt_gap_le_smp_pairing_config_t *pairing_config);

/**
 * @brief         This function gets the list of bonded LE devices.
 * @param[out]    list               is the list of bonded LE devices.
 * @param[in,out] count              is the input and output parameter. As an input parameter, it is the length of the list and the maximum number of
 *                                   bonded LE devices that the list can hold. As an output parameter, it is the actual number of the bonded LE devices
 *                                   stored in the list upon the return of this function, which cannot exceed the length of the list.
 * @return        None.
 */
void bt_device_manager_le_get_bonded_list(bt_bd_addr_t *list, uint8_t *count);

/**
 * @brief         This function gets the bonded status of the LE device specified by the address.
 * @param[in]     address            is the address of the LE device to check.
 * @return                           True, bonded.
 *                                   False, unbonded.
 */
bool bt_device_manager_le_is_bonded(bt_addr_t *address);

/**
 * @brief      This function gets the count of the bonded LE devices.
 * @return     The count of the bonded LE devices.
 */
uint8_t bt_device_manager_le_get_bonded_number(void);

/**
 * @brief     This function gets the public address of the device.
 * @return    The public address of the device.
 */
bt_bd_addr_ptr_t bt_device_manager_le_get_public_address(void);

/**
 * @brief     This function gets the bonded information of the device.
 * @param[in] remote_addr   is the address of the remote LE device.
 * @return    The LE bonding info of the remote device.
 */
bt_gap_le_bonding_info_t *bt_device_manager_le_get_bonding_info_by_addr(bt_bd_addr_t *remote_addr);


/**
 * @brief     This function gets the full bonded information of the device.
 * @param[in] remote_addr   is the address of the remote LE device.
 * @return    The full LE bonding info of the remote device.
 */
bt_device_manager_le_bonded_info_t *bt_device_manager_le_get_bonding_info_by_addr_ext(bt_bd_addr_t *remote_addr);

/**
 * @brief         This function gets all the bonding infos of bonded LE devices.
 * @param[in]    infos               is the buffer to save the infos of bonded LE devices, need provide by application.
 * @param[in,out] count              is the input and output parameter. As an input parameter, it is the length of the infos and the maximum number of
 *                                   bonded LE devices that the infos can hold. As an output parameter, it is the actual number of the bonded LE devices
 *                                   stored in the infos upon the return of this function, which cannot exceed the length of the infos.
 * @return                           BT_STATUS_SUCCESS, the infos is successfully get.
 *                                   BT_STATUS_FAIL, the infos is failed to get. 
 */
bt_status_t bt_device_manager_le_get_all_bonding_infos(bt_device_manager_le_bonded_info_t *infos, uint8_t *count);

/**
 * @brief         This function set the bonding infos of the bonded LE device.
 * @param[in] address               is the address of the bonded LE device.
 * @param[in] info              is the bonding info of the bonded LE device.
 * @return                           BT_STATUS_SUCCESS, the infos is successfully set.
 *                                   BT_STATUS_FAIL, the infos is failed to set. 
 */

bt_status_t bt_device_manager_le_set_bonding_info_by_addr(bt_addr_t *address, bt_gap_le_bonding_info_t *info);

/**
 * @brief   Function for application to register a event handler. 
 * @param[in] callback      is a pointer to the callback function to register. This callback function is invoked by the LE Device Manager module.
 * @return              #BT_STATUS_SUCCESS, if the operation is successful.
 *                          #BT_STATUS_FAIL, if the callback type is incorrect or already registered or if the maximum number of callbacks, 
 *                          as #BT_DEVICE_MANAGER_LE_CB_MAX_NUM  has been reached.
 */
bt_status_t bt_device_manager_le_register_callback(bt_device_manager_le_bonded_event_callback callback);


/**
 * @brief     Function for unregister the callback from the LE Device Manager module.
 * @param[in] callback      is a pointer to the callback function to unregister. 
 * @return                  #BT_STATUS_SUCCESS, if the operation is successful.
 *                          #BT_STATUS_FAIL, if the callback type is incorrect or unregistered, or the callback function is unregistered.
 */
bt_status_t bt_device_manager_le_deregister_callback(bt_device_manager_le_bonded_event_callback callback);


BT_EXTERN_C_END
    
/**
 * @}
 * @}
 */

#endif /* __BT_DEVICE_MANAGER_LE_H__ */
