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

#ifndef __BT_MESH_FRIEND_H__
#define __BT_MESH_FRIEND_H__

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

/**
 *   @addtogroup BluetoothService Bluetooth Services
 *   @{
 *   @addtogroup BluetoothMesh Mesh
 *   @{
 *   @addtogroup BluetoothMeshFeatures Features
 *   @{
 *      This section introduces how to use enums and structures to create a node with Friend feature supported and enabled, and use an event to monitor the status of friendships.
 *      @section bt_mesh_friend_api_usage How to create a Friend node
 *     - The application calls functions #bt_mesh_model_set_composition_data_header(),  #bt_mesh_init().
 *     - First, please refer to \ref bt_mesh_model_api_usage to create a mesh device.
 *        To have Friend feature supported and enabled, #BT_MESH_FEATURE_FRIEND is needed to be add in feature field in composition data header.
 *     - Second, please refer to \ref bt_mesh_api_usage to start Mesh module and use the friend initialization parameter to configure this node.
 *       If the field of friend initialization parameter is set to NULL, default Friend node setting will be applied.
 *     - When receiving a Friend Request that fulfills the requirements specified in the message parameters from Low Power node within a single hop of each other in the same mesh network, friend establishment will be started automatically.
 *     - The Friend feature operations: friend establishment, friend messaging, and friend management will be processed automatically by Mesh Friend module.
 *     - The status of friendships can be monitored through the event @ref BT_MESH_EVT_FRIENDSHIP_STATUS.
 *
 *     - Sample code:
 *         @code
            static void create_mesh_device(void)
            {
                uint8_t composition_data_header[10] =
                {
                    0x94, 0x00, // cid
                    0x1A, 0x00, // pid
                    0x01, 0x00, // vid
                    0x08, 0x00, // crpl
                    BT_MESH_FEATURE_FRIEND, 0x00, // features
                };

                ...

                bt_mesh_model_set_composition_data_header(10, composition_data_header);

                ...
            }

            static void meshinit()
            {
                printf("Mesh initialising...\n");
                bt_mesh_init_params_t *initparams = (bt_mesh_init_params_t *)malloc(sizeof(bt_mesh_init_params_t));
                bt_mesh_debug_init_params_t debug_param = {
                    .verbose_level = BT_MESH_DEBUG_APPLICATION | BT_MESH_DEBUG_MODEL | BT_MESH_DEBUG_MIDDLEWARE | BT_MESH_DEBUG_ACCESS | BT_MESH_DEBUG_FRIEND,
                    .info_level = 0,
                    .notify_level = BT_MESH_DEBUG_CONFIG | BT_MESH_DEBUG_PROVISION | BT_MESH_DEBUG_TRANSPORT | BT_MESH_DEBUG_NETWORK,
                    .warning_level = 0
                };
                initparams->debug = &debug_param;

                ...

                // init friend parameter
                initparams->friend = (bt_mesh_friend_init_params_t *)malloc(sizeof(bt_mesh_friend_init_params_t));
                initparams->friend->lpn_number = 2;
                initparams->friend->queue_size = 2;
                initparams->friend->subscription_list_size = 5;

                bt_mesh_init(initparams);
                free(initparams->friend);
                ...
                free(initparams);
            }

            bt_mesh_status_t bt_mesh_app_event_callback(bt_mesh_event_id evt_id, bool status, bt_mesh_evt_t *evt_data)
            {
                switch (evt_id) {
                    case BT_MESH_EVT_INIT_DONE: {
                         case BT_MESH_EVT_FRIENDSHIP_STATUS: {
                            bt_mesh_evt_friendship_status_t *p = &evt->evt.mesh_evt.mesh.friendship_status;
                            printf("BT_MESH_EVT_FRIENDSHIP_STATUS\n");
                            printf("\tLPN Address : 0x%03x", p->address);
                            printf("\tStatus      : %s", p->status? "FRIENDSHIP_ESTABLISHED":"FRIENDSHIP_TERMINATED");
                            break;
                        }
                    }

                return BT_MESH_SUCCESS;
            }

*          @endcode
*/

/**
 * @defgroup Bluetooth_mesh_features_struct Struct
 * @{
 */

/**
 * @brief The initialization parameters of Friend node. This parameter is used in #bt_mesh_init.
 */
typedef struct {
    uint8_t lpn_number;                     /**< The maximum number of Low Power nodes to be friends with. The default value is 2. */
    uint8_t queue_size;                     /**< The maximum message size can be stored in message queue for a Low Power node. The default value is 2. */
    uint8_t subscription_list_size;         /**< The maximum address count of the subscription list supported for a Low Power node. The default value is 5. */
} bt_mesh_friend_init_params_t;
/**
 * @}
 */

/*!
@}
@}
@}
*/

#endif // __BT_MESH_FRIEND_H__
