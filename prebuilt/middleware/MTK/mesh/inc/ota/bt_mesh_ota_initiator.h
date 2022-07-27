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

#ifndef BT_MESH_OTA_INITIATOR_HH
#define BT_MESH_OTA_INITIATOR_HH

#include <stdbool.h>
#include <stdint.h>

#include "bt_mesh_access.h"

/**
 *   @addtogroup BluetoothMesh Mesh
 *   @{
 *   @addtogroup BluetoothMeshMiddleware Middleware
 *   @{
        bt_mesh_ota_initiator.h defines the OTA initiator's APIs.
*/

/*************************************************************************
* Type define
*************************************************************************/
/*!
    @brief error code of initiator
*/
typedef enum {
    ERROR_OTA_SUCCESS,                  /**< Error code of indicating success.*/
    ERROR_OTA_WRONG_FIRMWARE_ID,        /**< Error code of inidcating wrong firmware id.*/
    ERROR_OTA_BUSY,                     /**< Error code of inidcating busy of distributor*/
    ERROR_OTA_NO_RESPONSE,              /**< Error code of inidcating no response of distributor*/
    ERROR_OTA_USER_STOP,                /**< Error code of inidcating user interuption*/
} bt_mesh_ota_initiator_error_e;

/*!
    @brief event id of initiator
*/
typedef enum {
    EVENT_DISTRIBUTION_STARTING, /**< Event id for informing status of a new distribution was starting.*/
    EVENT_DISTRIBUTION_STARTED,  /**< Event id for informing status of a new distribution was started.*/
    EVENT_DISTRIBUTION_ONGOING,  /**< Event id for informing status of the distribution was ongoing.*/
    EVENT_DISTRIBUTION_STOP,     /**< Event id for informing status of the distirbution was stopped.*/
    EVENT_DISTRIBUTION_QUEUED,   /**< Event id for informing status of a new distribution was queued.*/
    EVENT_DISTRIBUTION_DFU_READY,  /**< Event id for informing status of the distribution was dfu ready.*/
} bt_mesh_ota_initiator_event_e;

/*!
    @brief event of initiator
*/
typedef struct {
    bt_mesh_ota_initiator_event_e event_id; /**< Event id*/
    bt_mesh_ota_initiator_error_e error_code; /**< Status code*/
    uint32_t serial_number; /**< Serial number*/
    uint32_t firmware_id;   /**< Firmware id*/
    uint32_t time_escaped; /**< time escaped from started*/
} bt_mesh_ota_initiator_event_t;

/*************************************************************************
* Public functions
*************************************************************************/
/*!
    @brief Add as OTA initiator
    @param[in] element_idx Element index
    @param[in] serial_number Offset of serial number. The distribution tasks' serial numbers will start from it.
    @return true if success.
*/
bool bt_mesh_ota_initiator_init(uint16_t element_idx, uint32_t serial_number);


/*!
    @brief Start a distribution
    @param[in] manual_apply Don't apply firmware while downloading.This parameter is valied only when distributor and initiator was in the same device.
    @param[in] new_fw_id firmware ID.
    @param[in] dst_addr distributor's address
    @param[in] grp_addr updaters' group address
    @param[in] nodes updaters' array of unicaset address
    @param[in] node_count Number of updaters.
    @return If succeeded.
*/
bool bt_mesh_ota_initiator_start_distribution(bool manual_apply, uint32_t new_fw_id, uint16_t dst_addr, uint16_t grp_addr, uint16_t nodes[], uint16_t node_count);

/*!
    @brief Stop an ongoing distribution
    @param[in] new_fw_id firmware ID.
    @param[in] dst_addr distributor's address
*/
void bt_mesh_ota_initiator_stop_distribution(uint32_t new_fw_id, uint16_t dst_addr);

/*!
    @brief Get distribution detail
    @param[in] new_fw_id firmware ID.
    @param[in] dst_addr distributor's address
*/
void bt_mesh_ota_initiator_get_distribution_detail(uint32_t new_fw_id, uint16_t dst_addr);

/*!
    @brief Show the current ota conditions.
*/
void bt_mesh_ota_initiator_dump(void);


/*!
    @brief Apply the distribution. Only valided whil all nodes were in dfu ready state.
*/
void bt_mesh_ota_initiator_apply_distribution();

/*!
    @brief initiator event receiver function
    @param[in] evt Event of ota initiator
    @note This is a callback function. Application needs to implement to get the event.
*/
void bt_mesh_ota_initator_event(bt_mesh_ota_initiator_event_t *evt);

/*!
@}
@}
@}
*/

#endif

