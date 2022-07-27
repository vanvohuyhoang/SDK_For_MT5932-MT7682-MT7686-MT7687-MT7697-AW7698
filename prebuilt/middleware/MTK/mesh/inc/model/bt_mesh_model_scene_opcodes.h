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


#ifndef __BT_MESH_MODEL_SCENE_OPCODES_H__
#define __BT_MESH_MODEL_SCENE_OPCODES_H__

/**
 *   @addtogroup BluetoothMesh Mesh
 *   @{
 *   @addtogroup BluetoothMeshSigModel Sig_Model
 *   @{
 *   @addtogroup BluetoothMeshSceneModel Scene_Model
 *   @{
 *      bt_mesh_model_scene_opcodes.h defines the SIG Mesh Scene Model operation codes.
*/

/**
 * @defgroup Bluetooth_mesh_scene_model_define Define
 * @{
*/

/*!
     @name Scene model message opcode.
     @brief Opcode for scene models.
 */
#define BT_MESH_MODEL_SCENE_GET                                     0x8241
#define BT_MESH_MODEL_SCENE_RECALL                                  0x8242
#define BT_MESH_MODEL_SCENE_RECALL_UNACKNOWLEDGED                   0x8243
#define BT_MESH_MODEL_SCENE_STATUS                                  0x5E
#define BT_MESH_MODEL_SCENE_REGISTER_GET                            0x8244
#define BT_MESH_MODEL_SCENE_REGISTER_STATUS                         0x8245

/*!
     @name Scene setup model message opcode.
     @brief Opcode for scene setup models.
 */
#define BT_MESH_MODEL_SCENE_STORE                                   0x8246
#define BT_MESH_MODEL_SCENE_STORE_UNACKNOWLEDGED                    0x8247
#define BT_MESH_MODEL_SCENE_DELETE                                  0x829E
#define BT_MESH_MODEL_SCENE_DELETE_UNACKNOWLEDGED                   0x829F
/**
 * @}
 */

/*!
@}
@}
@}
@}
*/

#endif // __BT_MESH_MODEL_SCENE_OPCODES_H__

