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

#include "bwcs_wifi_interface.h"
#include "wifi_os_api.h"
#include "wifi_inband.h"
#include "bwcs_wifi.h"

extern bwcs_status_t g_bwcs_status;
extern QueueHandle_t g_bwcs_queue;

int32_t bwcs_wifi_event_notify(bwcs_queue_t wifi_event)
{
    if(g_bwcs_queue == NULL) {
        printf("bwcs queue invalid.\r\n");
        return -1;
    }
    printf("wifi event,%d.\r\n",wifi_event.event);

    if( wifi_os_queue_send(g_bwcs_queue, &wifi_event, 0) != 0 ) {
        return -1;
    }
    
    return 0;
}

int32_t bwcs_config_wifi_cmd(bwcs_wifi_cmd_t wifi_cmd)
{
    wifi_pta_config_t pta_config;
    wifi_pta_wire_t pta_wire;
    wifi_pta_cm_mode_t pta_cm_mode;
    wifi_pta_ant_mode_t pta_ant_mode;

    
    printf("wifi cmd,%d.\r\n",wifi_cmd);
    if(wifi_cmd == BWCS_WIFI_CMD_OFF) {
        pta_config = WIFI_PTA_DISABLE;
    }else if(wifi_cmd == BWCS_WIFI_CMD_ON) {
        pta_config = WIFI_PTA_ENABLE;
    }else {
        printf("Not support PTA config:%d.\r\n",wifi_cmd);
        return -1;
    }
    
    if(g_bwcs_status.pta_mode == PTA_MODE_TDD_1_WIRE) {
        pta_wire = WIFI_PTA_1_WIRE_MODE;
    }else if(g_bwcs_status.pta_mode == PTA_MODE_TDD_1_WIRE_EXTENDED) {
        pta_wire = WIFI_PTA_1_WIRE_EXTEND_MODE;
    }else if(g_bwcs_status.pta_mode == PTA_MODE_TDD_3_WIRE) {
        pta_wire = WIFI_PTA_3_WIRE_MODE;
    }else {
        printf("Not support PTA mode:%d.\r\n",g_bwcs_status.pta_mode);
        return -1;
    }

    if(g_bwcs_status.pta_cm_mode == PTA_CM_MODE_TDD) {
        pta_cm_mode = WIFI_PTA_SCM_MODE;
    }else if(g_bwcs_status.pta_cm_mode == PTA_CM_MODE_FDD) {
        pta_cm_mode = WIFI_PTA_UCM_MODE;
    }else {
        printf("Not support CM mode:%d.\r\n",g_bwcs_status.pta_cm_mode);
        return -1;
    }
    
    if(g_bwcs_status.antenna_mode == PTA_ANT_MODE_SINGLE) {
        pta_ant_mode = ANTENNA_MODE_SINGLE;
    }else if(g_bwcs_status.antenna_mode == PTA_ANT_MODE_DUAL){
        pta_ant_mode = ANTENNA_MODE_DUAL;
    }else {
        printf("Not support ANT mode:%d.\r\n",g_bwcs_status.antenna_mode);
        return -1;
    }
    
    if(wifi_cmd == BWCS_WIFI_CMD_ON) {
        wifi_inband_bwcs_config(pta_config,WIFI_PTA_PROFILE_NOCONN,WIFI_PS_PROFILE_DEFAULT,pta_wire,pta_cm_mode,pta_ant_mode);
    }
    return 0;
}

int32_t bwcs_config_ant_switch(pta_antenna_mode_t ant_mode)
{
    return 0;
}

