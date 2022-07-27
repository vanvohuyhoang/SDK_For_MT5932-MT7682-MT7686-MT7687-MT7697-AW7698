/* Copyright Statement:
 *
 * (C) 2005-2016  MediaTek Inc. All rights reserved.
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
 * the License Agreement ("Permitted User").  If you are not a Permitted User,
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

#ifndef __BT_AVM_H__
#define __BT_AVM_H__

#include "bt_platform.h"
#include "bt_system.h"
#include "bt_type.h"

BT_EXTERN_C_BEGIN

#define BT_AVM_TYPE_CALL (0x00)
#define BT_AVM_TYPE_A2DP (0x01)

typedef uint8_t bt_avm_audio_type_t;  /**< Define the bt audio type. */

/**
 * @brief The event report to user
 */
#define BT_AVM_DECODE_NOTIFICATION_IND    (BT_MODULE_AVM | 0x0000)    /**< A  
 * notification that DSP should start decode media data, with #bt_media_decode_notification_ind_t as the payload in the callback function. */
#define BT_AVM_CALL_ANCHOR_POINT_IND      (BT_MODULE_AVM | 0x0001)    /**< A  
         * notification that the anchor point for sco or esco data, with #bt_avm_call_anchor_point_ind_t as the payload in the callback function. */

/**
 * @brief    Bluetooth share buffer set to controller module, this structure is 
 * available only for AB155x and later version.
 */
typedef struct {
    uint32_t a2dp_address;      /**< A2DP media packet buffering pointer.*/
    uint32_t sco_up_address;    /**< eSCO/SCO up-link packet buffering pointer.*/
    uint32_t sco_dl_address;    /**< eSCO/SCO down-link packet buffering pointer.*/
    uint32_t clock_mapping_address; /**< Audio clock and BT clock mapping buffer pointer. */
} bt_avm_share_buffer_info_t;

/**
 * @brief This structure defines the data type of the parameter in the callback for #BT_AVM_DECODE_NOTIFICATION_IND event..
 * @{
 */
typedef struct {
    uint32_t timestamp; /**< The timestamp for audio packet.*/
    bt_clock_t clock;   /**< The bt clock when audio packet is received.*/
    uint16_t sequence_num; /** < The sequence number for related audio pakcet. */
} bt_avm_a2dp_packet_info_t;

/**
 * @brief This structure defines the data type of the parameter in the callback for #BT_AVM_DECODE_NOTIFICATION_IND event..
 * @{
 */
typedef struct {
    uint32_t handle;            /**< bt gap connection handle.*/
    bt_avm_a2dp_packet_info_t packet_info[2];   /**< The timestamp for first and second packet.*/
} bt_avm_decode_notify_ind_t;

/**
 * @brief This structure defines the anchor point_MEDIA_DECODE_NOTIFICATION_IND event..
 * @{
 */
typedef struct {
    uint32_t handle;            /**< bt gap connection handle.*/
    bt_clock_t anchor_clock;   /**< The timestamp for first and second packet.*/
} bt_avm_call_anchor_point_ind_t;

/**
 * @}
 */

/**
 * @brief     This function sets buffer to controller after #BT_POWER_ON_CNF, 
 *            which is accessed by host and controller used to update clock offset for each acl link.
 *            Note, this function is available on AB155x and later version.
 * @param[in] buffer is a buffer pointer for host and controller access.
 * @param[in] link_num is acl link num.
 * @return    #BT_STATUS_SUCCESS,  sets clock offset share buffer successfully.
 *            #BT_STATUS_FAIL, sets clock offset share buffer failed.
 */
bt_status_t bt_avm_set_clock_offset_share_buffer(uint8_t *buffer, uint8_t link_num);

/**
 * @brief     This function sets buffer to controller after #BT_POWER_ON_CNF, which is used by controler and DSP.
 *            Note, this function is available on AB155x and later version.
 * @param[in] buffer_info is a buffer set, including A2DP buffer, SCO/eSCO uplink buffer and down-link buffer.
 * @return    #BT_STATUS_SUCCESS,  sets share buffer successfully.
 *            #BT_STATUS_FAIL, sets share buffer failed.
 */
bt_status_t bt_avm_set_share_buffer(bt_avm_share_buffer_info_t *buffer_info);

/**
* @brief               This function sets the condition to controller where application will be notified if media packet condition is met.
* @param[in] condition is media packet node which needs be released after it is consumed.
* @return              #BT_STATUS_SUCCESS,  sets a2dp notifiy condition successfully.
*                      #BT_STATUS_FAIL, sets a2dp notifiy condition failed.
*/
bt_status_t bt_avm_set_a2dp_notify_condition(uint32_t gap_handle, uint32_t condition);

/**
* @brief               This function sets the bluethooth clock to controller when the audio sys should be opened.
* @param[in] gap_handle is the gap conneciton handle for current link.
* @param[in] type       is the media type.
* @param[in] bt_clock   is the bluetooth clock when the audio sys should be opened.
* @return              #BT_STATUS_SUCCESS,  sets audio tracking time successfully.
*                      #BT_STATUS_FAIL, sets audio tracking time failed.
*/
bt_status_t bt_avm_set_audio_tracking_time(uint32_t gap_handle, bt_avm_audio_type_t type, bt_clock_t *bt_clock);

/**
* @brief               This function updates the slope of a linear regression with which 
*                      the drifts between remote clock and local clock occurs in a special duration.
* @param[in] gap_handle is the gap conneciton handle for current link.
* @param[in] audio_instant the audio instant counter.
* @param[in] bt_clock   is the bluetooth clock when the audio sys should be opened.
* @param[in] drift      is the slope of a linear regression united in microsecond per sencod.
* @return              #BT_STATUS_SUCCESS,  updates the slope successfully.
*                      #BT_STATUS_FAIL, updates the slope failed.
*/
bt_status_t bt_avm_update_clock_drift(uint32_t gap_handle, uint32_t audio_instant, bt_clock_t *bt_clock, uint16_t drift);

/**
* @brief               This function get the pointor to the clock offset for 
specified link.
* @param[in] gap_handle is the gap conneciton handle for current link.
* @return              the pointer to corresponding buffer.
*                      NULL if the ACL link with the gap_handle is not existed.
*/
const void * bt_avm_get_clock_offset_address(uint32_t gap_handle);

BT_EXTERN_C_END

#endif


