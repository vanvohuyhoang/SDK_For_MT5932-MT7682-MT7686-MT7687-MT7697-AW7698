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

#include "hal_feature_config.h"
#ifdef HAL_CHARGER_MODULE_ENABLED
#if (PRODUCT_VERSION == 2523 || PRODUCT_VERSION == 2533 )
#include "FreeRTOS.h"
#include "task.h"
#include <string.h>
#include <stdio.h>
#include "at_command.h"
#include "syslog.h"

#include "hal_charger.h"

#ifdef MTK_BUILD_SMT_LOAD
#include "cust_charging.h"
#endif

log_create_module(atci_charger, PRINT_LEVEL_INFO);

/* ===================================================*/
/* define */
/* ===================================================*/

/* ===================================================*/
/* function */
/* ===================================================*/

/* ===================================================*/
/* global variable */
/* ===================================================*/

/* ===================================================*/
// AT command handler
atci_status_t atci_cmd_hdlr_charger(atci_parse_cmd_param_t *parse_cmd)
{
    atci_response_t response = { {0}};
    bool cable_in = false;
    int32_t batt_volt = 0;
    int32_t batt_temp = 0;
    int32_t char_curr = 0;
    int32_t char_volt = 0;

    LOG_MSGID_I(atci_charger, "[atci_charger]start\r\n", 0);

#ifndef MTK_BUILD_SMT_LOAD
    hal_charger_init();
    hal_charger_get_charger_detect_status(&cable_in);
    hal_charger_meter_get_charging_current(&char_curr);
#else
    hal_charger_init();
    hal_charger_get_charger_detect_status(&cable_in);
    if (cable_in) {
        /* Default pre-charging current. It depends on the battery specifications.
         The USBDL mode current level  = 16.8mV / R_SESNE */
        hal_charger_set_charging_current(16800 / CUST_R_SENSE);

        hal_charger_enable(true);
        LOG_MSGID_I(atci_charger, "hal_charger_enable delay start\r\n", 0);

        vTaskDelay(1000 / portTICK_RATE_MS);

        hal_charger_meter_get_charging_current(&char_curr);
        LOG_MSGID_I(atci_charger, "hal_charger_enable delay end\r\n", 0);
        hal_charger_enable(false);
    }
#endif

    hal_charger_meter_get_battery_voltage_sense(&batt_volt);
    hal_charger_meter_get_battery_temperature(&batt_temp);
    hal_charger_meter_get_charger_voltage(&char_volt);

    response.response_flag = 0;

    switch (parse_cmd->mode) {
        case ATCI_CMD_MODE_TESTING:
        /* AT+ECHAR=? */
        strcpy((char *)response.response_buf, "+ECHAR=(0,1)\r\nOK\r\n");
        response.response_len = strlen((char *)response.response_buf);
        atci_send_response(&response);
        break;

        case ATCI_CMD_MODE_READ:
        /* AT+ECHAR? */
        snprintf((char *)response.response_buf, ATCI_UART_TX_FIFO_BUFFER_SIZE, "+ECHAR: %d\r\n", (int)cable_in);
        response.response_len = strlen((char *)response.response_buf);
        response.response_flag |= ATCI_RESPONSE_FLAG_APPEND_OK;
        atci_send_response(&response);
        break;

        case ATCI_CMD_MODE_ACTIVE:
        /* AT+ECHAR */
        snprintf((char *)response.response_buf, ATCI_UART_TX_FIFO_BUFFER_SIZE, "+ECHAR:%d,%d,%d,%d\r\n", (int)batt_volt, (int)batt_temp, (int)char_curr, (int)char_volt);
        response.response_len = strlen((char *)response.response_buf);
        response.response_flag |= ATCI_RESPONSE_FLAG_APPEND_OK;
        atci_send_response(&response);
        break;

        case ATCI_CMD_MODE_EXECUTION:
        /* AT+ECHAR=... */

        default :
        // others are invalid command format
        LOG_MSGID_I(atci_charger, "[atci_charger] mode=%d\r\n",1, parse_cmd->mode);
        strcpy((char *)response.response_buf, "ERROR\r\n");
        response.response_len = strlen((char *)response.response_buf);
        atci_send_response(&response);
        break;
    }

    return ATCI_STATUS_OK;
}
#elif (PRODUCT_VERSION == 1552)
#include "FreeRTOS.h"
#include "task.h"
#include <string.h>
#include <ctype.h>
#include <stdio.h>
#include "at_command.h"
#include "syslog.h"
#include "battery_management_core.h"

#if (IC_CONFIG == ab155x)
#include "hal_charger.h"
#endif
log_create_module(atci_charger, PRINT_LEVEL_INFO);
#define MAX_CHAR_NUM 6
int task_timer=1000;
int count = 0;
static TaskHandle_t charger_task_handle = NULL;
#include "battery_management.h"

int ctoi(char s[]) {
    int i=0;
    int n = 0;
    for (; (s[i] >= '0' && s[i] <= '9'); ++i) {
            n = 10 * n + (s[i]- '0');
    }
    return n;
}
int htoi(char s[]) {
    int i;
    int n = 0;
    if (s[0] == '0' && (s[1] == 'x' || s[1] == 'X')) {
        i = 2;
    } else {
        i = 0;
    }
    for (; (s[i] >= '0' && s[i] <= '9') || (s[i] >= 'a' && s[i] <= 'z') || (s[i] >= 'A' && s[i] <= 'Z'); ++i) {
        if (tolower((int)s[i]) > '9') {
            n = 16 * n + (10 + tolower((int )s[i]) - 'a');
        } else {
            n = 16 * n + (tolower((int)s[i]) - '0');
        }
    }
    return n;
}

static void charger_task( void * pvParameters )
{
    const char *charger_type[10] = { "", "SDP", "CDP", "DCP", "SS", "IPAD2_IPAD4", "IPHONE_5V_1A", "NON-STD", "DP/DM_Floating", "UNABLE_TO_IDENTIFY"};
    const char *charger_state[8] = { "CHARGER OFF", "PRECC", "CC", "EXTENSION", "EOC", "THERMAL", "VBAT_OVP", "SAFETY TIMER"};

    for(;; )
    {
        int32_t charger_status;
        const TickType_t xDelay = task_timer / portTICK_PERIOD_MS;
        vTaskDelay( xDelay );
        charger_status =battery_management_get_battery_property(BATTERY_PROPERTY_CHARGER_EXIST);
        if (charger_status) {
            LOG_MSGID_I(common,"[BM]Battery capacity = %d(%%) ",1, (int)battery_management_get_battery_property(BATTERY_PROPERTY_CAPACITY));
            LOG_MSGID_I(common,"[BM]Charger status = %d ",1, (int)charger_status);
            LOG_I(common,"[BM]Charger type = %s",charger_type[battery_management_get_battery_property(BATTERY_PROPERTY_CHARGER_TYPE)]);
            LOG_MSGID_I(common,"[BM]Battery temperature = %d(Celsius degrees)" ,1, (int)battery_management_get_battery_property(BATTERY_PROPERTY_TEMPERATURE));
            LOG_MSGID_I(common,"[BM]Battery voltage = %d(mV)\n",1,(int) battery_management_get_battery_property(BATTERY_PROPERTY_VOLTAGE));
            LOG_I(common,"[BM]Charger State = %s\n", charger_state[battery_management_get_battery_property(BATTERY_PROPERTY_CHARGER_STATE)]);
            LOG_MSGID_I(common,"[BM]Multi level = %d\n",1, (int)(1+pmu_get_register_value_2byte_mt6388(PMU_RG_ICC_MULTIPLE_ITH_ADDR,PMU_RG_ICC_MULTIPLE_ITH_MASK, PMU_RG_ICC_MULTIPLE_ITH_SHIFT)));
        } else {
            LOG_MSGID_I(common,"[BM]Battery capacity = %d(%%)",1,(int) battery_management_get_battery_property(BATTERY_PROPERTY_CAPACITY));
            LOG_MSGID_I(common,"[BM]Charger status = %d(0:charge not exist; 1:charger exist)",1,(int) charger_status);
            LOG_MSGID_I(common,"[BM]Battery temperature = %d(Celsius degrees)",1,(int) battery_management_get_battery_property(BATTERY_PROPERTY_TEMPERATURE));
            LOG_MSGID_I(common,"[BM]Battery voltage = %d(mV)\n",1, (int)battery_management_get_battery_property(BATTERY_PROPERTY_VOLTAGE));
        }
    }
}

atci_status_t atci_cmd_hdlr_charger(atci_parse_cmd_param_t *parse_cmd)
{
    atci_response_t response = { {0}};
    switch (parse_cmd->mode) {
        case ATCI_CMD_MODE_EXECUTION: /* AT+ECHAR=SQC... */
        if(strncmp(parse_cmd->string_ptr, "AT+ECHAR=SQC,", 12) == 0) {
            char *mid_pos = NULL;
            char str[20] = {0};
            int input_domain = 0;
            int input_value = 0;
            char *end_pos = NULL;
            mid_pos = strchr(parse_cmd->string_ptr, ',');
            mid_pos++;
            end_pos = strchr(mid_pos, ',');
            memcpy(str, mid_pos, strlen(mid_pos) - strlen(end_pos));
            input_domain = htoi(mid_pos);
            end_pos++;
            input_value = htoi(end_pos);
            task_timer = ctoi(end_pos);
            mid_pos = NULL;
            end_pos = NULL;
            LOG_MSGID_I(common,"input_value1:%d, input_value2:%d\r\n",2, input_domain, input_value);
            switch (input_domain) {
                case 0:
                vTaskDelete(charger_task_handle);
                charger_task_handle = NULL;
                count = 0;
                break;

                case 1 :
                if (charger_task_handle == NULL) {
                    xTaskCreate(charger_task, "charger_task", 512, (void*)&input_value, tskIDLE_PRIORITY, &charger_task_handle);
                }
                break;

                case 2:
                vTaskSuspend(charger_task_handle);
                break;

                case 3:
                vTaskResume(charger_task_handle);
                break;
            }

        }else if(strncmp(parse_cmd->string_ptr, "AT+ECHAR=FASTCC,", 12) == 0) {
            int input_addr = 0;
            char *end_pos = NULL;
            end_pos = strchr(parse_cmd->string_ptr, ',');
            end_pos++;
            input_addr = htoi(end_pos);
            end_pos = NULL;
            LOG_MSGID_I(common,"Input leve : %d\r\n",1 ,input_addr);
            if(input_addr <=0){
                input_addr =1;
            }
            battery_multiple_level_setting((input_addr-1));

        } else if(strncmp(parse_cmd->string_ptr, "AT+ECHAR=MSG,", 12) == 0) {
            char *mid_pos = NULL;
            char str[20] = {0};
            int input_domain = 0;
            int input_value = 0;
            char *end_pos = NULL;
            mid_pos = strchr(parse_cmd->string_ptr, ',');
            mid_pos++;
            end_pos = strchr(mid_pos, ',');
            memcpy(str, mid_pos, strlen(mid_pos) - strlen(end_pos));
            input_domain = htoi(mid_pos);
            end_pos++;
            input_value = htoi(end_pos);
            mid_pos = NULL;
            end_pos = NULL;
            LOG_MSGID_I(common,"input_value1:%d, input_value2:%d\r\n",2, input_domain, input_value);
        } else if(strncmp(parse_cmd->string_ptr, "AT+ECHAR=CHG_OPTION,", 12) == 0) {
            int input_addr = 0;
            char *end_pos = NULL;
            end_pos = strchr(parse_cmd->string_ptr, ',');
            end_pos++;
            input_addr = htoi(end_pos);
            end_pos = NULL;
            LOG_MSGID_I(common,"Input leve : %d\r\n", 1, input_addr);
            battery_switch_charger_option(input_addr);
        }else {
            response.response_flag = 0 | ATCI_RESPONSE_FLAG_APPEND_ERROR;
        }
        response.response_flag = 0 | ATCI_RESPONSE_FLAG_APPEND_OK;
        atci_send_response(&response);
        break;
        default :
        // others are invalid command format
        LOG_MSGID_I(common, "[atci_charger] mode=%d\r\n",1, parse_cmd->mode);
        strcpy((char *)response.response_buf, "ERROR\r\n");
        response.response_len = strlen((char *)response.response_buf);
        atci_send_response(&response);
        break;
    }

    return ATCI_STATUS_OK;
}

#endif

#endif /* HAL_CHARGER_MODULE_ENABLED */

