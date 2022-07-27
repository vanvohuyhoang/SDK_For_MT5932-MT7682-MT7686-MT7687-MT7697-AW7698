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

#if !defined(MTK_SWITCH_AT_COMMAND_DISABLE) && defined(MTK_SWITCH_TO_RACE_COMMAND_ENABLE)

#include "FreeRTOS.h"
#include "task.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include "at_command.h"
#ifdef MTK_RACE_CMD_ENABLE
#include "race_xport.h"
#endif

#include "syslog.h"
#include "atci.h"

 /* Private macro -------------------------------------------------------------*/
#define AT_COMMAND_SWITCH_HELP \
"AT+SWITCH - swich to other command mode.\r\n\
usage:\r\n\
   switch to race command mode:   AT+SWITCH=1,race_command\r\n\
   reboot action can switch mode to at command mode again\r\n"

#define AT_COMMAND_SWITCH_INFO \
   "AT switch to RACE command mode!\r\n"
#define AT_COMMAND_NO_SWITCH_INFO \
   "RACE UART un-supported, keep AT command mode!\r\n"


#define LOGE(fmt,arg...)   LOG_E(atcmd, "[switch]"fmt,##arg)
#define LOGW(fmt,arg...)   LOG_W(atcmd, "[switch]"fmt,##arg)
#define LOGI(fmt,arg...)   LOG_I(atcmd ,"[switch]"fmt,##arg)

#define LOGMSGIDE(fmt,cnt,arg...)   LOG_MSGID_E(atcmd ,"[switch]"fmt,cnt,##arg)
#define LOGMSGIDW(fmt,cnt,arg...)   LOG_MSGID_W(atcmd ,"[switch]"fmt,cnt,##arg)
#define LOGMSGIDI(fmt,cnt,arg...)   LOG_MSGID_I(atcmd ,"[switch]"fmt,cnt,##arg)

extern atci_status_t atci_deinit(hal_uart_port_t port);

/*--- Function ---*/
atci_status_t atci_cmd_hdlr_switch(atci_parse_cmd_param_t *parse_cmd);

// AT command handler
atci_status_t atci_cmd_hdlr_switch(atci_parse_cmd_param_t *parse_cmd)
{
    atci_response_t response;
    char *param = NULL;
    bool isSwitchRace = true;
    bool isSwitchRace_no_atci = false;
    char *race = "RACE_COMMAND";
    int index = 9;    //char '=' position

    LOGMSGIDW("atci_cmd_hdlr_switch\r\n", 0);
    memset(&response, 0, sizeof(response));
#ifdef ATCI_APB_PROXY_ADAPTER_ENABLE
    response.cmd_id = parse_cmd->cmd_id;
#endif

    switch (parse_cmd->mode) {
    case ATCI_CMD_MODE_TESTING:
        /* AT+SWITCH=? */
        strcpy((char *)(response.response_buf), AT_COMMAND_SWITCH_HELP);
        response.response_flag |= ATCI_RESPONSE_FLAG_APPEND_OK;
        response.response_len = strlen((char *)response.response_buf);
        atci_send_response(&response);
        break;

    case ATCI_CMD_MODE_EXECUTION: // rec: AT+SWITCH=<p1>  the handler need to parse the parameters
        LOGMSGIDI("AT Executing...\r\n", 0);
        param = parse_cmd->string_ptr;
        //parsing the parameter
        param = param + index + 1;
        param = strtok(param, ",\n\r");

        LOGI("AT Executing...param=%s\r\n", param);

        if (param != NULL && strlen(param) > 0)
        {
            if (param[0] == '1' || param[0] == '2')
            {
                //if(param[0] == '2')
                {
                    isSwitchRace_no_atci = true;
                }
                param = param + 2; //first letter position in at command parameter

                int param_len = strlen(param);
                int cmp_len = strlen(race);
                if (param_len < cmp_len)
                {
                    isSwitchRace = false;
                    LOGMSGIDW("param len is short,no need to switch", 0);
                }
                else
                {
                    /* change lowercase letters to uppercase letters */

                    int i = 0;
                    for (; i < cmp_len; i++) {
                        if (param[i] >= 'a' && param[i] <= 'z') {
                            param[i] -= 32;
                        }
                        LOGMSGIDI("end::param is: %c\r\n",1, param[i]);
                        if (param[i] != race[i]) {
                            isSwitchRace = false;
                            LOGMSGIDI("no need to switch", 0);
                            break;
                        }
                    }
                }
            }
        }
        if (isSwitchRace) {
#if (MTK_RACE_CMD_SERIAL_PORT_ENABLE)
            strcpy((char *)(response.response_buf), AT_COMMAND_SWITCH_INFO);
            response.response_flag |= ATCI_RESPONSE_FLAG_APPEND_OK;
            response.response_len = strlen((char *)response.response_buf);
            atci_send_response(&response);
            if(isSwitchRace_no_atci)
            {
                atci_status_t sta = atci_deinit(g_atci_uart_port);
                LOGMSGIDW("AT Switch call atci_deinit. result=%d\r\n",1, sta);
            }

            //TODO: call race command init
            if (g_atci_uart_port == g_atci_uart_port_default)
            {
                race_status_t stu = race_deinit();
                if (stu != RACE_STATUS_OK)
                {
                    LOGMSGIDI("race deinit fail...!\r\n", 0);
                    return RACE_STATUS_ERROR;
                }

                race_status_t ret = race_serial_port_uart_init(g_atci_uart_port);
                if (ret != RACE_STATUS_OK)
                {
                    LOGMSGIDI("race uart init fail...! port:%d\r\n",1, g_atci_uart_port);
                    return RACE_STATUS_ERROR;
                }
            }
            else
            {
                #if defined(MTK_ATCI_VIA_PORT_SERVICE) && defined(MTK_PORT_SERVICE_ENABLE)
                serial_port_dev_t port;
                serial_port_setting_uart_t uart_setting;

                if(serial_port_config_read_dev_number("atci", &port) != SERIAL_PORT_STATUS_OK)
                {
                    port = SERIAL_PORT_DEV_UART_2; //SERIAL_PORT_DEV_USB_COM1;//SERIAL_PORT_DEV_UART_1;
                    serial_port_config_write_dev_number("atci", port);
                    LOG_MSGID_W(common, "serial_port_config_write_dev_number setting uart1", 0);
                    uart_setting.baudrate = HAL_UART_BAUDRATE_921600;
                    serial_port_config_write_dev_setting(port, (serial_port_dev_setting_t *)&uart_setting);
                }
                atci_init(port);
                #else
                atci_init(HAL_UART_2);
                #endif

            #if defined(MTK_RACE_CMD_ENABLE) && defined(MTK_PORT_SERVICE_ENABLE)
                serial_port_dev_t race_port;
                serial_port_setting_uart_t race_uart_setting;

                if(serial_port_config_read_dev_number("race", &race_port) != SERIAL_PORT_STATUS_OK)
                {
                    race_port = SERIAL_PORT_DEV_UART_1;
                    serial_port_config_write_dev_number("race", race_port);
                    LOG_MSGID_W(common, "serial_port_config_write_dev_number setting uart1", 0);
                    race_uart_setting.baudrate = HAL_UART_BAUDRATE_921600;
                    serial_port_config_write_dev_setting(race_port, (serial_port_dev_setting_t *)&race_uart_setting);
                }
                race_serial_port_uart_init(race_port);
            #else
                race_serial_port_uart_init(HAL_UART_1);
            #endif
            }

            /*if (!race_is_task_existed())
            {
                race_status_t ret = race_init();
                if (ret != RACE_STATUS_OK)
                {
                    LOGMSGIDI("race init fail...! port:%d",1, SERIAL_PORT_DEV_UART_2);
                    return ATCI_STATUS_ERROR;
                }
                else
                {
                    race_create_task();
                }
            }*/
#else
            strcpy((char *)(response.response_buf), AT_COMMAND_NO_SWITCH_INFO);
            response.response_flag |= ATCI_RESPONSE_FLAG_APPEND_OK;
            response.response_len = strlen((char *)response.response_buf);
            atci_send_response(&response);
#endif
        }
        else {
            /* others are invalid command format */
            response.response_flag |= ATCI_RESPONSE_FLAG_APPEND_ERROR;
            response.response_len = strlen((char *)response.response_buf);
            atci_send_response(&response);
        }
        break;

    default:
        /* others are invalid command format */
        response.response_flag |= ATCI_RESPONSE_FLAG_APPEND_ERROR;
        response.response_len = strlen((char *)response.response_buf);
        atci_send_response(&response);
        break;
    }

    return ATCI_STATUS_OK;
}

void atci_race_task()
{
  while(1){
    atci_processing();
#if defined(MTK_RACE_CMD_ENABLE)
    race_processing();
#endif
  }
}
#endif /* !MTK_SWITCH_AT_COMMAND_DISABLE && MTK_SWITCH_TO_RACE_COMMAND_ENABLE */

