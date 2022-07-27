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

#ifndef BT_MESH_OTA_DISTRIBUTOR_HH
#define BT_MESH_OTA_DISTRIBUTOR_HH

#include <stdbool.h>
#include <stdint.h>

#include "bt_mesh_access.h"

/**
 *   @addtogroup BluetoothMesh Mesh
 *   @{
 *   @addtogroup BluetoothMeshMiddleware Middleware
 *   @{
        bt_mesh_ota_provider.h defines the OTA distributor's APIs.
*/

/*************************************************************************
* Type define
*************************************************************************/
/**
 * @defgroup Bluetooth_mesh_middleware_struct Struct
 * @{
*/

/*!
    @brief callback functions for local firmware and storage
*/
typedef struct {
    bool (*firmware_is_exist)(uint32_t);        /**< Check the specificed firmware exist*/
    uint32_t (*firmware_get_blk_size)(bool is_log);    /**< Get the firmware block size*/
    void (*firmware_get_object_id)(uint8_t[8]);    /**< Get the object id of new firmware, should be unique to new firmware*/
    uint32_t (*firmware_get_object_size)(void); /**< Get firmware object size*/
    uint8_t *(*firmware_get_block_data)(uint8_t); /**< Get firmware block data*/
} bt_mesh_ota_distributor_new_firmware_t;
/**
 * @}
 */

/*************************************************************************
* Public functions
*************************************************************************/


/*!
    @brief Add as OTA distributor
    @param[in] element_idx Element index
    @param[in] new_firmware Callback functions of new firmware
    @return true if success.
*/
bool bt_mesh_ota_distributor_init(uint16_t element_idx, const bt_mesh_ota_distributor_new_firmware_t *new_firmware);

/*!
    @brief Set TTL of chunk data transfer
    @param[in] ttl TTL value
*/
void bt_mesh_ota_distributor_set_chunk_ttl(uint8_t ttl);

/*!
@}
@}
@}
*/

#endif

