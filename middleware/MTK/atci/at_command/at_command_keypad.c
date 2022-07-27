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

// For Register AT command handler
// System head file

#include "hal_keypad.h"

#if !defined(MTK_KEYPAD_AT_COMMAND_DISABLE) && defined(HAL_KEYPAD_MODULE_ENABLED)

#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include <string.h>
#include <stdio.h>
#include "at_command.h"
#include "syslog.h"
#include "keypad_custom.h"
#include "hal_gpio.h"
#include "hal_keypad_internal.h"
#include "task_def.h"
#include <stdlib.h>
#include "hal_platform.h"

log_create_module(atci_keypad, PRINT_LEVEL_INFO);

#define LOGE(fmt,arg...)   LOG_E(atci_keypad, "[keypad]"fmt,##arg)
#define LOGW(fmt,arg...)   LOG_W(atci_keypad, "[keypad]"fmt,##arg)
#define LOGI(fmt,arg...)   LOG_I(atci_keypad ,"[keypad]"fmt,##arg)

#define LOGMSGIDE(fmt,cnt,arg...)   LOG_MSGID_E(atci_keypad ,"[keypad]"fmt,cnt,##arg)
#define LOGMSGIDW(fmt,cnt,arg...)   LOG_MSGID_W(atci_keypad ,"[keypad]"fmt,cnt,##arg)
#define LOGMSGIDI(fmt,cnt,arg...)   LOG_MSGID_I(atci_keypad ,"[keypad]"fmt,cnt,##arg)

/*
 * sample code
*/

/*--- Function ---*/
atci_status_t atci_cmd_hdlr_keypad(atci_parse_cmd_param_t *parse_cmd);

/*
EKEYURC=<op>    OK /ERROR

EKEY keypad position:<key_position>, data:<key_data>, state:<key_state>

<op>: integer
0   turn off key URC information.
1   turn on keypad mode set to EPT tool configuration.
2   set to single key mode and open all keypad pin.
3   set to double key mode and open all keypad pin.
<key_position >: integer
The key position mapping to hardware number.

<key_data>: integer
The key data mapping to EPT tool configuration.

<key_state>: char
"release"
"press"
"longpress"
"repeat"

sample:
AT+EKEYURC=1        // turn on key URC information
OK
AT+EKEYURC=2        //set to single key
OK

EKEY keypad position:[1], data:[34], state:[press]
OK
EKEY keypad position:[1], data:[34], state:[release]
OK
EKEY keypad position:[10], data:[254], state:[press]
OK

AT+EKEYURC=3        //set to doule key
OK

EKEY keypad position:[10], data:[254], state:[release]
OK
EKEY keypad position:[19], data:[254], state:[press]
OK
EKEY keypad position:[19], data:[254], state:[release]
OK

AT+EKEYURC=0      // turn off key URC information
OK
*/

QueueHandle_t atci_keypad_queue_handle = NULL;

TaskHandle_t  atci_keypad_task_handle = NULL;

typedef enum {
    ATCI_KEYPAD_GET_KEY_DATA = 1,
    ATCI_POWERKEY_GET_KEY_DATA = 2
} atci_keypad_msg_type_t;

typedef struct {
    char *src_mod;
    atci_keypad_msg_type_t msg_id;
} atci_keypad_msg_t;


void atci_keypad_task_main(void *pvParameters);

void atci_keypad_task_init(void)
{
    atci_keypad_queue_handle   = xQueueCreate(ATCI_KEYPAD_QUEUE_LENGTH, sizeof(atci_keypad_msg_t));
    xTaskCreate(atci_keypad_task_main, ATCI_KEYPAD_TASK_NAME, ATCI_KEYPAD_TASK_STACKSIZE / sizeof(StackType_t), NULL, TASK_PRIORITY_NORMAL, &atci_keypad_task_handle);
}

void atci_keypad_task_deinit(void)
{
    vQueueDelete(atci_keypad_queue_handle);
    vTaskDelete(atci_keypad_task_handle);
    atci_keypad_queue_handle = NULL;
    atci_keypad_task_handle = NULL;
}

void atci_keypad_gpio_init(void)
{
#if 0
#ifdef HAL_GPIO_MODULE_ENABLED
    hal_pinmux_set_function(HAL_GPIO_3, 4)  ;   //set to row0
    hal_pinmux_set_function(HAL_GPIO_4, 4)  ;   //set to row1
    // hal_pinmux_set_function((hal_gpio_pin_t)21, 1)  ;   //set to row2
    hal_pinmux_set_function(HAL_GPIO_0, 4)  ;   //set to col0
    hal_pinmux_set_function(HAL_GPIO_1, 4)  ;   //set to col1
    // hal_pinmux_set_function((hal_gpio_pin_t)18, 1)  ;   //set to col2
#endif
#endif
}

void atci_keypad_set_key_mode(hal_keypad_mode_t mode)
{
#if 0
    if (mode == HAL_KEYPAD_MODE_SINGLE_KEY) {
        keypad->KP_SEL =  0x1c70;
    } else {
        keypad->KP_SEL =  0x1c71;
    }
#endif
}

atci_response_t resonse = {{0}};
void atci_keypad_task_main(void *pvParameters)
{
    atci_keypad_msg_t msgs;


    volatile int  key_data  = 0;
    volatile int  key_state = 0;
    volatile uint32_t key_position = 0;

    hal_keypad_event_t          keypad_event;
#ifdef HAL_KEYPAD_FEATURE_POWERKEY
    hal_keypad_powerkey_event_t powekey_event;
#endif
    hal_keypad_status_t         ret;

    static uint32_t stack_max = 0;
    uint32_t stack_cur = 0;

    char *string[5] = {"release", "press", "longpress", "repeat", "pmu_longpress"};
    while (1) {
        if (xQueueReceive(atci_keypad_queue_handle, &msgs, portMAX_DELAY)) {
            switch (msgs.msg_id) {
                case ATCI_KEYPAD_GET_KEY_DATA: {
                    LOGMSGIDI("[user]**enter keypad hanlder get event in at command\r\n", 0);
                    while (1) {
                        ret = hal_keypad_get_key(&keypad_event);

                        if (ret == HAL_KEYPAD_STATUS_ERROR) {
                            LOGMSGIDI("[keypad][DATA]normal no key in buffer\r\n", 0);
                            break;
                        }

                        key_position  = keypad_event.key_data; //key postion in register
                        key_state     = keypad_event.state;
                        key_data      = keypad_custom_translate_keydata(keypad_event.key_data);

                        LOGMSGIDI("[keypad][DATA]keypad state = %d, key_position = %d, key_data = %d\r\n",3, keypad_event.state, key_position, keypad_event.key_data);

                        snprintf((char *)resonse.response_buf, ATCI_UART_TX_FIFO_BUFFER_SIZE, "EKEY keypad position:[%d], data:[%d], state:[%s]\r\n", (int)key_position, (int)key_data, (char *)string[key_state]);
                        /* ATCI will help append "OK" at the end of resonse buffer  */
                        resonse.response_flag |= ATCI_RESPONSE_FLAG_URC_FORMAT;
                        resonse.response_len = strlen((char *)resonse.response_buf);
                        atci_send_response(&resonse);
                    }
                }
                break;

#ifdef HAL_KEYPAD_FEATURE_POWERKEY
                case ATCI_POWERKEY_GET_KEY_DATA: {
                    LOGMSGIDI("[user]**enter powerkey hanlder get event in at command\r\n", 0);
                    while (1) {
                        ret = hal_keypad_powerkey_get_key(&powekey_event);

                        if (ret == HAL_KEYPAD_STATUS_ERROR) {
                            LOGMSGIDI("[keypad][DATA]powerkey no key in buffer\r\n", 0);
                            break;
                        }

                        key_state     = powekey_event.state;
                        key_data      = powekey_event.key_data;

                        LOGMSGIDI("[keypad][DATA]powerkey state = %d, key_data = %d\r\n",2, powekey_event.state, powekey_event.key_data);

                        snprintf((char *)resonse.response_buf, ATCI_UART_TX_FIFO_BUFFER_SIZE, "EKEY powerkey data:[%d], state:[%s]\r\n", key_data, string[key_state]);
                        /* ATCI will help append "OK" at the end of resonse buffer  */
                        resonse.response_flag |= ATCI_RESPONSE_FLAG_URC_FORMAT;
                        resonse.response_len = strlen((char *)resonse.response_buf);
                        atci_send_response(&resonse);
                    }
                }
                break;
#endif

                default: {


                }
                break;
            }

            stack_cur = (ATCI_KEYPAD_TASK_STACKSIZE / sizeof(StackType_t) - uxTaskGetStackHighWaterMark(atci_keypad_task_handle)) * sizeof(portSTACK_TYPE);
            if (stack_cur > stack_max) {
                stack_max = stack_cur;
                LOGMSGIDI("KEYPAD_atci stack max-usage:%d \r\n",1, stack_max);
            }
        }
    }
}

#ifdef HAL_KEYPAD_FEATURE_POWERKEY
void atci_keypad_powerkey_handler(void)
{
    atci_keypad_msg_t msgs;
    BaseType_t xHigherPriorityTaskWoken;

    // We have not woken a task at the start of the ISR.
    xHigherPriorityTaskWoken = pdFALSE;

    msgs.msg_id =  ATCI_POWERKEY_GET_KEY_DATA;
    LOGMSGIDI("enter atci_keypad_handler \r\n", 0);
    while (xQueueSendFromISR(atci_keypad_queue_handle, &msgs, &xHigherPriorityTaskWoken) != pdTRUE);

    // Now the buffer is empty we can switch context if necessary.
    if (xHigherPriorityTaskWoken) {
        // Actual macro used here is port specific.
        portYIELD_FROM_ISR(pdTRUE);
    }

}
#endif

void atci_keypad_handler(void)
{
    atci_keypad_msg_t msgs;
    BaseType_t xHigherPriorityTaskWoken;

    // We have not woken a task at the start of the ISR.
    xHigherPriorityTaskWoken = pdFALSE;

    msgs.msg_id = ATCI_KEYPAD_GET_KEY_DATA ;
    LOGMSGIDI("enter atci_keypad_handler \r\n", 0);

    if (xQueueSendFromISR(atci_keypad_queue_handle, &msgs, &xHigherPriorityTaskWoken) == pdTRUE) {

        // Now the buffer is empty we can switch context if necessary.
        if (xHigherPriorityTaskWoken) {
            // Actual macro used here is port specific.
            portYIELD_FROM_ISR(pdTRUE);
        }
    } else {
    }

}

// AT command handler
atci_status_t atci_cmd_hdlr_keypad(atci_parse_cmd_param_t *parse_cmd)
{
    atci_response_t resonse = {{0}};
    char *param = NULL;
    int  param1_val = -1;
    uint32_t param_len = 0;
    volatile hal_keypad_status_t ret;

    ret = ret;

    LOGMSGIDI("atci_cmd_hdlr_keypad \r\n", 0);

    resonse.response_flag = 0; // Command Execute Finish.
#ifdef ATCI_APB_PROXY_ADAPTER_ENABLE
    resonse.cmd_id = parse_cmd->cmd_id;
#endif

    switch (parse_cmd->mode) {
        case ATCI_CMD_MODE_TESTING:    // rec: AT+EKEYURC=?
            strcpy((char *)resonse.response_buf, "+EKEYURC=(0,1)\r\n");
            resonse.response_flag = ATCI_RESPONSE_FLAG_APPEND_OK;
            //resonse.response_flag |= ATCI_RESPONSE_FLAG_URC_FORMAT;
            resonse.response_len = strlen((char *)resonse.response_buf);
            atci_send_response(&resonse);
            resonse.response_flag = ATCI_RESPONSE_FLAG_URC_FORMAT;
            break;
        case ATCI_CMD_MODE_EXECUTION: // rec: AT+EKEYURC=<op>  the handler need to parse the parameters
            strcpy((char *)resonse.response_buf, "\r\nkeypad atcmd\r\n");
            resonse.response_flag = ATCI_RESPONSE_FLAG_APPEND_OK;
            resonse.response_len = strlen((char *)resonse.response_buf);
            atci_send_response(&resonse);
            resonse.response_flag = ATCI_RESPONSE_FLAG_URC_FORMAT;

            //param = strtok(parse_cmd->string_ptr, ",\n\r");
            //param = strtok(parse_cmd->string_ptr, "AT+EKEYURC=");
            param = parse_cmd->string_ptr + parse_cmd->parse_pos;
            if (param==NULL) {
                LOGMSGIDI("param is null after assign\r\n", 0);
                break;
            }
            param_len = strlen(param);
            LOGMSGIDI("param_len=%d\r\n",1, param_len);
            
            if ((param_len <= 1)) {
                strcpy((char *)resonse.response_buf, "ERROR\r\n");
                resonse.response_len = strlen((char *)resonse.response_buf);
                atci_send_response(&resonse);
                break;
            }
            param1_val = atoi(param);
            LOGMSGIDI("param=%d\r\n",1, param1_val);

            if (param != NULL && ((param1_val >= 0) && (param1_val <= 3))) {
                /* valid parameter, update the data and return "OK"
                                param1_val = 0 :  Turn off key URC information.
                                param1_val = 1 :  Turn on key URC information.
                                param1_val = 2 :  Set to open all keypad pin, and select single key mode.
                                param1_val = 3 :  Set to open all keypad pin, and select double key mode.
                          */

                if (param1_val == 1) {
                    /*set to EPT default mode, and init keypad*/
                    if (keypad_custom_init() == true) {
                        atci_keypad_task_init();
                        ret = hal_keypad_register_callback((hal_keypad_callback_t)atci_keypad_handler, NULL);
                        ret = hal_keypad_enable();
#ifdef HAL_KEYPAD_FEATURE_POWERKEY
                        ret = hal_keypad_powerkey_register_callback((hal_powerkey_callback_t)atci_keypad_powerkey_handler, NULL);
#endif
                        LOGMSGIDI("keypad and powerkey initilized OK...\r\n", 0);
                    } else {
                        LOGMSGIDI("keypad and powerkey has been initilized...\r\n", 0);
                        /* others are invalid command format */
                        strcpy((char *)resonse.response_buf, "\r\nKeypad init error\r\n");
                        resonse.response_flag = ATCI_RESPONSE_FLAG_URC_FORMAT;
                        resonse.response_len = strlen((char *)resonse.response_buf);
                        atci_send_response(&resonse);
                        return ATCI_STATUS_ERROR;
                    }
                } else if (param1_val == 2) {   /*set to single key and open all keypad GPTO pin*/
                    atci_keypad_set_key_mode(HAL_KEYPAD_MODE_SINGLE_KEY);
                    atci_keypad_gpio_init();
                    LOGMSGIDI("keypad set to single key and all keypad GPIO pin is opened...\r\n", 0);
                } else if (param1_val == 3) {   /*set to double key and open all keypad GPTO pin*/
                    atci_keypad_set_key_mode(HAL_KEYPAD_MODE_DOUBLE_KEY);
                    atci_keypad_gpio_init();
                    LOGMSGIDI("keypad set to double key all keypad GPIO pin is opened...\r\n", 0);
                } else if (param1_val == 0) {   /*deinit keypad and disable keypad module*/
                    if (keypad_custom_deinit() == true) {
                        if (atci_keypad_queue_handle != NULL) {
                            atci_keypad_task_deinit();
                            ret = hal_keypad_disable();
                        }
                        LOGMSGIDI("keypad and powerkey deinitilized OK...\r\n", 0);
                    } else {
                        LOGMSGIDI("keypad and powerkey deinitilized error...\r\n", 0);
                    }
                }
            } else {
                strcpy((char *)resonse.response_buf, "Invalide parameter\r\n");
                resonse.response_flag = ATCI_RESPONSE_FLAG_APPEND_ERROR;
                resonse.response_len = strlen((char *)resonse.response_buf);
                atci_send_response(&resonse);
                return ATCI_STATUS_ERROR;
            }
            break;
        default :
            /* others are invalid command format */
            strcpy((char *)resonse.response_buf, "Invalide command\r\n");
            resonse.response_flag = ATCI_RESPONSE_FLAG_APPEND_ERROR;
            resonse.response_len = strlen((char *)resonse.response_buf);
            atci_send_response(&resonse);
            break;
    }
    return ATCI_STATUS_OK;
}

#endif

