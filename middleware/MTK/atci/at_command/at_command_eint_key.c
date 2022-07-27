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

// For Register AT command handler
// System head file

#ifdef MTK_EINT_KEY_ENABLE

#include "FreeRTOS.h"
#include "task.h"
#include <string.h>
#include <stdio.h>
#include "at_command.h"
#include "hal_gpt.h"
#include "syslog.h"
#include <stdlib.h>

#include "eint_key_custom.h"
#include "eint_key.h"



/*--- Function ---*/
atci_status_t atci_cmd_hdlr_eint_key(atci_parse_cmd_param_t *parse_cmd);


/* AT command handler  */

void atci_eint_key_send_response(int32_t *len1, atci_response_t *resonse)
{
    resonse->response_len = (uint16_t)(* len1);
    resonse->response_flag = ATCI_RESPONSE_FLAG_URC_FORMAT;
    atci_send_response(resonse);
}


void atci_eint_key_handler(bsp_eint_key_event_t event, uint8_t key_data, void *user_data)
{
    int32_t ret_len1;
    atci_response_t resonse = {{0}};

    ret_len1 =  snprintf((char *)(resonse.response_buf), ATCI_UART_TX_FIFO_BUFFER_SIZE, \
                         "event=%d, key_data=%d\r\n", event, key_data);
    atci_eint_key_send_response(&ret_len1, &resonse);
}

atci_status_t atci_cmd_hdlr_eint_key(atci_parse_cmd_param_t *parse_cmd)
{
    atci_response_t resonse = {{0}};

    char *param1 = NULL;
    char *param2 = NULL;

    int32_t ret_len1;
    uint32_t config_data[10];

    bsp_eint_key_config_t config;

    resonse.response_flag = 0; /*    Command Execute Finish.  */
#ifdef ATCI_APB_PROXY_ADAPTER_ENABLE
    resonse.cmd_id = parse_cmd->cmd_id;
#endif

    /*
        sprintf((char *)resonse.response_buf, "mode=%d\r\n",parse_cmd->mode);
        resonse.response_len = strlen((char *)resonse.response_buf);
        resonse.response_flag |= ATCI_RESPONSE_FLAG_URC_FORMAT;
        atci_send_response(&resonse);
        */

    switch (parse_cmd->mode) {
        case ATCI_CMD_MODE_TESTING:

            strncpy((char *)resonse.response_buf, "ATCI_CMD_MODE_TESTING", strlen("ATCI_CMD_MODE_TESTING"));
            resonse.response_len = strlen((char *)resonse.response_buf);
            resonse.response_flag = ATCI_RESPONSE_FLAG_APPEND_OK;
            atci_send_response(&resonse);

            ret_len1 =  snprintf((char *)(resonse.response_buf), ATCI_UART_TX_FIFO_BUFFER_SIZE, \
                                 "Eint key at command test!\r\n");
            atci_eint_key_send_response(&ret_len1, &resonse);

            break;

        case ATCI_CMD_MODE_EXECUTION:
            strncpy((char *)resonse.response_buf, "ATCI_CMD_MODE_EXECUTION", strlen("ATCI_CMD_MODE_EXECUTION"));
            resonse.response_len = strlen((char *)resonse.response_buf);
            resonse.response_flag = ATCI_RESPONSE_FLAG_APPEND_OK;
            atci_send_response(&resonse);

            param1 = strtok(parse_cmd->string_ptr, ":");

            if (strncmp(param1, "AT+EKEY=INIT", strlen("AT+EKEY=INIT")) == 0) {
                param2 = strtok(NULL, ",");

                config_data[0] = (uint32_t)atoi(param2);

                if (config_data[0] == 1) {
                    config.longpress_time = 2000;
                    config.repeat_time = 800;

                    eint_key_custom_init();
                    bsp_eint_key_init(&config);
                    bsp_eint_key_register_callback(atci_eint_key_handler, NULL);
                    bsp_eint_key_enable();

                    snprintf((char *)resonse.response_buf, ATCI_UART_TX_FIFO_BUFFER_SIZE, \
                             "Eint key init done!longpress=%d,repeat=%d\r\n", (int)config.longpress_time, (int)config.repeat_time);
                } else {
                    snprintf((char *)resonse.response_buf, ATCI_UART_TX_FIFO_BUFFER_SIZE, \
                             "Eint key init fail!\r\n");
                }

                resonse.response_len = strlen((char *)resonse.response_buf);
                resonse.response_flag = ATCI_RESPONSE_FLAG_URC_FORMAT;
                atci_send_response(&resonse);

                break;
            }


        default :
            /* others are invalid command format */
            strncpy((char *)resonse.response_buf, "Wrong command ERROR\r\n", strlen("Wrong command ERROR\r\n"));
            resonse.response_len = strlen((char *)resonse.response_buf);
            resonse.response_flag = ATCI_RESPONSE_FLAG_APPEND_ERROR;
            atci_send_response(&resonse);
            break;
    }

    snprintf((char *)resonse.response_buf, ATCI_UART_TX_FIFO_BUFFER_SIZE, \
             "/*******next command line********\r\n");
    resonse.response_len = strlen((char *)resonse.response_buf);
    resonse.response_flag = ATCI_RESPONSE_FLAG_URC_FORMAT;
    atci_send_response(&resonse);
    return ATCI_STATUS_OK;
}

#endif

