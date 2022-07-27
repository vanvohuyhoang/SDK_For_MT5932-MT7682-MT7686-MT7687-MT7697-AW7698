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


#ifndef __BT_MESH_MODEL_LIGHTING_H__
#define __BT_MESH_MODEL_LIGHTING_H__

/**
 *   @addtogroup BluetoothMesh Mesh
 *   @{
 *   @addtogroup BluetoothMeshSigModel Sig_Model
 *   @{
 *   @addtogroup BluetoothMeshLightingModel Lighting_Model
 *   @{
 *      bt_mesh_model_lighting_server.h defines the SIG Mesh Lighting Model APIs.

* - Light servers define states for light controls, including lightness, color temperature, and color-changing lights.
* - To create Lightness CTL setup server in your device, call the API #bt_mesh_model_lighting_add_ctl_setup_server() declared in bt_mesh_model_lighting_server.h.
* - To create Lightness HSL setup server in your device, call the API #bt_mesh_model_lighting_add_hsl_setup_server() declared in bt_mesh_model_lighting_server.h.
* - When CTL (setup) server and HSL (setup) server are created, both would in turn adds another element in the device, the output parameter element_list lists all element index when the model is created.
* - Please note that neither of light servers is a root server, which means all light servers extend one or more servers.
* - Calling APIs for extended servers is not necessary, for example, if a Light Lightness Setup server needs to be created in a device.
* - As defined in SIG Mesh Model specification, the Light Lightness Setup Server model extends the Light Lightness Server model and the Generic Power OnOff Setup Server model.
* - In this case, only bt_mesh_model_lighting_add_lightness_setup_server() needs to be called, calling _add_lightness_server() and _add_generic_power_onoff_setup_server() is not necessary.
* - However, please note that state binding is required to be handled in application code since state values(structures) are all defined in applications.
* -
* - Please refer to sample code below for CTL server:
* - CTL server is designed to have two elements, each element has its own Generic Level Server. Hence messages of Generic Level Server( such as _GENERIC_LEVEL_GET) need to be re-directed to correct server according to the destination address.
* -
* -
* - Sample code:
*      @code

   typedef struct
   {
       uint16_t model_handle;
       lighting_lightness_state_t *lightness;
       uint16_t present_ctl_temperature;
       uint16_t target_ctl_temperature;
       uint16_t present_ctl_delta_uv;
       uint16_t target_ctl_delta_uv;
       uint16_t default_temperature;
       uint16_t default_delta_uv;
       uint16_t range_min; //temperature range min
       uint16_t range_max; //temperature range max
       uint8_t TID;
       uint8_t transition_time;
       uint8_t delay;
       generic_level_server_model_t *level_server;
       uint16_t element_index;
   }lighting_ctl_server_t;

   typedef struct
   {
       uint16_t model_handle;
       uint16_t present_ctl_temperature;
       uint16_t target_ctl_temperature;
       uint16_t present_ctl_delta_uv;
       uint16_t target_ctl_delta_uv;
       uint8_t TID;
       uint8_t transition_time;
       uint8_t delay;
       generic_level_server_model_t *level_server;
       uint16_t element_index;
   }lighting_ctl_temperature_server_t;

   static void _publish_ctl_status(void)
   {
       bt_mesh_access_message_tx_t msg;
       uint8_t buffer[4];
       memcpy(buffer, &gCTL_server->lightness->present_lightness, 2);
       memcpy(buffer+2, &gCTL_server->present_ctl_temperature, 2);
       msg.opcode.opcode = BT_MESH_MODEL_LIGHT_CTL_STATUS;
       msg.opcode.company_id = MESH_MODEL_COMPANY_ID_NONE;
       msg.buffer = buffer;
       msg.length = 4;

       bt_mesh_status_code_t ret = bt_mesh_access_model_publish(gCTL_server->model_handle, &msg);
   }

   static void _publish_ctl_temp_status(void)
   {
       bt_mesh_access_message_tx_t msg;
       uint8_t buffer[4];
       memcpy(buffer, &gCTL_temperature_server->present_ctl_temperature, 2);
       memcpy(buffer+2, &gCTL_temperature_server->present_ctl_delta_uv, 2);
       msg.opcode.opcode = BT_MESH_MODEL_LIGHT_CTL_TEMPERATURE_STATUS;
       msg.opcode.company_id = MESH_MODEL_COMPANY_ID_NONE;
       msg.buffer = buffer;
       msg.length = 4;

       bt_mesh_status_code_t ret = bt_mesh_access_model_publish(gCTL_temperature_server->model_handle, &msg);
   }


   static void _light_server_msg_handler(uint16_t model_handle, const bt_mesh_access_message_rx_t* msg, const void* arg)
   {
       {
           switch(msg->opcode.opcode)
           ...

           case BT_MESH_MODEL_GENERIC_LEVEL_GET:
           {
               if(gLevel_server->transition_time == 0)
                   length = 2;
               else
                   length = 5;

               //find the _GENERIC_LEVEL_GET is meant for which server
               if(bt_mesh_model_get_element_address(gCTL_server->element_index) == msg->meta_data.dst_addr)
               {
                   memcpy(reply, &gCTL_server->level_server->present_level, 2);
                   goto level_get_transition;
               }
               if(bt_mesh_model_get_element_address(gCTL_temperature_server->element_index) == msg->meta_data.dst_addr)
               {
                   memcpy(reply, &gCTL_temperature_server->level_server->present_level, 2);
                   goto level_get_transition;
               }
               {
                   memcpy(reply, &gLevel_server->present_level, 2);
               }

level_get_transition:
               if(length == 5)
               {
                   //calculate remaining_time

                   //find the _GENERIC_LEVEL_GET is meant for which server
                   if(bt_mesh_model_get_element_address(gCTL_server->element_index) == msg->meta_data.dst_addr)
                   {
                       memcpy(&reply[2], &gCTL_server->level_server->target_level, 2);
                       goto level_get_done;
                   }
                   if(bt_mesh_model_get_element_address(gCTL_temperature_server->element_index) == msg->meta_data.dst_addr)
                   {
                       memcpy(&reply[2], &gCTL_temperature_server->level_server->target_level, 2);
                       goto level_get_done;
                   }
                   {
                       memcpy(&reply[2], &gLevel_server->target_level, 2);
                   }

level_get_done:
                   reply[4] = remaining_time;
               }

               //reply with:
               bt_mesh_model_generic_level_status(model_handle, reply, length, msg);

               break;
           }
           case BT_MESH_MODEL_LIGHT_CTL_GET:
           {
               if(gCTL_server->lightness->target_lightness == gCTL_server->lightness->present_lightness &&
                   gCTL_server->target_ctl_temperature == gCTL_server->present_ctl_temperature &&
                   gCTL_server->transition_time == 0)
               {
                   length = 4;

                   memcpy(reply, &gCTL_server->lightness->present_lightness, 2);
                   memcpy(&reply[2], &gCTL_server->present_ctl_temperature, 2);
               }
               else
               {
                   //calculate remaining_time
                   length = 9;

                   memcpy(reply, &gCTL_server->lightness->present_lightness, 2);
                   memcpy(&reply[2], &gCTL_server->present_ctl_temperature, 2);
                   memcpy(&reply[4], &gCTL_server->lightness->target_lightness, 2);
                   memcpy(&reply[6], &gCTL_server->target_ctl_temperature, 2);
                   reply[8] = remaining_time;
               }

               //reply with:
               bt_mesh_model_lighting_ctl_status(model_handle, reply, length, msg);

           }
           case BT_MESH_MODEL_LIGHT_CTL_SET:
           {
               //check if temperature is in valid range

               if(msg->length > 7)
                   //rx message has the value of transition_time
               else
                   //rx message does not have the value of transition_time, use default_transition_time if applicable

               gCTL_server->lightness->target_lightness = msg->buffer
               gCTL_server->target_ctl_temperature = &msg->buffer[2]
               gCTL_server->target_ctl_delta_uv = &msg->buffer[4]

               if((gCTL_server->transition_time & 0x3F) != 0)
               {
                   length = 9;
                   //set a delay timer
                   //set value for CTL state when time is up
                   //bind the value for target states
               }
               else
               {
                   length = 4;
                   //immediately change current CTL values to the ones in rx message
                   //bind the value for current and target states

                   //publish ctl status and temperature status
                   _publish_ctl_status();
                   _publish_ctl_temp_status();
               }

               //reply with:
               bt_mesh_model_lighting_ctl_status(model_handle, reply, length, msg);

           }
       }
   }

   static void mesh_create_device(void)
   {
       //set default value
       gCTL_server->range_min = 0x320;
       gCTL_server->range_max = 0x4E20;

       bt_mesh_model_lighting_add_ctl_setup_server(&model_handle, &element_count, &element_list, element_index, _light_server_msg_handler, _light_server_publish_callback);
       gCTL_server->model_handle = bt_mesh_model_get_handle(element_index, BT_MESH_MODEL_SIG_MODEL_ID_LIGHT_CTL_SERVER);
       gCTL_server->level_server->model_handle = bt_mesh_model_get_handle(element_index, BT_MESH_MODEL_SIG_MODEL_ID_GENERIC_LEVEL_SERVER);
       gCTL_server->element_index = element_index;

       if(element_list != NULL && element_list[0] == element_index)
       {
           gCTL_temperature_server->element_index = element_list[1];
           gCTL_temperature_server->model_handle = bt_mesh_model_get_handle(gCTL_temperature_server->element_index, BT_MESH_MODEL_SIG_MODEL_ID_LIGHT_CTL_TEMPERATURE_SERVER);
           gCTL_temperature_server->level_server->model_handle = bt_mesh_model_get_handle(gCTL_temperature_server->element_index, BT_MESH_MODEL_SIG_MODEL_ID_GENERIC_LEVEL_SERVER);
       }

       //bind all CTL server states on startup
       _bind_ctl_temperature(MESH_MODEL_STATE_LIGHTING_CTL_TEMPERATURE, MESH_MODEL_BINDING_BOTH_VALUE);
   }

*      @endcode
*


*/

#include "bt_mesh_access.h"

/*!
    @brief Add a lighting lightness server model.
    @param[out] model_handle is the handle of this added model.
    @param[in] element_index is the index of element that this model to be added in.
    @param[in] callback is the message handler for lighting lightness server model.
    @param[in] publish_timeout_cb is the periodic publishing timeout callback.
    @return
    @c true means adding lighting lightness server model successfully. \n
    @c false means adding lighting lightness server model failed.
*/
bool bt_mesh_model_lighting_add_lightness_server(
    uint16_t *model_handle, uint16_t element_index,
    bt_mesh_access_msg_handler callback, bt_mesh_access_publish_timeout_cb_t publish_timeout_cb);

/*!
    @brief Add a lighting lightness setup server model.
    @param[out] model_handle is the handle of this added model.
    @param[in] element_index is the index of element that this model to be added in.
    @param[in] callback is the message handler for lighting lightness setup server model.
    @param[in] publish_timeout_cb is the periodic publishing timeout callback.
    @return
    @c true means adding lighting lightness setup server model successfully. \n
    @c false means adding lighting lightness setup server model failed.
*/
bool bt_mesh_model_lighting_add_lightness_setup_server(
    uint16_t *model_handle, uint16_t element_index,
    bt_mesh_access_msg_handler callback, bt_mesh_access_publish_timeout_cb_t publish_timeout_cb);

/*!
    @brief Add a lighting CTL server model.
    @param[out] model_handle is the handle of this added model.
    @param[out] element_count is the element count that are created for this model.
    @param[out] element_list is the list of elements created for this model.
    @param[in] element_index is the index of element that this model to be added in.
    @param[in] callback is the message handler for lighting CTL server model.
    @param[in] publish_timeout_cb is the periodic publishing timeout callback.
    @return
    @c true means adding lighting CTL server model successfully. \n
    @c false means adding lighting CTL server model failed.
    @note Element indexes are consecutive for all newly created elements.
*/
bool bt_mesh_model_lighting_add_ctl_server(
    uint16_t *model_handle, uint8_t *element_count, uint16_t **element_list, uint16_t element_index,
    bt_mesh_access_msg_handler callback, bt_mesh_access_publish_timeout_cb_t publish_timeout_cb);

/*!
    @brief Add a lighting CTL temperature server model.
    @param[out] model_handle is the handle of this added model.
    @param[in] element_index is the index of element that this model to be added in.
    @param[in] callback is the message handler for lighting CTL temperature server model.
    @param[in] publish_timeout_cb is the periodic publishing timeout callback.
    @return
    @c true means adding lighting CTL temperature server model successfully. \n
    @c false means adding lighting CTL temperature server model failed.
*/
bool bt_mesh_model_lighting_add_ctl_temperature_server(
    uint16_t *model_handle, uint16_t element_index,
    bt_mesh_access_msg_handler callback, bt_mesh_access_publish_timeout_cb_t publish_timeout_cb);

/*!
    @brief Add a lighting CTL setup server model.
    @param[out] model_handle is the handle of this added model.
    @param[out] element_count is the element count that are created for this model.
    @param[out] element_list is the list of elements created for this model.
    @param[in] element_index is the index of element that this model to be added in.
    @param[in] callback is the message handler for lighting CTL setup server model.
    @param[in] publish_timeout_cb is the periodic publishing timeout callback.
    @return
    @c true means adding lighting CTL setup server model successfully. \n
    @c false means adding lighting CTL setup server model failed.
    @note Element indexes are consecutive for all newly created elements.
*/
bool bt_mesh_model_lighting_add_ctl_setup_server(
    uint16_t *model_handle, uint8_t *element_count, uint16_t **element_list, uint16_t element_index,
    bt_mesh_access_msg_handler callback, bt_mesh_access_publish_timeout_cb_t publish_timeout_cb);

/*!
    @brief Add a lighting HSL server model.
    @param[out] model_handle is the handle of this added model.
    @param[out] element_count is the element count that are created this model.
    @param[out] element_list is the list of elements created for this model.
    @param[in] element_index is the index of element that this model to be added in.
    @param[in] callback is the message handler for lighting HSL server model.
    @param[in] publish_timeout_cb is the periodic publishing timeout callback.
    @return
    @c true means adding lighting HSL server model successfully. \n
    @c false means adding lighting HSL server model failed.
    @note Element indexes are consecutive for all newly created elements.
*/
bool bt_mesh_model_lighting_add_hsl_server(
    uint16_t *model_handle, uint8_t *element_count, uint16_t **element_list, uint16_t element_index,
    bt_mesh_access_msg_handler callback, bt_mesh_access_publish_timeout_cb_t publish_timeout_cb);

/*!
    @brief Add a lighting HSL hue server model.
    @param[out] model_handle is the handle of this added model.
    @param[in] element_index is the index of element that this model to be added in.
    @param[in] callback is the message handler for lighting HSL hue server model.
    @param[in] publish_timeout_cb is the periodic publishing timeout callback.
    @return
    @c true means adding lighting HSL hue server model successfully. \n
    @c false means adding lighting HSL hue server model failed.
*/
bool bt_mesh_model_lighting_add_hsl_hue_server(
    uint16_t *model_handle, uint16_t element_index,
    bt_mesh_access_msg_handler callback, bt_mesh_access_publish_timeout_cb_t publish_timeout_cb);

/*!
    @brief Add a lighting HSL saturation server model.
    @param[out] model_handle is the handle of this added model.
    @param[in] element_index is the index of element that this model to be added in.
    @param[in] callback is the message handler for lighting HSL saturation server model.
    @param[in] publish_timeout_cb is the periodic publishing timeout callback.
    @return
    @c true means adding lighting HSL saturation server model successfully. \n
    @c false means adding lighting HSL saturation server model failed.
*/
bool bt_mesh_model_lighting_add_hsl_saturation_server(
    uint16_t *model_handle, uint16_t element_index,
    bt_mesh_access_msg_handler callback, bt_mesh_access_publish_timeout_cb_t publish_timeout_cb);

/*!
    @brief Add a lighting HSL setup server model.
    @param[out] model_handle is the handle of this added model.
    @param[out] element_count is the element count that are created this model.
    @param[out] element_list is the list of elements created for this model.
    @param[in] element_index is the index of element that this model to be added in.
    @param[in] callback is the message handler for lighting HSL setup server model.
    @param[in] publish_timeout_cb is the periodic publishing timeout callback.
    @return
    @c true means adding lighting HSL setup server model successfully. \n
    @c false means adding lighting HSL setup server model failed.
    @note Element indexes are consecutive for all newly created elements.
*/
bool bt_mesh_model_lighting_add_hsl_setup_server(
    uint16_t *model_handle, uint8_t *element_count, uint16_t **element_list, uint16_t element_index,
    bt_mesh_access_msg_handler callback, bt_mesh_access_publish_timeout_cb_t publish_timeout_cb);

/*!
    @brief Sends a lighting lightness status message.
    @param[in] model_handle is the model handle which the message belongs to.
    @param[in] buffer is the message payload to be sent.
    @param[in] buffer_length is the message payload length.
    @param[in] msg is the received message which this API replies to.
    @return
    #BT_MESH_SUCCESS, requesting message is performed successfully. \n
    #BT_MESH_ERROR_INVALID_ADDR, cannot find corresponding address. \n
    #BT_MESH_ERROR_OOM, not enough memory for sending reply
*/
bt_mesh_status_t bt_mesh_model_lighting_lightness_status(uint16_t model_handle, uint8_t *buffer,
        uint8_t buffer_length, const bt_mesh_access_message_rx_t *msg);

/*!
    @brief Sends a lighting lightness linear status message.
    @param[in] model_handle is the model handle which the message belongs to.
    @param[in] buffer is the message payload to be sent.
    @param[in] buffer_length is the message payload length.
    @param[in] msg is the received message which this API replies to.
    @return
    #BT_MESH_SUCCESS, requesting message is performed successfully. \n
    #BT_MESH_ERROR_INVALID_ADDR, cannot find corresponding address. \n
    #BT_MESH_ERROR_OOM, not enough memory for sending reply
*/
bt_mesh_status_t bt_mesh_model_lighting_lightness_linear_status(uint16_t model_handle, uint8_t *buffer,
        uint8_t buffer_length, const bt_mesh_access_message_rx_t *msg);

/*!
    @brief Sends a lighting lightness last status message.
    @param[in] model_handle is the model handle which the message belongs to.
    @param[in] lightness is the message payload to be sent.
    @param[in] msg is the received message which this API replies to.
    @return
    #BT_MESH_SUCCESS, requesting message is performed successfully. \n
    #BT_MESH_ERROR_INVALID_ADDR, cannot find corresponding address. \n
    #BT_MESH_ERROR_OOM, not enough memory for sending reply
*/
bt_mesh_status_t bt_mesh_model_lighting_lightness_last_status(uint16_t model_handle, uint16_t lightness,
        const bt_mesh_access_message_rx_t *msg);

/*!
    @brief Sends a lighting lightness default status message.
    @param[in] model_handle is the model handle which the message belongs to.
    @param[in] lightness is the message payload to be sent.
    @param[in] msg is the received message which this API replies to.
    @return
    #BT_MESH_SUCCESS, requesting message is performed successfully. \n
    #BT_MESH_ERROR_INVALID_ADDR, cannot find corresponding address. \n
    #BT_MESH_ERROR_OOM, not enough memory for sending reply
*/
bt_mesh_status_t bt_mesh_model_lighting_lightness_default_status(uint16_t model_handle, uint16_t lightness,
        const bt_mesh_access_message_rx_t *msg);

/*!
    @brief Sends a lighting lightness range status message.
    @param[in] model_handle is the model handle which the message belongs to.
    @param[in] status is the status code for the requesting message.
    @param[in] range_min is the value of Range Min for light lightness range state.
    @param[in] range_max is the value of Range Max for light lightness range state.
    @param[in] msg is the received message which this API replies to.
    @return
    #BT_MESH_SUCCESS, requesting message is performed successfully. \n
    #BT_MESH_ERROR_INVALID_ADDR, cannot find corresponding address. \n
    #BT_MESH_ERROR_OOM, not enough memory for sending reply
*/
bt_mesh_status_t bt_mesh_model_lighting_lightness_range_status(uint16_t model_handle, uint8_t status,
        uint16_t range_min, uint16_t range_max, const bt_mesh_access_message_rx_t *msg);

/*!
    @brief Sends a lighting CTL status message.
    @param[in] model_handle is the model handle which the message belongs to.
    @param[in] buffer is the message payload to be sent.
    @param[in] buffer_length is the message payload length.
    @param[in] msg is the received message which this API replies to.
    @return
    #BT_MESH_SUCCESS, requesting message is performed successfully. \n
    #BT_MESH_ERROR_INVALID_ADDR, cannot find corresponding address. \n
    #BT_MESH_ERROR_OOM, not enough memory for sending reply
*/
bt_mesh_status_t bt_mesh_model_lighting_ctl_status(uint16_t model_handle, uint8_t *buffer,
        uint8_t buffer_length, const bt_mesh_access_message_rx_t *msg);

/*!
    @brief Sends a lighting CTL temperature range status message.
    @param[in] model_handle is the model handle which the message belongs to.
    @param[in] status is the status code for the requesting message.
    @param[in] range_min is the value of Range Min for CTL temperature range state.
    @param[in] range_max is the value of Range Max for CTL temperature range state.
    @param[in] msg is the received message which this API replies to.
    @return
    #BT_MESH_SUCCESS, requesting message is performed successfully. \n
    #BT_MESH_ERROR_INVALID_ADDR, cannot find corresponding address. \n
    #BT_MESH_ERROR_OOM, not enough memory for sending reply
*/
bt_mesh_status_t bt_mesh_model_lighting_ctl_temperature_range_status(uint16_t model_handle, uint8_t status,
        uint16_t range_min, uint16_t range_max, const bt_mesh_access_message_rx_t *msg);

/*!
    @brief Sends a lighting CTL temperature status message.
    @param[in] model_handle is the model handle which the message belongs to.
    @param[in] buffer is the message payload to be sent.
    @param[in] buffer_length is the message payload length.
    @param[in] msg is the received message which this API replies to.
    @return
    #BT_MESH_SUCCESS, requesting message is performed successfully. \n
    #BT_MESH_ERROR_INVALID_ADDR, cannot find corresponding address. \n
    #BT_MESH_ERROR_OOM, not enough memory for sending reply
*/
bt_mesh_status_t bt_mesh_model_lighting_ctl_temperature_status(uint16_t model_handle, uint8_t *buffer,
        uint8_t buffer_length, const bt_mesh_access_message_rx_t *msg);

/*!
    @brief Sends a lighting CTL default status message.
    @param[in] model_handle is the model handle which the message belongs to.
    @param[in] lightness is the value of lightness default state.
    @param[in] temperature is the value of CTL temperature default state.
    @param[in] delta_uv is the value of CTL delta UV default state.
    @param[in] msg is the received message which this API replies to.
    @return
    #BT_MESH_SUCCESS, requesting message is performed successfully. \n
    #BT_MESH_ERROR_INVALID_ADDR, cannot find corresponding address. \n
    #BT_MESH_ERROR_OOM, not enough memory for sending reply
*/
bt_mesh_status_t bt_mesh_model_lighting_ctl_default_status(uint16_t model_handle, uint16_t lightness,
        uint16_t temperature, uint16_t delta_uv, const bt_mesh_access_message_rx_t *msg);

/*!
    @brief Sends a lighting HSL status message.
    @param[in] model_handle is the model handle which the message belongs to.
    @param[in] buffer is the message payload to be sent.
    @param[in] buffer_length is the message payload length.
    @param[in] msg is the received message which this API replies to.
    @return
    #BT_MESH_SUCCESS, requesting message is performed successfully. \n
    #BT_MESH_ERROR_INVALID_ADDR, cannot find corresponding address. \n
    #BT_MESH_ERROR_OOM, not enough memory for sending reply
*/
bt_mesh_status_t bt_mesh_model_lighting_hsl_status(uint16_t model_handle, uint8_t *buffer,
        uint8_t buffer_length, const bt_mesh_access_message_rx_t *msg);

/*!
    @brief Sends a lighting HSL target status message.
    @param[in] model_handle is the model handle which the message belongs to.
    @param[in] buffer is the message payload to be sent.
    @param[in] buffer_length is the message payload length.
    @param[in] msg is the received message which this API replies to.
    @return
    #BT_MESH_SUCCESS, requesting message is performed successfully. \n
    #BT_MESH_ERROR_INVALID_ADDR, cannot find corresponding address. \n
    #BT_MESH_ERROR_OOM, not enough memory for sending reply
*/
bt_mesh_status_t bt_mesh_model_lighting_hsl_target_status(uint16_t model_handle, uint8_t *buffer,
        uint8_t buffer_length, const bt_mesh_access_message_rx_t *msg);

/*!
    @brief Sends a lighting HSL hue status message.
    @param[in] model_handle is the model handle which the message belongs to.
    @param[in] buffer is the message payload to be sent.
    @param[in] buffer_length is the message payload length.
    @param[in] msg is the received message which this API replies to.
    @return
    #BT_MESH_SUCCESS, requesting message is performed successfully. \n
    #BT_MESH_ERROR_INVALID_ADDR, cannot find corresponding address. \n
    #BT_MESH_ERROR_OOM, not enough memory for sending reply
*/
bt_mesh_status_t bt_mesh_model_lighting_hsl_hue_status(uint16_t model_handle, uint8_t *buffer,
        uint8_t buffer_length, const bt_mesh_access_message_rx_t *msg);

/*!
    @brief Sends a lighting HSL saturation status message.
    @param[in] model_handle is the model handle which the message belongs to.
    @param[in] buffer is the message payload to be sent.
    @param[in] buffer_length is the message payload length.
    @param[in] msg is the received message which this API replies to.
    @return
    #BT_MESH_SUCCESS, requesting message is performed successfully. \n
    #BT_MESH_ERROR_INVALID_ADDR, cannot find corresponding address. \n
    #BT_MESH_ERROR_OOM, not enough memory for sending reply
*/
bt_mesh_status_t bt_mesh_model_lighting_hsl_saturation_status(uint16_t model_handle, uint8_t *buffer,
        uint8_t buffer_length, const bt_mesh_access_message_rx_t *msg);

/*!
    @brief Sends a lighting HSL default status message.
    @param[in] model_handle is the model handle which the message belongs to.
    @param[in] lightness is the the value of lightness default state.
    @param[in] hue is the value of Light HSL Hue default state.
    @param[in] saturation is the value of Light HSL saturation default state.
    @param[in] msg is the received message which this API replies to.
    @return
    #BT_MESH_SUCCESS, requesting message is performed successfully. \n
    #BT_MESH_ERROR_INVALID_ADDR, cannot find corresponding address. \n
    #BT_MESH_ERROR_OOM, not enough memory for sending reply
*/
bt_mesh_status_t bt_mesh_model_lighting_hsl_default_status(uint16_t model_handle, uint16_t lightness,
        uint16_t hue, uint16_t saturation, const bt_mesh_access_message_rx_t *msg);

/*!
    @brief Sends a lighting HSL range status message.
    @param[in] model_handle is the model handle which the message belongs to.
    @param[in] status is the status code for the requesting message.
    @param[in] hue_range_min is the value of Hue Range Min field of Light HSL Hue Range state.
    @param[in] hue_range_max is the value of Hue Range Max field of Light HSL Hue Range state.
    @param[in] saturation_range_min is value of Saturation Range Min field of Light HSL Saturation Range state.
    @param[in] saturation_range_max is value of Saturation Range Max field of Light HSL Saturation Range state.
    @param[in] msg is the received message which this API replies to.
    @return
    #BT_MESH_SUCCESS, requesting message is performed successfully. \n
    #BT_MESH_ERROR_INVALID_ADDR, cannot find corresponding address. \n
    #BT_MESH_ERROR_OOM, not enough memory for sending reply
*/
bt_mesh_status_t bt_mesh_model_lighting_hsl_range_status(uint16_t model_handle, uint8_t status,
        uint16_t hue_range_min, uint16_t hue_range_max, uint16_t saturation_range_min,
        uint16_t saturation_range_max, const bt_mesh_access_message_rx_t *msg);

/*!
    @brief Add a lighting XYL server model.
    @param[out] model_handle is the handle of this added model.
    @param[in] element_index is the index of element that this model to be added in.
    @param[in] callback is the message handler for lighting XYL server model.
    @param[in] publish_timeout_cb is the periodic publishing timeout callback.
    @return
    @c true means adding lighting XYL server model successfully. \n
    @c false means adding lighting XYL server model failed.
*/
bool bt_mesh_model_lighting_add_xyl_server(
    uint16_t *model_handle, uint16_t element_index,
    bt_mesh_access_msg_handler callback, bt_mesh_access_publish_timeout_cb_t publish_timeout_cb);

/*!
    @brief Add a lighting XYL setup server model.
    @param[out] model_handle is the handle of this added model.
    @param[in] element_index is the index of element that this model to be added in.
    @param[in] callback is the message handler for lighting XYL setup server model.
    @param[in] publish_timeout_cb is the periodic publishing timeout callback.
    @return
    @c true means adding lighting XYL setup server model successfully. \n
    @c false means adding lighting XYL setup server model failed.
*/
bool bt_mesh_model_lighting_add_xyl_setup_server(
    uint16_t *model_handle, uint16_t element_index,
    bt_mesh_access_msg_handler callback, bt_mesh_access_publish_timeout_cb_t publish_timeout_cb);

/*!
    @brief Send a lighting xyL status message.
    @param[in] model_handle is the model handle which the message belongs to.
    @param[in] buffer is the message payload to be sent.
    @param[in] buffer_length is the message payload length.
    @param[in] msg is the received message which this API replies to.
    @return
    #BT_MESH_SUCCESS, requesting message is performed successfully. \n
    #BT_MESH_ERROR_INVALID_ADDR, cannot find corresponding address. \n
    #BT_MESH_ERROR_OOM, not enough memory for sending reply
*/
bt_mesh_status_t bt_mesh_model_lighting_xyl_status(uint16_t model_handle, uint8_t *buffer,
        uint8_t buffer_length, const bt_mesh_access_message_rx_t *msg);

/*!
    @brief Send a lighting xyL target status message.
    @param[in] model_handle is the model handle which the message belongs to.
    @param[in] buffer is the message payload to be sent.
    @param[in] buffer_length is the message payload length.
    @param[in] msg is the received message which this API replies to.
    @return
    #BT_MESH_SUCCESS, requesting message is performed successfully. \n
    #BT_MESH_ERROR_INVALID_ADDR, cannot find corresponding address. \n
    #BT_MESH_ERROR_OOM, not enough memory for sending reply
*/
bt_mesh_status_t bt_mesh_model_lighting_xyl_target_status(uint16_t model_handle, uint8_t *buffer,
        uint8_t buffer_length, const bt_mesh_access_message_rx_t *msg);

/*!
    @brief Send a lighting xyL default status message.
    @param[in] model_handle is the model handle which the message belongs to.
    @param[in] xyl_lightness is the value of Light xyL Lightness default state.
    @param[in] xyl_x is the value of Light xyL x default state.
    @param[in] xyl_y is the value of Light xyL y default state.
    @param[in] msg is the received message which this API replies to.
    @return
    #BT_MESH_SUCCESS, requesting message is performed successfully. \n
    #BT_MESH_ERROR_INVALID_ADDR, cannot find corresponding address. \n
    #BT_MESH_ERROR_OOM, not enough memory for sending reply
*/
bt_mesh_status_t bt_mesh_model_lighting_xyl_default_status(uint16_t model_handle, uint16_t xyl_lightness,
        uint16_t xyl_x, uint16_t xyl_y, const bt_mesh_access_message_rx_t *msg);

/*!
    @brief Send a lighting xyL range status message.
    @param[in] model_handle is the model handle which the message belongs to.
    @param[in] status is the status code for the requesting message.
    @param[in] xyl_x_range_min is xyL x Range Min field of the Light xyL Range state.
    @param[in] xyl_x_range_max is xyL x Range Max field of the Light xyL Range state.
    @param[in] xyl_y_range_min is xyL y Range Min field of the Light xyL Range state.
    @param[in] xyl_y_range_max is xyL y Range Max field of the Light xyL Range state.
    @param[in] msg is the received message which this API replies to.
    @return
    #BT_MESH_SUCCESS, requesting message is performed successfully. \n
    #BT_MESH_ERROR_INVALID_ADDR, cannot find corresponding address. \n
    #BT_MESH_ERROR_OOM, not enough memory for sending reply
*/
bt_mesh_status_t bt_mesh_model_lighting_xyl_range_status(uint16_t model_handle, uint8_t status,
        uint16_t xyl_x_range_min, uint16_t xyl_x_range_max, uint16_t xyl_y_range_min,
        uint16_t xyl_y_range_max, const bt_mesh_access_message_rx_t *msg);
/*!
@}
@}
@}
*/

#endif // __BT_MESH_MODEL_LIGHTING_SERVER_H__

