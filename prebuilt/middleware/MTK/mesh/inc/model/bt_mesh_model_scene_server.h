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

#ifndef __BT_MESH_MODEL_SCENE_SERVER_H__
#define __BT_MESH_MODEL_SCENE_SERVER_H__

/**
 *   @addtogroup BluetoothMesh Mesh
 *   @{
 *   @addtogroup BluetoothMeshSigModel Sig_Model
 *   @{
 *   @addtogroup BluetoothMeshSceneModel Scene_Model
 *   @{
 *      bt_mesh_model_scene_server.h defines the SIG Mesh Scene Server Model APIs.
*
* @section bt_mesh_scene_api_usage How to add and use a scene server model.
* - Scenes serve as memory banks for storage of states (e.g., a light level/color). Values of states of an element can be stored as a scene and can be recalled later from the scene memory.
* - To create scene setup server in your device, call the API #bt_mesh_model_add_scene_setup_server() declared in bt_mesh_model_scene_server.h. Please note that the scene server should only be created in the primary element.
* - SIG Mesh Model specification requests that a scene server should include 16 scene registers to store neccessary data.
* - When the device receives a message with op code BT_MESH_MODEL_SCENE_STORE, it has to store the values for some specified stated.
* - When the device receives a message with op code BT_MESH_MODEL_SCENE_RECALL, it has to recall the stored scene, applying the values in the scene register to current states of server model.
* - States to be stored and not to be stored are specified in the Mesh Model specification. Please look into spec to know what states need to be stored in your application.
* - For every model table which lists states and bindings, there also exists a column "Stored with Scene" specifying what states of a server are required to be store if scene server is supported.
* - Take Generic OnOff Server for example, please refer to Table 3.85 in section 3.3.1.1, the column "Stored with Scene" marks the state Generic OnOff with a yes, which means Generic OnOff states needs to be stored in the scene register.
* - The example below shows a device that supports scene server is also lightness server, and hence extends a generic server. According to "Table 6.117: Light Lightness Server states and bindings" in model spec, there are 4 states need to be stored.
* - Please refer to sample code for a sample stucture: the scene server model should contain 16 registers for storing state, and each register contains required states.
* - Sample code:
*      @code
   typedef struct {
       uint16_t generic_onoff;
       uint16_t generic_level;
   } scene_generic_server_t;

   typedef struct {
       uint16_t light_lightness_actual;
       uint16_t light_lightness_linear;
   } scene_lighting_server_t;

   typedef struct
   {
       uint16_t scene_number;
       scene_generic_server_t generic_scene;
       scene_lighting_server_t lightness_scene;
   } scene_register;

   typedef struct {
       scene_register register_state[SCENE_REGISTER_NUMBER];
       uint16_t current_scene;
       uint16_t target_scene;
       uint8_t TID;
       uint8_t transition_time;
       uint8_t delay;
   } scene_server_model_t;

   static void _publish_scene_status(void)
   {
       bt_mesh_access_message_tx_t msg;
       uint8_t buffer[3];
       buffer[0] = 0;
       memcpy(buffer+1, &gScene_server->current_scene, 2);
       msg.opcode.opcode = BT_MESH_MODEL_SCENE_STATUS;
       msg.opcode.company_id = MESH_MODEL_COMPANY_ID_NONE;
       msg.buffer = buffer;
       msg.length = 3;

       bt_mesh_status_t ret = bt_mesh_access_model_publish(gScene_server->model_handle, &msg);
   }

   static void _light_server_msg_handler(uint16_t model_handle, const bt_mesh_access_message_rx_t* msg, const void* arg)
   {
       switch(msg->opcode.opcode)
       {
           case BT_MESH_MODEL_SCENE_GET:
           {
               if(transition_time == 0)
               {
                   //copy gScene_server->current_scene into tx payload
               }
               else
               {
                   //copy current_scene, target_scene and remaining_time into tx payload
               }

               //reply with:
               bt_mesh_model_scene_status(model_handle, reply, length, msg);
               break;
           }
           case BT_MESH_MODEL_SCENE_RECALL:
           {
               //####Field Size     (octets)  Notes
               //Scene_Number     2        The number of the scene to be recalled.
               //TID              1        Transaction Identifier.
               //transition_time  1        Format as defined in Section 3.1.3. (Optional)
               //Delay            1        Message
               //####Table 5.25: Scene Recall message parameters######

               if(transition_time == 0)
               {
                   //immediately recall scene[Scene_Number] previously stored in scene server
                   //publish scene status (see _publish_scene_status() above)
               }
               else
               {
                   //copy Scene_Number into target_scene and start countdown timer for the scene to take effect
                   //when the timer times out, apply the value of scene_register[Scene_Number] to lightness server and generic onOff/lever server
                   //publish scene status in timer handler
               }

               //reply with:
               bt_mesh_model_scene_status(model_handle, reply, length, msg);
               break;
           }
           case BT_MESH_MODEL_SCENE_RECALL_UNACKNOWLEDGED:
           {
               //do what BT_MESH_MODEL_SCENE_RECALL does EXCEPT NOT replying with bt_mesh_model_scene_status()
               break;
           }
           case BT_MESH_MODEL_SCENE_REGISTER_GET:
           {
               //copy every scene number of stored scene_register state

               //reply with:
               bt_mesh_model_scene_register_status(model_handle, reply, 3+2*(count), msg);
               break;
           }
           case BT_MESH_MODEL_SCENE_STORE:
           {
               //####Field Size     (octets)  Notes
               //Scene_Number      2        The number of the scene to be stored.
               //####Table 5.23: Scene Store message parameters

               //store all neccecary states with Scene_Number assigned in rx message
               //eg.
               gScene_server->register_state[index].scene_number = scene_number;
               gScene_server->register_state[index].generic_scene.generic_onoff = gOn_off_server->present_on_off;
               gScene_server->register_state[index].generic_scene.generic_level = gLevel_server->present_level;

               gScene_server->register_state[index].lightness_scene.light_lightness_actual = gLightness_server->lightness->present_lightness;
               gScene_server->register_state[index].lightness_scene.light_lightness_linear = gLightness_server->present_linear_lightness;

               //reply with:
               bt_mesh_model_scene_register_status(model_handle, reply, 3+2*(count), msg);
               break;
           }
           case BT_MESH_MODEL_SCENE_STORE_UNACKNOWLEDGED:
           {
               //do what BT_MESH_MODEL_SCENE_STORE does EXCEPT NOT replying with bt_mesh_model_scene_register_status()
               break;
           }
           case BT_MESH_MODEL_SCENE_DELETE:
           {
               //####Field Size   (octets)   Notes
               //Scene_Number   2          The number of the scene to be deleted.
               //####Table 5.29: Scene Delete message parameter

               //Delete content of register_state[Scene_Number]
               //publish scene status (see _publish_scene_status() above)

               //reply with:
               bt_mesh_model_scene_register_status(model_handle, reply, 3+2*(count), msg);
               break;
           }
           case BT_MESH_MODEL_SCENE_DELETE_UNACKNOWLEDGED:
           {
               //do what BT_MESH_MODEL_SCENE_DELETE does EXCEPT NOT replying with bt_mesh_model_scene_register_status()
               break;
           }
       }
   }

   static void mesh_create_device(void)
   {
       gScene_server = malloc(sizeof(scene_server_model_t));
       memset(gScene_server, 0, sizeof(scene_server_model_t));

       bt_mesh_model_add_scene_setup_server(&model_handle, element_index, _light_server_msg_handler, NULL);
   }

*      @endcode
*

*/

#include "bt_mesh_access.h"

/*!
    @brief Add a scene server model.
    @param[out] model_handle is the handle of this added model.
    @param[in] element_index is the index of element that this model to be added in.
    @param[in] callback is the message handler for scene server model.
    @param[in] publish_timeout_cb is the periodic publishing timeout callback.
    @return
    @c true means adding scene server model successfully. \n
    @c false means adding scene server model failed.
*/
bool bt_mesh_model_add_scene_server(
    uint16_t *model_handle, uint16_t element_index,
    bt_mesh_access_msg_handler callback, bt_mesh_access_publish_timeout_cb_t publish_timeout_cb);

/*!
    @brief Add a scene setup server model.
    @param[out] model_handle is the handle of this added model.
    @param[in] element_index is the index of element that this model to be added in.
    @param[in] callback is the message handler for scene setup server model.
    @param[in] publish_timeout_cb is the periodic publishing timeout callback.
    @return
    @c true means adding scene setup server model successfully. \n
    @c false means adding scene setup server model failed.
*/
bool bt_mesh_model_add_scene_setup_server(
    uint16_t *model_handle, uint16_t element_index,
    bt_mesh_access_msg_handler callback, bt_mesh_access_publish_timeout_cb_t publish_timeout_cb);

/*!
    @brief Sends a scene status message.
    @param[in] model_handle is the model handle which the message belongs to.
    @param[in] buffer is the message payload to be sent.
    @param[in] buffer_length is the message payload length.
    @param[in] msg is the received message which this API replies to.
    @return
    #BT_MESH_SUCCESS, requesting message is performed successfully. \n
    #BT_MESH_ERROR_INVALID_ADDR, cannot find corresponding address. \n
    #BT_MESH_ERROR_OOM, not enough memory for sending reply
*/
bt_mesh_status_t bt_mesh_model_scene_status(uint16_t model_handle,
        uint8_t *buffer, uint8_t buffer_length, const bt_mesh_access_message_rx_t *msg);

/*!
    @brief Sends a scene register status message.
    @param[in] model_handle is the model handle which the message belongs to.
    @param[in] buffer is the message payload to be sent.
    @param[in] buffer_length is the message payload length.
    @param[in] msg is the received message which this API replies to.
    @return
    #BT_MESH_SUCCESS, requesting message is performed successfully. \n
    #BT_MESH_ERROR_INVALID_ADDR, cannot find corresponding address. \n
    #BT_MESH_ERROR_OOM, not enough memory for sending reply
*/
bt_mesh_status_t bt_mesh_model_scene_register_status(uint16_t model_handle,
        uint8_t *buffer, uint8_t buffer_length, const bt_mesh_access_message_rx_t *msg);

/*!
@}
@}
@}
*/

#endif // __BT_MESH_MODEL_SCENE_SERVER_H__


