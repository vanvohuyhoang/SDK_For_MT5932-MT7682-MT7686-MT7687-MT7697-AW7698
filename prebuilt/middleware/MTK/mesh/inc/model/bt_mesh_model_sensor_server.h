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

#ifndef __BT_MESH_MODEL_SENSOR_SERVER_H__
#define __BT_MESH_MODEL_SENSOR_SERVER_H__

/**
 *   @addtogroup BluetoothMesh Mesh
 *   @{
 *   @addtogroup BluetoothMeshSigModel Sig_Model
 *   @{
 *   @addtogroup BluetoothMeshSensorModel Sensor_Model
 *   @{
 *      bt_mesh_model_sensor_server.h defines the SIG Mesh Sensor Server Model APIs.

* - The Sensor state consists of four states: the Sensor Descriptor state, the Sensor Setting and Sensor Cadence states, and the measurement value, which may be represented as a single data point Sensor Data state or as a column of a series of data points, such as a histogram Sensor Series Column state.
* - To create sensor setup server in your device, call the API #bt_mesh_model_add_sensor_setup_server() declared in bt_mesh_model_time_server.h.
* - Property IDs, characteristic, and detailed description is defined in the spec Mesh Device Properties, please look into this spec for related sensor characteristics that your device need.
* - The Sensor Cadence controls the cadence of sensor reports. It allows a sensor to be configured to send measured values using Sensor Status messages at a different cadence for a range of measured values. It also allows a sensor to be configured to send measured values when the value changes up or down by more than a configured delta value.
* - Multiple instances of Sensor states may be present within the same model, provided that each instance has a unique value of the Sensor Property ID to allow the instances to be differentiated.
* - The Sensor Data is a sequence of one or more pairs of Sensor Property ID and Raw Value fields, with each Raw Value field size and representation defined by the characteristics referenced by the Sensor Property ID (in Mesh Device Properties).
* - When the Sensor Property refers multiple characteristics, the corresponding Raw Value field is a concatenated sequence of all formats defined by all characteristics.
* - Please refer to sample code below for a sample stucture:

* -
* - Sample code:
*      @code

   typedef struct
   {
       uint32_t sensor_positive_tolerance : 12;
       uint32_t sensor_negative_tolerance : 12;
       uint32_t sensor_sampling_function : 8;
       uint8_t sensor_measurement_period;
       uint8_t sensor_update_interval;
   } sensor_descriptor;

   typedef struct
   {
       uint8_t fast_cadence_period_divisor : 7;
       uint8_t status_trigger_type : 1;
       uint16_t status_trigger_delta_down;
       uint16_t status_trigger_delta_up;
       uint8_t status_min_interval;
       uint16_t fast_cadence_low;
       uint16_t fast_cadence_high;
   } sensor_cadence;

   typedef struct
   {
       uint8_t *sensor_raw_value_x;
       uint8_t *sensor_column_width;
       uint8_t *sensor_raw_value_y;
   } sensor_column;

   typedef struct
   {
       uint16_t sensor_setting_property_id;
       uint8_t sensor_setting_access;
       uint8_t *sensor_setting_raw;
       uint8_t sensor_setting_raw_length;
   } sensor_setting;

   typedef struct {
       uint16_t id;        // Property ID
       uint8_t *value;     // Property value
       uint16_t length;    // Property value length
       sensor_descriptor descriptor;  // Sensor descriptor
       sensor_cadence cadence;  // Sensor cadence of this property
       void *series; // Sensor series
       sensor_setting setting; // Sensor setting
   } bt_mesh_sensor_property_t;

   static void _sensor_server_msg_handler(uint16_t model_handle, const bt_mesh_access_message_rx_t* msg, const void* arg)
   {
           switch(msg->opcode.opcode)
           ...
           case BT_MESH_MODEL_SENSOR_DESCRIPTOR_GET:
           {
               if(msg->length == 2)
               {
                   //specified property, check if property_id valid

                   //look into device descriptor list(queue) for specific descriptor according to property_id

                   if(is_found_property)
                       bt_mesh_model_sensor_descriptor_status(model_handle, descriptor, 8, msg);
                   else //unknown property id
                       bt_mesh_model_sensor_descriptor_status(model_handle, descriptor, 2, msg);
               }
               else if(msg->length == 0)
               {
                   //copy all descriptors from device descriptor list
                   bt_mesh_model_sensor_descriptor_status(model_handle, descriptor, length, msg);
               }

               break;
           }
           case BT_MESH_MODEL_SENSOR_GET:
           {
               if(msg->length == 2)
               {
                   //specified property, check if property_id valid

                   //tx message length depends on what property_id is as defined in Mesh Device Properties

                   if(is_found_property)
                       bt_mesh_model_sensor_status(model_handle, reply, length, msg);
                   else
                   {
                       reply[0] = (property_id & 0x7) << 5 | (0 << 1);
                       reply[1] = (property_id & 0x7F8) >> 3;
                       bt_mesh_model_sensor_status(model_handle, reply, 2, msg);
                   }
               }
               else if(msg->length == 0)
               {
                   //all property, complete content of sensor data depends on implementation
                   //Please refer to Figure 4.4 in spec 4.2.14 for Sensor Data Marshalling

                   bt_mesh_model_sensor_status(model_handle, reply, length, msg);
               }

               break;
           }
   }

   static void mesh_create_device(void)
   {

       bt_mesh_model_add_sensor_setup_server(&model_handle, element_index, _sensor_server_msg_handler, NULL);

       //Add sensor property and set sensor states as an example
       bt_mesh_sensor_property_t *entry = (bt_mesh_sensor_property_t *)AB_queue_entry_alloc(sizeof(bt_mesh_sensor_property_t));
       sensor_column *column_entry = (sensor_column *)AB_queue_entry_alloc(sizeof(sensor_column));

       memset(entry, 0, sizeof(bt_mesh_sensor_property_t));

       entry->id = PROPERTY_ID_PEOPLE_COUNT;
       entry->length = 2;

       entry->descriptor.sensor_measurement_period = 0x10;

       entry->value = malloc(entry->length);
       entry->value[0] = 50;
       entry->value[1] = 0;

       entry->series = (void *)AB_queue_alloc();

       column_entry->sensor_raw_value_x = malloc(entry->length);
       memset(column_entry->sensor_raw_value_x, 1, entry->length);
       column_entry->sensor_column_width = malloc(entry->length);
       memset(column_entry->sensor_column_width, 1, entry->length);
       column_entry->sensor_raw_value_y = malloc(entry->length);
       memset(column_entry->sensor_raw_value_y, 1, entry->length);
       bt_mesh_os_layer_ds_queue_push(entry->series, column_entry);

       //sensor setting state
       entry->setting.sensor_setting_property_id = PROPERTY_ID_MOTION_SENSED;
       entry->setting.sensor_setting_raw_length = 1;
       entry->setting.sensor_setting_raw = malloc(entry->setting.sensor_setting_raw_length);

       bt_mesh_os_layer_ds_queue_push(sensor_queue, entry);

       entry = (bt_mesh_sensor_property_t *)AB_queue_entry_alloc(sizeof(bt_mesh_sensor_property_t));
       memset(entry, 0, sizeof(bt_mesh_sensor_property_t));
   }

*      @endcode
*

*/

#include "bt_mesh_access.h"

/*!
    @brief Add a sensor server model.
    @param[out] model_handle is the handle of this added model.
    @param[in] element_index is the index of element that this model to be added in.
    @param[in] callback is the message handler for sensor server model.
    @param[in] publish_timeout_cb is the periodic publishing timeout callback.
    @return
    @c true means adding sensor server model successfully. \n
    @c false means adding sensor server model failed.
*/
bool bt_mesh_model_add_sensor_server(
    uint16_t *model_handle, uint16_t element_index,
    bt_mesh_access_msg_handler callback, bt_mesh_access_publish_timeout_cb_t publish_timeout_cb);

/*!
    @brief Add a sensor setup server model.
    @param[out] model_handle is the handle of this added model.
    @param[in] element_index is the index of element that this model to be added in.
    @param[in] callback is the message handler for sensor setup server model.
    @param[in] publish_timeout_cb is the periodic publishing timeout callback.
    @return
    @c true means adding sensor setup server model successfully. \n
    @c false means adding sensor setup server model failed.
*/
bool bt_mesh_model_add_sensor_setup_server(
    uint16_t *model_handle, uint16_t element_index,
    bt_mesh_access_msg_handler callback, bt_mesh_access_publish_timeout_cb_t publish_timeout_cb);

/*!
    @brief Sends a sensor descriptor status message.
    @param[in] model_handle is the model handle which the message belongs to.
    @param[in] buffer is the message payload to be sent.
    @param[in] buffer_length is the message payload length.
    @param[in] msg is the received message which this API replies to.
    @return
    #BT_MESH_SUCCESS, requesting message is performed successfully. \n
    #BT_MESH_ERROR_INVALID_ADDR, cannot find corresponding address. \n
    #BT_MESH_ERROR_OOM, not enough memory for sending reply
*/
bt_mesh_status_t bt_mesh_model_sensor_descriptor_status(uint16_t model_handle,
        uint8_t *buffer, uint8_t buffer_length, const bt_mesh_access_message_rx_t *msg);

/*!
    @brief Sends a sensor cadence status message.
    @param[in] model_handle is the model handle which the message belongs to.
    @param[in] buffer is the message payload to be sent.
    @param[in] buffer_length is the message payload length.
    @param[in] msg is the received message which this API replies to.
    @return
    #BT_MESH_SUCCESS, requesting message is performed successfully. \n
    #BT_MESH_ERROR_INVALID_ADDR, cannot find corresponding address. \n
    #BT_MESH_ERROR_OOM, not enough memory for sending reply
*/
bt_mesh_status_t bt_mesh_model_sensor_cadence_status(uint16_t model_handle,
        uint8_t *buffer, uint8_t buffer_length, const bt_mesh_access_message_rx_t *msg);

/*!
    @brief Sends a sensor settings status message.
    @param[in] model_handle is the model handle which the message belongs to.
    @param[in] buffer is the message payload to be sent.
    @param[in] buffer_length is the message payload length.
    @param[in] msg is the received message which this API replies to.
    @return
    #BT_MESH_SUCCESS, requesting message is performed successfully. \n
    #BT_MESH_ERROR_INVALID_ADDR, cannot find corresponding address. \n
    #BT_MESH_ERROR_OOM, not enough memory for sending reply
*/
bt_mesh_status_t bt_mesh_model_sensor_settings_status(uint16_t model_handle,
        uint8_t *buffer, uint8_t buffer_length, const bt_mesh_access_message_rx_t *msg);

/*!
    @brief Sends a sensor setting status message.
    @param[in] model_handle is the model handle which the message belongs to.
    @param[in] buffer is the message payload to be sent.
    @param[in] buffer_length is the message payload length.
    @param[in] msg is the received message which this API replies to.
    @return
    #BT_MESH_SUCCESS, requesting message is performed successfully. \n
    #BT_MESH_ERROR_INVALID_ADDR, cannot find corresponding address. \n
    #BT_MESH_ERROR_OOM, not enough memory for sending reply
*/
bt_mesh_status_t bt_mesh_model_sensor_setting_status(uint16_t model_handle,
        uint8_t *buffer, uint8_t buffer_length, const bt_mesh_access_message_rx_t *msg);

/*!
    @brief Sends a sensor status message.
    @param[in] model_handle is the model handle which the message belongs to.
    @param[in] buffer is the message payload to be sent.
    @param[in] buffer_length is the message payload length.
    @param[in] msg is the received message which this API replies to.
    @return
    #BT_MESH_SUCCESS, requesting message is performed successfully. \n
    #BT_MESH_ERROR_INVALID_ADDR, cannot find corresponding address. \n
    #BT_MESH_ERROR_OOM, not enough memory for sending reply
*/
bt_mesh_status_t bt_mesh_model_sensor_status(uint16_t model_handle,
        uint8_t *buffer, uint8_t buffer_length, const bt_mesh_access_message_rx_t *msg);

/*!
    @brief Sends a sensor column status message.
    @param[in] model_handle is the model handle which the message belongs to.
    @param[in] buffer is the message payload to be sent.
    @param[in] buffer_length is the message payload length.
    @param[in] msg is the received message which this API replies to.
    @return
    #BT_MESH_SUCCESS, requesting message is performed successfully. \n
    #BT_MESH_ERROR_INVALID_ADDR, cannot find corresponding address. \n
    #BT_MESH_ERROR_OOM, not enough memory for sending reply
*/
bt_mesh_status_t bt_mesh_model_sensor_column_status(uint16_t model_handle,
        uint8_t *buffer, uint8_t buffer_length, const bt_mesh_access_message_rx_t *msg);

/*!
    @brief Sends a sensor series status message.
    @param[in] model_handle is the model handle which the message belongs to.
    @param[in] buffer is the message payload to be sent.
    @param[in] buffer_length is the message payload length.
    @param[in] msg is the received message which this API replies to.
    @return
    #BT_MESH_SUCCESS, requesting message is performed successfully. \n
    #BT_MESH_ERROR_INVALID_ADDR, cannot find corresponding address. \n
    #BT_MESH_ERROR_OOM, not enough memory for sending reply
*/
bt_mesh_status_t bt_mesh_model_sensor_series_status(uint16_t model_handle,
        uint8_t *buffer, uint8_t buffer_length, const bt_mesh_access_message_rx_t *msg);

/*!
@}
@}
@}
*/

#endif // __BT_MESH_MODEL_SENSOR_SERVER_H__

