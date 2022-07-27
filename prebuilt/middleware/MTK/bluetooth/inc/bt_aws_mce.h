/* Copyright Statement:
 *
 * (C) 2005-2017 MediaTek Inc. All rights reserved.
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
 * the License Agreement ("Permitted User"). If you are not a Permitted User,
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

#ifndef __BT_AWS_MCE_H__
#define __BT_AWS_MCE_H__

/**
 * @addtogroup Bluetooth
 * @{
 * @addtogroup BluetoothAWS-MCE AWS-MCE
 * @{
 * The Airoha Wireless Stereo (AWS) with Multicast Extension (MCE) is an Airoha proprietary profile 
 * to support voice/audio over multiple Bluetooth Audio devices via a special link called AWS_MCE. 
 * AWS_MCE Link is an extension for the Bluetooth link with an AWS_MCE packet/role extension mechanism. 
 * There is no extra cost for a piconet switch and less overhead compared with other solutions because 
 * only one Wireless Link is used in AWS.
 *
 * Terms and Acronyms
 * ======
 * |Terms                         |Details                                                                 |
 * |------------------------------|------------------------------------------------------------------------|
 * |\b AWS Link                   | AWS Link is an extension for the Bluetooth Link with a AWS PACKET/ROLE EXTENSION mechanism. |
 * |\b Agent role                 | Agent role is an extension role in the AWS Link. The device in the Agent role works in either the Master role or Slave role in AWS Link, which is connected with SP. |
 * |\b Client role                | Client role is an extension role in the AWS Link. In general, Client role does not give the feedback directly to the Phone role. It receives the data from either the Phone role or the Agent role.|
 * |\b Partner role               | Partner role is a special Client that can exchange information with the Agent. In general, there is only one Partner in an AWS-MCE link.|
 * |\b IF packet                  | IF Packet, information packet, is exchanged between the Agent and Partner, and is used to share sink A2DP, CALL or other status.|
 *
 * @section bt_aws_mce_api_usage How to use this module
 * This section presents the AWS-MCE connection handle and a method for the information packet.
 *  - 1. (Mandatory) Implement #bt_app_event_callback() to handle the AWS-MCE events, such as connect, state change, information update, and more.
 *   - Sample code:
 *    @code
 *       void bt_app_event_callback(bt_msg_type_t msg_type, bt_status_t status, void *data)
 *       {
 *          switch (msg_type)
 *          {
 *              case BT_AWS_MCE_CONNECTED:
 *              {
 *                  bt_aws_mce_connected_t *connected = (bt_aws_mce_connected_t *)data;
 *                  // AWS-MCE link connected.
 *                  // Records the connected information(handle, remote address).
 *                  break;
 *              }
 *              case BT_AWS_MCE_STATE_CHANGED_IND:
 *              {
 *                  // The state is only for the Agent.
  *                  bt_aws_mce_state_change_ind_t *changed_data = (bt_aws_mce_state_change_ind_t *)data;
 *                  // Do something in different new state.
 *                  break;
 *              }
 *              case BT_AWS_MCE_INFOMATION_PACKET_IND:
 *              {
 *                  bt_aws_mce_information_t *information = (bt_aws_mce_information_t *)data;
 *                  // The application can get the information packet type and related data to perform different operations.
 *                  switch (information->type) {
 *                      case BT_AWS_MCE_INFORMATION_A2DP: 
 *                      {
 *                          // enter A2DP related procedure.
 *                          break;
 *                      }
 *                      case BT_AWS_MCE_INFORMATION_SCO: 
 *                      {
 *                          // enter call related procedure.
 *                          break;
 *                      }
 *                      case BT_AWS_MCE_INFORMATION_REPORT_APP: 
 *                      {
 *                          // enter other application related procedure. For example, the key operation.
 *                          break;
 *                      }
 *                      default: 
 *                      {
 *                          break;
 *                      }
 *                  }
 *                  break;
 *              }
 *              default:
 *              {
 *                  break;
 *              }
 *           }
 *       }
 *    @endcode
 *  - 2. (Mandatory) Connect to a remote device, as shown in figure titled as "AWS-MCE connection establishment message sequence" in the Bluetooth developer's guide under the [sdk_root]/doc folder.
 *   - Sample code:
 *      Connect to a remote device by passing the remote Bluetooth address and getting the connection handle.
 *      The event #BT_AWS_MCE_CONNECTED captured by #bt_app_event_callback() indicates the AWS-MCE is connected.
 *    @code
 *       bt_aws_mce_connect(&handle, &addr);
 *    @endcode
  *  - 3. (Mandatory) Change the Agent state, as shown in figure titled as "AWS-MCE connection establishment message sequence" in the Bluetooth developer's guide under the [sdk_root]/doc folder.
 *   - Sample code:
 *      Change to connectable state when AWS-MCE is connected, so that Partner can attach to Agent.
 *      The event #BT_AWS_MCE_STATE_CHANGED_IND captured by #bt_app_event_callback() indicates the AWS-MCE agent state changed.
 *    @code
 *       bt_aws_mce_set_state(&handle, BT_AWS_MCE_AGENT_STATE_CONNECTABLE);
 *    @endcode
 *  - 4. (Mandatory) Synchronize the sink A2DP, sink Call, or other information (state change, a2dp play time with the Bluetooth clock) as shown in figure titled as "AWS-MCE synchronize the information" in the Bluetooth developer's guide under the [sdk_root]/doc folder.
 *   - Sample code:
 *      Call bt_aws_mce_send_information API to share the related data from the Agent to the Partner or the Partner to the Agent.
 *    @code
 *      bt_aws_mce_information_t a2dp_infomation;
 *      a2dp_infomation.type = BT_AWS_MCE_INFORMATION_A2DP;
 *      a2dp_infomation.data_length = sizeof(a2dp_data);
 *      a2dp_infomation.data = (uint8_t *)&a2dp_data;
 *      bt_aws_mce_send_information(aws_handle, &a2dp_infomation, true);
 *     // Plays the music when the time is up.
 *    @endcode
 */

  #include "bt_type.h"
 
  /**
 * @defgroup Bluetoothbt_aws_mce_define Define
 * @{
 */
 
/**
 * @brief An invalid value for the AWS-MCE handle. 
 */
#define BT_AWS_MCE_INVALID_HANDLE               0x00000000

/**
 * @brief Define for AWS-MCE role type.
 */
#define BT_AWS_MCE_ROLE_NONE                0x00          /**< No Role. */
#define BT_AWS_MCE_ROLE_CLINET              0x10          /**< Client Role. */
#define BT_AWS_MCE_ROLE_PARTNER             0x20          /**< Partner Role. */
#define BT_AWS_MCE_ROLE_AGENT               0x40          /**< Agent Role. */
typedef uint8_t bt_aws_mce_role_t;                        /**< The type of AWS-MCE role. */

/**
 * @brief Define for AWS-MCE state type.
 */
#define BT_AWS_MCE_AGENT_STATE_NONE         0x00          /**< No State. */
#define BT_AWS_MCE_AGENT_STATE_INACTIVE     0x40          /**< Inactive Agent State. In this state, the Agent is unaccessible by the Partner.*/
#define BT_AWS_MCE_AGENT_STATE_CONNECTABLE  0x50          /**< Connectable Agent State. In this state, the Agent is accessible by the Partner, and the Partner can scan for the Agent and connects with Agent.*/
#define BT_AWS_MCE_AGENT_STATE_ATTACHED     0x60          /**< Attached Agent State. The Agent enters this state after the Partner successfully connected wit Agent. */
typedef uint8_t bt_aws_mce_agent_state_type_t;            /**< The type of AWS-MCE Agent State. */

/**
 * @brief The event reported to the user.
 */
#define BT_AWS_MCE_CONNECTED                   (BT_MODULE_AWS_MCE | 0x0001)    /**< An attempt to connect from a remote device with the #bt_aws_mce_connected_t payload. */
#define BT_AWS_MCE_DISCONNECTED                (BT_MODULE_AWS_MCE | 0x0002)    /**< An attempt to disconnect is initiated by a local or remote device or a link loss occurred with the #bt_aws_mce_disconnected_t payload. */
#define BT_AWS_MCE_CALL_AUDIO_CONNECTED        (BT_MODULE_AWS_MCE | 0x0003)    /**< An attempt to connect from a remote device with the #bt_aws_mce_call_audio_connected_t payload. */
#define BT_AWS_MCE_CALL_AUDIO_DISCONNECTED     (BT_MODULE_AWS_MCE | 0x0004)    /**< An attempt to disconnect is initiated by a local or remote device or a link loss occurred with the #bt_aws_mce_call_audio_disconnected_t payload. */
#define BT_AWS_MCE_INFOMATION_PACKET_IND       (BT_MODULE_AWS_MCE | 0x0005)    /**< A sink or application status update notification is received from an AWS-MCE remote device with the #bt_aws_mce_information_ind_t payload. */
#define BT_AWS_MCE_STATE_CHANGED_IND           (BT_MODULE_AWS_MCE | 0x0006)    /**< An Agent state update notification with the #bt_aws_mce_state_change_ind_t payload. */
#define BT_AWS_MCE_PREPARE_ROLE_HANDOVER_CNF   (BT_MODULE_AWS_MCE | 0x0007)    /**< The result of preparing the role handover by the Agent, with the #bt_aws_mce_prepare_role_handover_cnf_t payload. */
#define BT_AWS_MCE_ROLE_HANDOVER_CNF           (BT_MODULE_AWS_MCE | 0x0008)    /**< The result of a role handover by the Agent, with the #bt_aws_mce_role_handover_cnf_t payload. */
#define BT_AWS_MCE_ROLE_HANDOVER_IND           (BT_MODULE_AWS_MCE | 0x0009)    /**< A role handover notification by the Agent is received with the #bt_aws_mce_role_handover_ind_t payload. */

/**
 * @brief Define for information notification type.
 */
#define BT_AWS_MCE_INFORMATION_A2DP              (0x81)        /**< The type of Client to Sync A2DP Status. */
#define BT_AWS_MCE_INFORMATION_SCO               (0x82)        /**< The type of Client to Sync Call Status. */
#define BT_AWS_MCE_INFORMATION_REPORT_APP        (0x85)        /**< The type of Agent or Partner’s Application Data. */
#define BT_AWS_MCE_INFORMATION_ROLE_HANDOVER     (0x87)        /**< The type of action to ask the Partner to Handover to the Agent role. */
#define BT_AWS_MCE_INFORMATION_PARTNER_SELECTION (0x8C)        /**< The type of select a Partner based on its number. */
#define BT_AWS_MCE_INFORMATION_MIC_SELECTION     (0x8E)        /**< The type of MIC selection for a speaker scenario. */
#define BT_AWS_MCE_INFORMATION_RACE              (0x8F)        /**< The type of RACE command between the Agent and the Client. */
typedef uint8_t bt_aws_mce_information_type_t;                 /**< The type of information notification. */

/**
 * @brief Define for role handover error code.
 */
#define BT_AWS_MCE_RHO_ERROR_BLE_ADVERTISING           (0x80)     /**< AWS_MCE role handover is failure because the BLE advertiser is on goiong. */
#define BT_AWS_MCE_RHO_ERROR_SNIFF_MODE                (0x81)     /**< AWS_MCE role handover is failure because the ACL link is in sniff mode. */
#define BT_AWS_MCE_RHO_ERROR_NO_PARTNER                (0x82)     /**< AWS_MCE role handover is failure because the Partner is lost or not attached. */
#define BT_AWS_MCE_RHO_ERROR_CLOSE_TO_LOST_PARTNER     (0x83)     /**< AWS_MCE role handover is failure because the Partner was communicated with Agent and might be closed to lost. */
#define BT_AWS_MCE_RHO_ERROR_UNSUPPORTED_REMOTE_FEATURE    (0x1a)     /**< AWS_MCE role handover is failure because the Smart Phone dose not support features which will allow role handover. */

/**
 * @}
 */

 /**
 * @defgroup Bluetoothbt_aws_mce_struct Struct
 * @{
 */

/**
 *  @brief This structure defines the result of the #BT_AWS_MCE_CONNECTED event.
 */
typedef struct {
    uint32_t handle;            /**< AWS-MCE handle. */
    bt_bd_addr_t *address;      /**< Bluetooth address of a remote device. */
} bt_aws_mce_connected_t;

/**
 *  @brief This structure defines the result of the #BT_AWS_MCE_CALL_AUDIO_CONNECTED event.
 */
typedef struct {
    uint32_t handle;            /**< AWS-MCE handle. */
    bt_bd_addr_t *address;      /**< Bluetooth address of a remote device. */
    uint8_t sco_type;           /**< SCO type, 2-CVSD, 3-mSBC.*/
} bt_aws_mce_call_audio_connected_t;

/**
 *  @brief This structure defines the #BT_AWS_MCE_DISCONNECTED result.
 */
typedef struct {
    uint32_t handle;            /**< AWS-MCE handle. */
    uint32_t reason;            /**< The reason for the disconnection. */
} bt_aws_mce_disconnected_t;

/**
 *  @brief This structure defines the #BT_AWS_MCE_CALL_AUDIO_DISCONNECTED result.
 */
typedef bt_aws_mce_disconnected_t bt_aws_mce_call_audio_disconnected_t;

/**
 *  @brief This structure defines the necessary AWS-MCE information.
 */
typedef struct {
    bt_aws_mce_information_type_t type;         /**< The information notification type. */
    uint8_t padding;                            /**< Padding byte. */
    uint16_t data_length;                       /**< The packet data length for information sent. */
    uint8_t *data;                              /**< The packet data. */
} bt_aws_mce_information_t;

/**
 *  @brief This structure defines the #BT_AWS_MCE_INFOMATION_PACKET_IND result.
 */
typedef struct {
    uint32_t handle;                    /**< AWS-MCE handle. */
    bt_aws_mce_information_t packet;    /**< The sent information packet. */
} bt_aws_mce_information_ind_t;

/**
 *  @brief This structure defines the #BT_AWS_MCE_STATE_CHANGED_IND result.
 */
typedef struct {
    uint32_t handle;                  /**< AWS-MCE handle. */
    bt_aws_mce_agent_state_type_t state;    /**< AWS-MCE Agent new state. */
} bt_aws_mce_state_change_ind_t;

/**
 *  @brief This structure defines the #BT_AWS_MCE_PREPARE_ROLE_HANDOVER_CNF result.
 */
typedef struct {
    uint32_t handle;                  /**< AWS-MCE handle. */
} bt_aws_mce_prepare_role_handover_cnf_t;

/**
 *  @brief This structure defines the #BT_AWS_MCE_ROLE_HANDOVER_CNF result.
 */
typedef struct {
    uint32_t handle;                  /**< AWS-MCE handle. */
} bt_aws_mce_role_handover_cnf_t;

/**
 *  @brief This structure defines the set of connection handles which are required by the upper layer at the Partner when there is a role handover.
 */
typedef struct {
    uint32_t gap_handle;                   /**< GAP connection handle. */
    uint32_t hfp_handle;                   /**< HFP connection handle. */
    uint32_t a2dp_handle;                  /**< A2DP connection handle. */
    uint32_t avrcp_handle;                 /**< AVRCP connection handle. */
    uint32_t hsp_handle;                   /**< HSP connection handle. */
} bt_aws_mce_role_handover_profile_t;

/**
 *  @brief This structure defines the #BT_AWS_MCE_ROLE_HANDOVER_IND result to notify the upper layer at the Partner.
 */
typedef struct {
    uint32_t handle;                                /**< AWS-MCE handle. */
    bt_aws_mce_role_handover_profile_t profile_info;/**< The connection handles for host profiles. */
    uint16_t length;                                /**< The data length for the role handover. */
    uint8_t *data;                                  /**< The data that must be updated by the uppper layer. */
} bt_aws_mce_role_handover_ind_t;
/**
 * @}
 */
 
BT_EXTERN_C_BEGIN
/**
 * @brief                This funciton initiates the AWS-MCE role when BT successfully powers on.
 * @param[in]    role    is the AWS-MCE role. The role is set as BT_AWS_MCE_ROLE_NONE if the role is not corrected.
 * @return       None
 */
void bt_aws_mce_init_role(bt_aws_mce_role_t role);

/**
 * @brief                This functon sets the AWS-MCE state.
 * @param[in]    handle  is the AWS-MCE handle.
 * @param[in]    state  is the new state to which the Agent wants to change.
 * @return              #BT_STATUS_SUCCESS, the operation completed successfully.
 *                      #BT_STATUS_FAIL, the operation failed.
 *                      #BT_STATUS_OUT_OF_MEMORY, the operation failed because there is not enough memory.
 */
bt_status_t bt_aws_mce_set_state(uint32_t handle, bt_aws_mce_agent_state_type_t state);

/**
 * @brief                This function connects to the specified Bluetooth link. The #BT_AWS_MCE_CONNECTED event is sent to the upper layer with the connected request result.
 * @param[out]   handle  is the connection handle of the specified link.
 * @param[in]    address is the Bluetooth address of a remote device. 
 * @return               #BT_STATUS_SUCCESS, the operation completed successfully.
 *                       #BT_STATUS_FAIL, the operation failed.
 *                       #BT_STATUS_OUT_OF_MEMORY, the operation failed because there is not enough memory.
 */
bt_status_t bt_aws_mce_connect(uint32_t *handle, const bt_bd_addr_t *address);

/**
 * @brief               This function disconnects from the specified Bluetooth link at the Client or Partner. The #BT_AWS_MCE_DISCONNECTED event is sent to the upper layer with the detachment request result.
 * @param[in] handle    is the connection handle for the specified remote device.
 * @return              #BT_STATUS_SUCCESS, the operation completed successfully.
 *                      #BT_STATUS_FAIL, the operation failed.
 *                      #BT_STATUS_OUT_OF_MEMORY, the operation failed because there is not enough memory.
 */
bt_status_t bt_aws_mce_disconnect(uint32_t handle);

/**
 * @brief               This function sends information data from the Agent to Partner or the Partner to Agent. The #BT_AWS_MCE_INFOMATION_PACKET_IND event is received at the remote end.
 *                      Note: The length for the information packet must be less than 200B.
 * @param[in] handle    is the connection handle for the specified link.
 * @param[in] information defines the updated information from the upper layer.
 * @param[in] urgent defines if the information packet should be sent out instantly. To ensure the audio quality, the urgent is set true only for A2DP and Call information.
 * @return              #BT_STATUS_SUCCESS, the operation completed successfully.
 *                      #BT_STATUS_FAIL, the operation failed.
 *                      #BT_STATUS_OUT_OF_MEMORY, the operation failed because there is not enough memory.
 */
bt_status_t bt_aws_mce_send_information(uint32_t handle, const bt_aws_mce_information_t *information, bool urgent);

/**
 * @brief               This function prepares the role handover by the Agent. The #BT_AWS_MCE_PREPARE_ROLE_HANDOVER_CNF event is received at the Agent.
 *                       Note: This procedure will be failure under the conditions below:
 *                             1). SPP connection or BLE connection is existed.
 *                             2). The conneciton is in sniff mode.
 *                             3). BLE avdertising is enabled.
 *                             4). Partner is lost, not attached or closed to lost.
 *                             5). Phone is Slave role.
 *                             6). Phone dose not support power control feature.
 * @param[in] handle    is the connection handle for the specified link.
 * @param[in] remote_address is the Bluetooth address of a remote device. 
 * @return              #BT_STATUS_SUCCESS, the operation completed successfully.
 *                      #BT_STATUS_BUSY, the operatio is failed because BT is busy on other operation.
 *                      #BT_STATUS_FAIL, the operation failed.
 *                      #BT_STATUS_OUT_OF_MEMORY, the operation failed because there is not enough memory.
 *                      #BT_AWS_MCE_RHO_ERROR_BLE_ADVERTISING, the BLE advertiser is on goiong.
 *                      #BT_AWS_MCE_RHO_ERROR_SNIFF_MODE, the ACL link is in sniff mode.
 *                      #BT_AWS_MCE_RHO_ERROR_NO_PARTNER, the Partner is lost or not attached.
 *                      #BT_AWS_MCE_RHO_ERROR_CLOSE_TO_LOST_PARTNER, the Partner was communicated with Agent and might be closed to lost.
 */
bt_status_t bt_aws_mce_prepare_role_handover(uint32_t handle, const bt_bd_addr_t *remote_address);

/**
 * @brief               This function performs the role handover by the Agent. The #BT_AWS_MCE_ROLE_HANDOVER_CNF event id received at the Agent.
 *                      And the #BT_AWS_MCE_ROLE_HANDOVER_IND event is received at the Partner if the result of the role handover is successful. 
 *                      Note: 1. This procedure should be performed after the procedure #bt_aws_mce_prepare_role_handover is successful.
 *                            2. The data length must be less than 100B.
 * @param[in] handle    is the connection handle for the specified link.
 * @param[in] data      is the application data which is requested to initiate the remote end. 
 * @param[in] length    is the data length. 
 * @return              #BT_STATUS_SUCCESS, the operation completed successfully.
 *                      #BT_STATUS_BUSY, the operatio is failed because BT is busy on other operation.
 *                      #BT_STATUS_FAIL, the operation failed.
 *                      #BT_STATUS_OUT_OF_MEMORY, the operation failed because there is not enough memory.
 *                      #BT_AWS_MCE_RHO_ERROR_UNSUPPORTED_REMOTE_FEATURE, the BLE advertiser is on goiong.
 */
bt_status_t bt_aws_mce_role_handover(uint32_t handle, const uint8_t *data, uint16_t length);

/**
 * @brief     This function gets the address of a connected device.
 * @param[in] handle  is the AWS_MCE handle.
 * @return            A pointer to the address, if the operation completed successfully.
 *                    NULL, if it failed.
 */
const bt_bd_addr_t* bt_aws_mce_get_bd_addr_by_handle(uint32_t handle);
BT_EXTERN_C_END
/**
 * @}
 * @}
 */
#endif /*__BT_AWS_MCE_H__*/

