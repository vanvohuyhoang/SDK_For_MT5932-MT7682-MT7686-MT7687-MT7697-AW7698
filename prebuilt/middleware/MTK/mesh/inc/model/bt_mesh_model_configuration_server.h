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

#ifndef __BT_MESH_MODEL_CONFIGURATION_SERVER_H__
#define __BT_MESH_MODEL_CONFIGURATION_SERVER_H__

#include "bt_mesh_access.h"

/**
 *   @addtogroup BluetoothMesh Mesh
 *   @{
 *   @addtogroup BluetoothMeshSigModel Sig_Model
 *   @{
 *   @addtogroup BluetoothMeshConfigModel Configuration_Model
 *   @{
 *      bt_mesh_model_configuration_server.h defines the Configuration Server Model APIs.
 *   This section introduces the Configuration Server Model APIs. It shows how to add this model and to handle responsed event.
 *
 * @section bt_mesh_config_server_api_usage How to add a configuration server.
 *
 * - The configuration server is a mandatory server in each mesh device. This server can only be added in primary element.
 * - Use #bt_mesh_model_add_configuration_server to add a configuration server to your device.
 * @code
    static void add_config_server()
    {
        uint16_t model_handle;
        bt_mesh_model_add_configuration_server(&model_handle, NULL);
    }
 * @endcode
 * - All configuration server behaviours are handled and responsed in mesh stack. If you want to monitor what event is received, please register the callback.
 * - Here is an example for retrieving ConfigModelSubscriptionAdd message.
 * @code
    static void add_config_server()
    {
        uint16_t model_handle;
        bt_mesh_model_add_configuration_server(&model_handle, _configuration_server_msg_handler);
    }

    static void _configuration_server_msg_handler(uint16_t model_handle, const bt_mesh_access_message_rx_t *msg, const void *arg)
    {
        switch(msg->opcode.opcode) {
            case BT_MESH_ACCESS_MSG_CONFIG_MODEL_SUBSCRIPTION_ADD: {
                printf("ConfigModelSubscriptionAdd\n");

                uint16_t addr, element_addr;
                uint32_t model_id = 0;
                uint8_t idlen = msg->length - 4;

                // Please refer to Mesh Specification V1.0, chapter 4.3.2 for detail of the message payload.
                memcpy(&element_addr, msg->buffer, 2);
                memcpy(&addr, &msg->buffer[2], 2);
                if (idlen == 2) {
                    memcpy(&model_id, &msg->buffer[4], 2);
                } else {
                    model_id = (msg->buffer[5] << 24) | (msg->buffer[4] << 16) | (msg->buffer[7] << 8) | (msg->buffer[6]);
                }

                printf("\tElementAddr = 0x%04x\n", element_addr);
                printf("\tAddr = 0x%04x", addr);
                if (idlen == 2) {
                    printf("\tModelId[Sig] = 0x%04lx\n", model_id);
                } else if (idlen == 4) {
                    printf("\tModelId[Vendor] = 0x%08lx\n", model_id);
                }
                break;
            }
            default:
                break;
        }
    }
 * @endcode
*/

/*!
    @brief Add a configuration server model
    @param[out] model_handle is the handle of this added model.
    @param[in] callback is the message handler for configuration server model
    @return
    @c true means the configuration server model was added successfully. \n
    @c false means adding the configuration server model failed.
    @note Configuration server model is always added in the primary element.
*/
bool bt_mesh_model_add_configuration_server(uint16_t *model_handle, bt_mesh_access_msg_handler callback);

/*!
@}
@}
@}
@}
*/

#endif // __BT_MESH_MODEL_CONFIGURATION_SERVER_H__

