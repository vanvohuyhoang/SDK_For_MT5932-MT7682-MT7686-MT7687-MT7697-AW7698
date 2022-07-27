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


#ifndef __BT_MESH_MODEL_SENSOR_OPCODES_H__
#define __BT_MESH_MODEL_SENSOR_OPCODES_H__

/**
 *   @addtogroup BluetoothMesh Mesh
 *   @{
 *   @addtogroup BluetoothMeshSigModel Sig_Model
 *   @{
 *   @addtogroup BluetoothMeshSensorModel Sensor_Model
 *   @{
 *      bt_mesh_model_sensor_opcodes.h defines the SIG Mesh Sensor Model operation codes.
*/

/**
 * @defgroup Bluetooth_mesh_sensor_model_define Define
 * @{
 */

/*!
     @name Sensor model message opcode.
     @brief Opcode for sensor models.
     @{
 */
#define BT_MESH_MODEL_SENSOR_DESCRIPTOR_GET                    0x8230 /**< opcode for Sensor Descriptor Get */
#define BT_MESH_MODEL_SENSOR_DESCRIPTOR_STATUS                 0x51   /**< opcode for Sensor Descriptor Status */
#define BT_MESH_MODEL_SENSOR_GET                               0x8231 /**< opcode for Sensor Get */
#define BT_MESH_MODEL_SENSOR_STATUS                            0x52   /**< opcode for Sensor Status */
#define BT_MESH_MODEL_SENSOR_COLUMN_GET                        0x8232 /**< opcode for Sensor Column Get */
#define BT_MESH_MODEL_SENSOR_COLUMN_STATUS                     0x53   /**< opcode for Sensor Column Status */
#define BT_MESH_MODEL_SENSOR_SERIES_GET                        0x8233 /**< opcode for Sensor Series Get */
#define BT_MESH_MODEL_SENSOR_SERIES_STATUS                     0x54   /**< opcode for Sensor Series Status */
/*! @} */

/*!
     @name Sensor setup model message opcode.
     @brief Opcode for sensor models.
     @{
 */
#define BT_MESH_MODEL_SENSOR_CADENCE_GET                       0x8234 /**< opcode for Sensor Cadence Get */
#define BT_MESH_MODEL_SENSOR_CADENCE_SET                       0x55   /**< opcode for Sensor Cadence Set */
#define BT_MESH_MODEL_SENSOR_CADENCE_SET_UNACKNOWLEDGED        0x56   /**< opcode for Sensor Cadence Set Unacknowledged */
#define BT_MESH_MODEL_SENSOR_CADENCE_STATUS                    0x57   /**< opcode for Sensor Cadence Status */
#define BT_MESH_MODEL_SENSOR_SETTINGS_GET                      0x8235 /**< opcode for Sensor Settings Get */
#define BT_MESH_MODEL_SENSOR_SETTINGS_STATUS                   0x58   /**< opcode for Sensor Settings Status */
#define BT_MESH_MODEL_SENSOR_SETTING_GET                       0x8236 /**< opcode for Sensor Setting Get */
#define BT_MESH_MODEL_SENSOR_SETTING_SET                       0x59   /**< opcode for Sensor Setting Set */
#define BT_MESH_MODEL_SENSOR_SETTING_SET_UNACKNOWLEDGED        0x5A   /**< opcode for Sensor Setting Set Unacknowledged */
#define BT_MESH_MODEL_SENSOR_SETTING_STATUS                    0x5B   /**< opcode for Sensor Setting Status */
/*! @} */

/*!
@}
@}
@}
@}
*/

#endif // __BT_MESH_MODEL_SENSOR_OPCODES_H__
