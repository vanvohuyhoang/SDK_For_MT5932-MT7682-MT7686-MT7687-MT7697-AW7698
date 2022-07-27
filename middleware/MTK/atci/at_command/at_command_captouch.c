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

#include "FreeRTOS.h"
#include "task.h"
#include <string.h>
#include <stdio.h>
#include "at_command.h"
#include "hal_gpt.h"
#include "syslog.h"
#include <stdlib.h>

#ifdef HAL_CAPTOUCH_MODULE_ENABLED

#include "hal_captouch.h"
#include "hal_captouch_internal.h"
#include "nvdm.h"

#define CAPTOUCH_SMT
#define CAPTOUCH_HQA

#ifdef AIRO_KEY_EVENT_ENABLE
#include "airo_key_event.h"
#endif
/*
 * sample code
 * AT+ECPT=GPIO_GET
 * AT+ECPT=GPIO_SET:<pin number>:<mode><dir><pull><output>
 * AT+ECPT=GPIO_SET_MODE:<pin number>:<mode>
 * ... ...
*/

log_create_module(atci_cpt, PRINT_LEVEL_INFO);
#define LOGE(fmt,arg...)   LOG_E(atci_cpt, "[CPT]"fmt,##arg)
#define LOGW(fmt,arg...)   LOG_W(atci_cpt, "[CPT]"fmt,##arg)
#define LOGI(fmt,arg...)   LOG_I(atci_cpt ,"[CPT]"fmt,##arg)

#define LOGMSGIDE(fmt,cnt,arg...)   LOG_MSGID_E(atci_cpt ,"[CPT]"fmt,cnt,##arg)
#define LOGMSGIDW(fmt,cnt,arg...)   LOG_MSGID_W(atci_cpt ,"[CPT]"fmt,cnt,##arg)
#define LOGMSGIDI(fmt,cnt,arg...)   LOG_MSGID_I(atci_cpt ,"[CPT]"fmt,cnt,##arg)

/*--- Function ---*/
atci_status_t atci_cmd_hdlr_captouch(atci_parse_cmd_param_t *parse_cmd);


/* AT command handler  */

void atci_captouch_send_response(int32_t *len1, atci_response_t *resonse)
{
    resonse->response_len = (uint16_t)(* len1);
    resonse->response_flag = ATCI_RESPONSE_FLAG_URC_FORMAT;
    atci_send_response(resonse);
}


void atci_captouch_interrupt_handler(void *user_data)
{

    hal_captouch_event_t event;
    hal_captouch_status_t ret;
    atci_response_t resonse = {{0}};
    int32_t ret_len1;

    hal_captouch_key_state_t state;
    uint32_t   key_data;
    uint32_t   time_stamp;

    while (1) {
        ret = hal_captouch_get_event(&event);

        if (ret == HAL_CAPTOUCH_STATUS_NO_EVENT) {
            ret_len1 =  snprintf((char *)(resonse.response_buf), ATCI_UART_TX_FIFO_BUFFER_SIZE, \
                                 "Buffer empty");
            atci_captouch_send_response(&ret_len1, &resonse);
            break;
        }

        time_stamp  = event.time_stamp;
        state       = event.state;
        key_data    = event.key_data;

        LOGMSGIDI("[captouch]key_data = %d, state = %d, time_stamp = %d;", 3, (int)key_data, (int)state, (int)time_stamp);

        snprintf((char *)resonse.response_buf, ATCI_UART_TX_FIFO_BUFFER_SIZE, "\r\n[captouch]key_data = %d, state = %d, time_stamp = %d; ", (int)key_data, (int)state, (int)time_stamp);
        /* ATCI will help append "OK" at the end of resonse buffer  */
        resonse.response_flag |= ATCI_RESPONSE_FLAG_URC_FORMAT;
        resonse.response_len = strlen((char *)resonse.response_buf);
        atci_send_response(&resonse);
    }
}

atci_status_t atci_cmd_hdlr_captouch(atci_parse_cmd_param_t *parse_cmd)
{
    atci_response_t resonse = {{0}};

    char *param1 = NULL;
    char *param2 = NULL;
    char *param3 = NULL;
    char *param4 = NULL;
#ifdef AIRO_KEY_EVENT_ENABLE
#if 0
    char *param5 = NULL;
    char *param6 = NULL;
    char *param7 = NULL;
    char *param8 = NULL;
#endif
#endif

    uint32_t i;
    uint8_t temp_buf[256];
    uint8_t temp1[20], temp2[20];
    int32_t ret_len1, ret_len2;
    uint32_t config_data[10];
    uint16_t udata[50];
    int16_t  idata[50];
    uint32_t temp;
    uint32_t count;
    int32_t  total[2];

    char 	 nvkey[5];
    uint16_t captouch_nveky = 0xD100;

    hal_captouch_status_t ret;

    hal_captouch_config_t config;

    hal_captouch_tune_data_t tune_data[8];

    hal_captouch_nvdm_data captouch_data;

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
            /*
            description: Test captouch at command
            command: AT+ECPT=?\0d\0a
            */
            strncpy((char *)resonse.response_buf, "ATCI_CMD_MODE_TESTING", strlen("ATCI_CMD_MODE_TESTING"));
            resonse.response_len = strlen((char *)resonse.response_buf);
            resonse.response_flag = ATCI_RESPONSE_FLAG_APPEND_OK;
            atci_send_response(&resonse);

            ret_len1 =  snprintf((char *)(resonse.response_buf), ATCI_UART_TX_FIFO_BUFFER_SIZE, \
                                 "Captouch AT command test, the EPT configured used channel is 0x%x\r\n", CAPTOUCH_USED_CHANNEL_MASK);
            atci_captouch_send_response(&ret_len1, &resonse);

            break;

        case ATCI_CMD_MODE_EXECUTION:
            strncpy((char *)resonse.response_buf, "ATCI_CMD_MODE_EXECUTION", strlen("ATCI_CMD_MODE_EXECUTION"));
            resonse.response_len = strlen((char *)resonse.response_buf);
            resonse.response_flag = ATCI_RESPONSE_FLAG_APPEND_OK;
            atci_send_response(&resonse);

            param1 = strtok(parse_cmd->string_ptr, ":");

            /*
            description: Read or wirte the HIF 26M register
            command: AT+ECPT=RW_RG:[R/W],[ADD],[DATA]\0d\0a
            example: AT+ECPT=RW_RG:W,0xa2110004,0x12345678\0d\0a  //writ the data 0x12345678 to the addr 0xa2110004
                     AT+ECPT=RW_RG:R,0xa2110004,NULL\0d\0a        //read the addr 0xa2110004,don't care null
            */
            if (strncmp(param1, "AT+ECPT=RW_RG", strlen("AT+ECPT=RW_RG")) == 0) {
                param2 = strtok(NULL, ",");
                param3 = strtok(NULL, ",");
                param4 = strtok(NULL, ",");

                strncpy((char *)temp1, (char *)param3, strlen((char *)param3));
                strncpy((char *)temp2, (char *)param4, strlen((char *)param4));

                config_data[0] = (uint32_t)strtoul((char *)temp1, NULL, 16);
                config_data[1] = (uint32_t)strtoul((char *)temp2, NULL, 16);

                if ((*(char *)param2 == 'W') || (*(char *)param2 == 'w')) {
                    *((volatile uint32_t *)(config_data[0])) = config_data[1];
                    hal_gpt_delay_us(1);
                    temp = *((volatile uint32_t *)(config_data[0]));

                    snprintf((char *)resonse.response_buf, ATCI_UART_TX_FIFO_BUFFER_SIZE, \
                             "Write Captouch HIF addr:0x%x = 0x%x done,read=0x%x!\r\n", (int)config_data[0], (int)config_data[1], (int)temp);
                } else {
                    temp = *((volatile uint32_t *)(config_data[0]));
                    snprintf((char *)resonse.response_buf, ATCI_UART_TX_FIFO_BUFFER_SIZE, \
                             "Read Captouch HIF addr:0x%x = 0x%x done!\r\n", (int)config_data[0], (int)temp);
                }

                resonse.response_len = strlen((char *)resonse.response_buf);
                resonse.response_flag = ATCI_RESPONSE_FLAG_URC_FORMAT;
                atci_send_response(&resonse);

                break;
            }

            /*
            description: The wire3 read or wirte the analog reigster
            command: AT+ECPT=RW_WIRE3:[R/W],[ADD],[DATA]\0d\0a
            example: AT+ECPT=RW_WIRE3:W,0x01,0x1234\0d\0a      //writ the data 0x1234 to the addr 0x01
                     AT+ECPT=RW_WIRE3:R,0x01,NULL\0d\0a        //read the addr 0x01,don't care null
                     AT+ECPT=RW_WIRE3:A,NULL,NULL\0d\0a        //read the all analog register,don't care null
            */
            if (strncmp(param1, "AT+ECPT=RW_WIRE3", strlen("AT+ECPT=RW_WIRE3")) == 0) {
                param2 = strtok(NULL, ",");
                param3 = strtok(NULL, ",");
                param4 = strtok(NULL, ",");

                strncpy((char *)temp1, (char *)param3, strlen((char *)param3));
                strncpy((char *)temp2, (char *)param4, strlen((char *)param4));

                config_data[0] = (uint32_t)strtol((char *)temp1, NULL, 0);
                config_data[1] = (uint32_t)strtol((char *)temp2, NULL, 0);

                if (config_data[0] > 0x27) {
                    snprintf((char *)resonse.response_buf, ATCI_UART_TX_FIFO_BUFFER_SIZE, \
                             "input error parameter, your input addr is 0x%d, it must be <0x27\r\n", (int)config_data[0]);
                    resonse.response_len = strlen((char *)resonse.response_buf);
                    resonse.response_flag = ATCI_RESPONSE_FLAG_URC_FORMAT;
                    atci_send_response(&resonse);
                    break;
                }

                if (config_data[1] > 0xffff) {
                    snprintf((char *)resonse.response_buf, ATCI_UART_TX_FIFO_BUFFER_SIZE, \
                             "input error parameter, your input data is 0x%d, it must be not over than 16bit\r\n", (int)config_data[1]);
                    resonse.response_len = strlen((char *)resonse.response_buf);
                    resonse.response_flag = ATCI_RESPONSE_FLAG_URC_FORMAT;
                    atci_send_response(&resonse);
                    break;
                }


                if ((*(char *)param2 == 'W') || (*(char *)param2 == 'w')) {
                    captouch_analog_write_data(config_data[0], config_data[1]);
                    temp = captouch_analog_read_data(config_data[0]);

                    snprintf((char *)resonse.response_buf, ATCI_UART_TX_FIFO_BUFFER_SIZE, \
                             "Write Captouch Analog addr:0x%x = 0x%x done,read=0x%x!\r\n", (int)config_data[0], (int)config_data[1], (int)temp);
                } else if ((*(char *)param2 == 'A')) {
                    ret_len1 = snprintf((char *)resonse.response_buf, ATCI_UART_TX_FIFO_BUFFER_SIZE, \
                                        "Read Captouch Analog addr:\r\n");

                    ret_len2 = 0;
                    for (i = 0; i < 0x28; i++) {
                        udata[i] = captouch_analog_read_data(i);

                        if (((i >= 0x2) && (i <= 0x11)) || (i == 0x1d) || (i == 0x1e)) {
                            idata[i] = captouch_9signed_to_16signed(udata[i]);
                            ret_len1 = snprintf((char *)(resonse.response_buf + ret_len2), ATCI_UART_TX_FIFO_BUFFER_SIZE, \
                                                "0x%.2x = 0x%x, u:%d, s:%d\r\n", (int)i, (int)udata[i], (int)udata[i], idata[i]);
                        } else if (((i >= 0x11) && (i <= 0x19)) || (i == 0x1c)) {
                            idata[i] = captouch_7signed_to_16signed(udata[i]);
                            ret_len1 = snprintf((char *)(resonse.response_buf + ret_len2), ATCI_UART_TX_FIFO_BUFFER_SIZE, \
                                                "0x%.2x = 0x%x, u:%d, s:%d\r\n", (int)i, (int)udata[i], (int)udata[i], idata[i]);
                        } else if (i == 0x23) {
                            temp_buf[0] = (udata[i] >> 0) & 0x7;
                            temp_buf[1] = (udata[i] >> 3) & 0x7;
                            temp_buf[2] = (udata[i] >> 6) & 0x7;
                            temp_buf[3] = (udata[i] >> 9) & 0x7;
                            ret_len1 = snprintf((char *)(resonse.response_buf + ret_len2), ATCI_UART_TX_FIFO_BUFFER_SIZE, \
                                                "0x%.2x = 0x%x, ch0:%d, ch1:%d, ch2:%d, ch3:%d\r\n", (int)i, (int)udata[i], temp_buf[0], temp_buf[1], temp_buf[2], temp_buf[3]);
                        } else if (i == 0x24) {
                            temp_buf[0] = (udata[i] >> 0) & 0x7;
                            temp_buf[1] = (udata[i] >> 3) & 0x7;
                            temp_buf[2] = (udata[i] >> 6) & 0x7;
                            temp_buf[3] = (udata[i] >> 9) & 0x7;
                            ret_len1 = snprintf((char *)(resonse.response_buf + ret_len2), ATCI_UART_TX_FIFO_BUFFER_SIZE, \
                                                "0x%.2x = 0x%x, ch4:%d, ch5:%d, ch6:%d, ch7:%d\r\n", (int)i, (int)udata[i], temp_buf[0], temp_buf[1], temp_buf[2], temp_buf[3]);
                        } else {
                            ret_len1 = snprintf((char *)(resonse.response_buf + ret_len2), ATCI_UART_TX_FIFO_BUFFER_SIZE, \
                                                "0x%.2x = 0x%x\r\n", (int)i, (int)udata[i]);
                        }
                        if ((ret_len1 + ret_len2) >= ATCI_UART_TX_FIFO_BUFFER_SIZE) {
                            resonse.response_len = strlen((char *)resonse.response_buf);
                            resonse.response_flag = ATCI_RESPONSE_FLAG_URC_FORMAT;
                            atci_send_response(&resonse);
                            ret_len2 = 0;
                            vTaskDelay(2 / portTICK_PERIOD_MS);
                        } else {
                            ret_len2 = ret_len1 + ret_len2;
                        }
                    }
                } else {
                    temp = captouch_analog_read_data(config_data[0]);
                    snprintf((char *)resonse.response_buf, ATCI_UART_TX_FIFO_BUFFER_SIZE, \
                             "Read Captouch Analog addr:0x%x = 0x%x done!\r\n", (int)config_data[0], (int)temp);
                }

                resonse.response_len = strlen((char *)resonse.response_buf);
                resonse.response_flag = ATCI_RESPONSE_FLAG_URC_FORMAT;
                atci_send_response(&resonse);

                break;
            }



            /*
            description: Init and deinit the captouch driver
            command: AT+ECPT=INIT:[channle_bit_map][init/deinit]\0d\0a
            example: AT+ECPT=INIT:0xff,1\0d\0a        //call hal_captouch_init() to init the captouch channel0~7 driver
                     AT+ECPT=INIT:0x1,0\0d\0a         //call hal_captouch_deinit() to deinit the captouch channel0 driver
            */
            if (strncmp(param1, "AT+ECPT=INIT", strlen("AT+ECPT=INIT")) == 0) {

                param2 = strtok(NULL, ",");
                param3 = strtok(NULL, ",");
                param4 = strtok(NULL, ",");

                strncpy((char *)temp1, (char *)param2, strlen((char *)param2));
                strncpy((char *)temp2, (char *)param3, strlen((char *)param3));

                config_data[0] = (uint32_t)strtol((char *)temp1, NULL, 0); //channel bit map
                config_data[1] = (uint32_t)strtol((char *)temp2, NULL, 0); // 1:init; 0:deinit

                if (
                    (config_data[0] > 0xff) || (config_data[1] > 1)) {
                    ret_len1 =  snprintf((char *)(resonse.response_buf), ATCI_UART_TX_FIFO_BUFFER_SIZE, \
                                         "The input pareameter is error,channel_bit_map:0x%x, init:%d\r\n", (int)config_data[0], (int)config_data[1]);
                    atci_captouch_send_response(&ret_len1, &resonse);
                    break;;
                }

                if (config_data[1] == 1) {
                    config.callback.callback  = atci_captouch_interrupt_handler;
                    config.callback.user_data = NULL;
                    config.channel_bit_map = config_data[0];
                    for (i = 0; i < 8; i++) {
                        config.high_thr[i] = 200;
                        config.low_thr[i]  = 50;
                        config.coarse_cap[i] = 2;
                        config.mavg_r = 0xff;
                        config.avg_s  = 0xff;
                    }
                    hal_captouch_init(&config);

                    ret_len1 =  snprintf((char *)(resonse.response_buf), ATCI_UART_TX_FIFO_BUFFER_SIZE, \
                                         "Captouch init done!dafault coare_cap=2, thr_h:210,thr_l:50,bit_map:0x%x,mavg_r:10,avg_s:5\r\n", (int)config_data[0]);
                    atci_captouch_send_response(&ret_len1, &resonse);
                } else if (config_data[1] == 0) {

                    hal_captouch_deinit();
                    ret_len1 =  snprintf((char *)(resonse.response_buf), ATCI_UART_TX_FIFO_BUFFER_SIZE, \
                                         "Captouch deinit done!\r\n");
                    atci_captouch_send_response(&ret_len1, &resonse);
                }

                break;
            }


            /*
            description: set debug channel
            command: AT+ECPT=SET_DBG:[channle]\0d\0a
            example: AT+ECPT=SET_DBG:1\0d\0a          //set channel 1 debug out
            AT+ECPT=SET_DBG:2\0d\0a          //set channel 2 debug out
            */
            if (strncmp(param1, "AT+ECPT=SET_DBG", strlen("AT+ECPT=SET_DBG")) == 0) {
                param2 = strtok(NULL, ",");

                strncpy((char *)temp1, (char *)param2, strlen((char *)param2));

                config_data[0] = (uint32_t)atoi(param2);

                captouch_get_tune_state(config_data[0], &tune_data[0]);

                ret_len1 =  snprintf((char *)(resonse.response_buf), ATCI_UART_TX_FIFO_BUFFER_SIZE, \
                                     "ch:%d, crs_cap=%d, f_cap=%d, m_cap=%d, avg=%d,vadc=%d\r\n", \
                                     (int)config_data[0], (int)tune_data[0].coarse_cap, (int)tune_data[0].fine_cap, (int)tune_data[0].man, (int)tune_data[0].avg_adc, (int)tune_data[0].vadc);

                atci_captouch_send_response(&ret_len1, &resonse);
                vTaskDelay(100 / portTICK_PERIOD_MS);

                break;
            }


            /*description: set the channel to get the specify data number to fine the max and the min value, calulate the delta.
            command: AT+ECPT=GET_P2P:[channle],[data_num]\0d\0a
            example: AT+ECPT=GET_P2P:1,512\0d\0a          //get the peak-to-peak value of the channel in 512 data.
                     AT+ECPT=GET_P2P:2,512\0d\0a          //get the peak-to-peak value of the channe2 in 512 data.
            */
#ifdef CAPTOUCH_HQA
            if (strncmp(param1, "AT+ECPT=GET_P2P", strlen("AT+ECPT=GET_P2P")) == 0) {
                param2 = strtok(NULL, ",");
                param3 = strtok(NULL, ",");

                strncpy((char *)temp1, (char *)param2, strlen((char *)param2));
                strncpy((char *)temp2, (char *)param3, strlen((char *)param3));

                config_data[0] = (uint32_t)atoi(param2);
                config_data[1] = (uint32_t)atoi(param3);

                idata[0] = -256;  // vadc max value
                idata[1] = 255;   // vadc min value
                total[0] = 0;     // average value
                idata[3] = -256;  // avgadc max value
                idata[4] = 255;   // avgadc min value
                total[1] = 0;     // avgerge value
                idata[6] = -64;   // fine cap max value
                idata[7] = 63;    // fine cap min value

                udata[0] = 0;     // coarse cap max value
                udata[1] = 7;     // coarse cap min value

                for (i = 0; i < config_data[1]; i++) {
                    captouch_get_tune_state(config_data[0], &tune_data[0]);

                    if (tune_data[0].vadc > idata[0]) {
                        idata[0] = tune_data[0].vadc;
                    }

                    if (tune_data[0].vadc < idata[1]) {
                        idata[1] = tune_data[0].vadc;
                    }

                    total[0] = total[0] + tune_data[0].vadc;

                    if (tune_data[0].avg_adc > idata[3]) {
                        idata[3] = tune_data[0].avg_adc;
                    }

                    if (tune_data[0].avg_adc < idata[4]) {
                        idata[4] = tune_data[0].avg_adc;
                    }

                    total[1] = total[1] + tune_data[0].avg_adc;


                    if (tune_data[0].fine_cap > idata[6]) {
                        idata[6] = tune_data[0].fine_cap;
                    }

                    if (tune_data[0].fine_cap < idata[7]) {
                        idata[7] = tune_data[0].fine_cap;
                    }


                    if (tune_data[0].coarse_cap > udata[0]) {
                        udata[0] = tune_data[0].coarse_cap;
                    }

                    if (tune_data[0].coarse_cap < udata[1]) {
                        udata[1] = tune_data[0].coarse_cap;
                    }
                }

                idata[8] = idata[0] - idata[1]; //delta vadc
                idata[9] = idata[3] - idata[4]; //deta avg

                total[0] = (int32_t)(total[0] / config_data[1]); //vadc avg
                total[1] = (int32_t)(total[1] / config_data[1]); //avg avg

                ret_len1 =  snprintf((char *)(resonse.response_buf), ATCI_UART_TX_FIFO_BUFFER_SIZE, \
                                     "ch[%d] %d data:\r\n max:avg=%d,vadc=%d,crs_cap=%d, f_cap=%d\r\n min:avg=%d,vadc=%d,crs_cap=%d, f_cap=%d\r\n avg:avg=%d,vadc=%d\r\n del:avg=%d,vadc=%d\r\n", \
                                     (int)config_data[0], (int)config_data[1], \
                                     (int)idata[3], (int)idata[0], (int)udata[0], (int)idata[6], \
                                     (int)idata[4], (int)idata[1], (int)udata[1], (int)idata[7], \
                                     (int)total[1], (int)total[0], \
                                     (int)idata[9], (int)idata[8]);

                atci_captouch_send_response(&ret_len1, &resonse);
                vTaskDelay(100 / portTICK_PERIOD_MS);

                break;
            }
#endif

            /*
            description: set lowper or normal mode
            command: AT+ECPT=SET_LP:[data]\0d\0a
            example: AT+ECPT=SET_LP:0\0d\0a          //set normal mode
                     AT+ECPT=SET_LP:1\0d\0a          //set lowpower mode
            */
            if (strncmp(param1, "AT+ECPT=SET_LP", strlen("AT+ECPT=SET_LP")) == 0) {
                param2 = strtok(NULL, ",");

                strncpy((char *)temp1, (char *)param2, strlen((char *)param2));

                config_data[0] = (uint32_t)atoi(param2);

                if (config_data[0] == 0) {
                    hal_captouch_lowpower_control(HAL_CAPTOUCH_MODE_NORMAL);
                    ret_len1 =  snprintf((char *)(resonse.response_buf), ATCI_UART_TX_FIFO_BUFFER_SIZE, "set to normal mode done!\r\n");
                } else if (config_data[0] == 1) {
                    hal_captouch_lowpower_control(HAL_CAPTOUCH_MODE_LOWPOWER);
                    ret_len1 =  snprintf((char *)(resonse.response_buf), ATCI_UART_TX_FIFO_BUFFER_SIZE, "set to lowpoer mode done!\r\n");
                } else {
                    ret_len1 =  snprintf((char *)(resonse.response_buf), ATCI_UART_TX_FIFO_BUFFER_SIZE, "lowpoer mode data=%d wrong,should <2\r\n", (int)config_data[0]);
                }

                atci_captouch_send_response(&ret_len1, &resonse);
                vTaskDelay(100 / portTICK_PERIOD_MS);

                break;
            }

            /*
            description: Calibration control
            command: AT+ECPT=CAL_KEY:[channel],[key_type]\0d\0a
            example: AT+ECPT=CAL_KEY:0xff,0\0d\0a      //HAL_CAPTOUCH_TUNE_MAN,channel0~7 need to file the coarse_cap and the fine_cap value
                     AT+ECPT=CAL_KEY:0x1,1\0d\0a       //HAL_CAPTOUCH_TUNE_HW_AUTO,channel0 hw auto key
                     AT+ECPT=CAL_KEY:0x80,2\0d\0a      //HAL_CAPTOUCH_TUNE_SW_AUTO,channel7 sw auto key, need more time
            */
            if (strncmp(param1, "AT+ECPT=CAL_KEY", strlen("AT+ECPT=CAL_KEY")) == 0) {
                param2 = strtok(NULL, ",");
                param3 = strtok(NULL, ",");

                strncpy((char *)temp1, (char *)param2, strlen((char *)param2));
                strncpy((char *)temp2, (char *)param3, strlen((char *)param3));

                config_data[0] = (uint32_t)strtol((char *)temp1, NULL, 0);
                config_data[1] = (uint32_t)strtol((char *)temp2, NULL, 0);

                //parameter error check
                if ((config_data[1] > 2) || (config_data[0] > 0xff) || (config_data[0] == 0)) {
                    ret_len1 =  snprintf((char *)(resonse.response_buf), ATCI_UART_TX_FIFO_BUFFER_SIZE, \
                                         "The input value AT+ECPT=CAL_KEY:0x%x,%d is error!\r\n", (int)config_data[0], (int)config_data[1]);
                    atci_captouch_send_response(&ret_len1, &resonse);
                }

                //setup the environment
                /*
                config.callback.callback  = atci_captouch_interrupt_handler;
                config.callback.user_data = NULL;
                config.channel_bit_map = 0;
                for(i=0;i<8;i++) {
                    config.high_thr[i]   = 210;
                    config.low_thr[i]    = 50;
                    config.coarse_cap[i] = 2;
                }

                if (hal_captouch_init(&config) == HAL_CAPTOUCH_STATUS_INITIALIZED) {
                    hal_captouch_deinit();
                    hal_captouch_init(&config);
                }*/

                vTaskDelay(5 / portTICK_PERIOD_MS);


                nvdm_init();
                memset(&captouch_data, 0, sizeof(hal_captouch_nvdm_data));
                temp = sizeof(hal_captouch_nvdm_data);
                snprintf(nvkey, sizeof(nvkey), "%X", captouch_nveky);
                nvdm_read_data_item("AB15", nvkey, (uint8_t *)(&captouch_data), &temp);

                ret_len2 = 0;
                ret_len1 =  snprintf((char *)(resonse.response_buf + ret_len2), ATCI_UART_TX_FIFO_BUFFER_SIZE, \
                                     "type%d tune start,ch_bit_map:0x%x!\r\n", (int)config_data[1], (int)config_data[0]);
                ret_len2 = ret_len1 + ret_len2;
                for (i = 0; i < 8; i++) {
                    if ((1 << i) & config_data[0]) {
                        ret = hal_captouch_tune_control((hal_captouch_channel_t)i, config_data[1], &tune_data[i]);
                        if (ret == HAL_CAPTOUCH_STATUS_OK) {
                            //return the press result
                            captouch_data.is_key[i]     = true;
                            captouch_data.coarse_cap[i] = tune_data[i].coarse_cap;
                            captouch_data.fine_cap[i]   = tune_data[i].fine_cap;

                            ret_len1 =  snprintf((char *)(resonse.response_buf + ret_len2), ATCI_UART_TX_FIFO_BUFFER_SIZE, \
                                                 "[OK]ch:%d, crs_cap=%d, f_cap=%d, man_cap=%d, avg=%d,vadc=%d\r\n", \
                                                 (int)i, tune_data[i].coarse_cap, tune_data[i].fine_cap, tune_data[i].man, tune_data[i].avg_adc, tune_data[i].vadc);
                        } else {
                            captouch_data.is_key[i] = false;
                            ret_len1 =  snprintf((char *)(resonse.response_buf + ret_len2), ATCI_UART_TX_FIFO_BUFFER_SIZE, \
                                                 "[Err]ch:%d, crs_cap=%d, f_cap=%d, man_cap=%d, avg=%d,vadc=%d ret=%d\r\n", \
                                                 (int)i, tune_data[i].coarse_cap, tune_data[i].fine_cap, tune_data[i].man, tune_data[i].avg_adc, tune_data[i].vadc, ret);

                        }
                        ret_len2 = ret_len1 + ret_len2;
                    }
                }

                for (i = 0; i < 8; i++) {
                    LOGMSGIDI("write:is_key=%d, crs_cap=%d, f_cap=%d\r\n", 3, captouch_data.is_key[i], captouch_data.coarse_cap[i], captouch_data.fine_cap[i]);
                }


                snprintf(nvkey, sizeof(nvkey), "%X", captouch_nveky);
                nvdm_write_data_item("AB15", nvkey, NVDM_DATA_ITEM_TYPE_STRING, (uint8_t *)(&captouch_data), sizeof(hal_captouch_nvdm_data));

                memset(&captouch_data, 0, sizeof(hal_captouch_nvdm_data));

                temp = sizeof(hal_captouch_nvdm_data);
                snprintf(nvkey, sizeof(nvkey), "%X", captouch_nveky);
                nvdm_read_data_item("AB15", nvkey, (uint8_t *)(&captouch_data), &temp);;

                for (i = 0; i < 8; i++) {
                    LOGMSGIDI("read:is_key=%d, crs_cap=%d, f_cap=%d\r\n", 3, captouch_data.is_key[i], captouch_data.coarse_cap[i], captouch_data.fine_cap[i]);
                }

                ret_len1 =  snprintf((char *)(resonse.response_buf + ret_len2), ATCI_UART_TX_FIFO_BUFFER_SIZE, \
                                     "tune done!\r\n");
                ret_len2 = ret_len1 + ret_len2;

                atci_captouch_send_response(&ret_len2, &resonse);

                break;
            }


            /*
            description: set the threshold
            command: AT+ECPT=SET_THR:[channel],[THR_VALUE_H],[THR_VALUE_L]\0d\0a
            example: AT+ECPT=SET_THR:0,100,50\0d\0a           //set the channel 0 threshold_h to 100,  set threshold_l to 50
                     AT+ECPT=SET_THR:1,-100,30\0d\0a          //set the channel 1 threshold_h to -100, set threshold_l to 30
                     AT+ECPT=SET_THR:7,200,-50\0d\0a          //set the channel 7 threshold_h to 200,  set threshold_l to -50
            */
            if (strncmp(param1, "AT+ECPT=SET_THR", strlen("AT+ECPT=SET_THR")) == 0) {
                param2 = strtok(NULL, ",");
                param3 = strtok(NULL, ",");
                param4 = strtok(NULL, ",");


                strncpy((char *)temp1, (char *)param3, strlen((char *)param3));
                strncpy((char *)temp2, (char *)param4, strlen((char *)param4));

                config_data[0] = (uint32_t)atoi(param2);

                config_data[1] = (uint32_t)strtol((char *)temp1, NULL, 0);
                config_data[2] = (uint32_t)strtol((char *)temp2, NULL, 0);

                //parameter error check
                if ((config_data[0] > 7) || ((int32_t)config_data[1] > 255) || ((int32_t)config_data[1] < (-256)) || ((int32_t)config_data[2] < (-256)) || ((int32_t)config_data[2] > 255)) {
                    ret_len1 =  snprintf((char *)(resonse.response_buf), ATCI_UART_TX_FIFO_BUFFER_SIZE, \
                                         "The input value AT+ECPT=SET_THR:%d,%d,%d is error!\r\n", (int)config_data[0], (int)config_data[1], (int)config_data[2]);
                    atci_captouch_send_response(&ret_len1, &resonse);
                    break;
                }

                hal_captouch_set_threshold((hal_captouch_channel_t)config_data[0], config_data[1], config_data[2]);

                ret_len1 =  snprintf((char *)(resonse.response_buf), ATCI_UART_TX_FIFO_BUFFER_SIZE, \
                                     "Captouch set threshold done! ch%d thr_h:%d,thr_l:%d!\r\n", (int)config_data[0], (int)config_data[1], (int)config_data[2]);
                atci_captouch_send_response(&ret_len1, &resonse);

                break;
            }

            /*
            description: set the fine cap value
            command: AT+ECPT=SET_FINE:[channel],[fine_cap_value]\0d\0a
            example: AT+ECPT=SET_FINE:0,-60\0d\0a        //set the channel 0 fine_cap to -60
            AT+ECPT=SET_FINE:0,1\0d\0a          //set the channel 0 fine_cap to 1
            */
            if (strncmp(param1, "AT+ECPT=SET_FINE", strlen("AT+ECPT=SET_FINE")) == 0) {
                param2 = strtok(NULL, ",");
                param3 = strtok(NULL, ",");

                strncpy((char *)temp1, (char *)param3, strlen((char *)param3));

                config_data[0] = (uint32_t)atoi(param2);

                config_data[1] = (uint32_t)strtol((char *)temp1, NULL, 0);

                //parameter error check
                if ((config_data[0] > 7) || ((int32_t)config_data[1] > 63) || ((int32_t)config_data[1] < (-64))) {
                    ret_len1 =  snprintf((char *)(resonse.response_buf), ATCI_UART_TX_FIFO_BUFFER_SIZE, \
                                         "The input value AT+ECPT=SET_FINE:%d,%d is error!\r\n", (int)config_data[0], (int)config_data[1]);
                    atci_captouch_send_response(&ret_len1, &resonse);
                    break;
                }

                hal_captouch_set_fine_cap((hal_captouch_channel_t)config_data[0], config_data[1]);

                ret_len1 =  snprintf((char *)(resonse.response_buf), ATCI_UART_TX_FIFO_BUFFER_SIZE, \
                                     "Captouch set fine_cap done! ch%d, fine_cap:%d\r\n", (int)config_data[0], (int)config_data[1]);
                atci_captouch_send_response(&ret_len1, &resonse);

                break;
            }

            /*
            description: set the coarse cap value
            command: AT+ECPT=SET_COARSE:[channel],[coarse_cap_value]\0d\0a
            example: AT+ECPT=SET_COARSE:0,3\0d\0a       //set the channel 0 coarse_cap to 3
                     AT+ECPT=SET_COARSE:A,3\0d\0a       //set the all channel coarse_cap to 3
            */
            if (strncmp(param1, "AT+ECPT=SET_COARSE", strlen("AT+ECPT=SET_COARSE")) == 0) {
                param2 = strtok(NULL, ",");
                param3 = strtok(NULL, ",");

                strncpy((char *)temp1, (char *)param3, strlen((char *)param3));

                config_data[0] = (uint32_t)atoi(param2);

                config_data[1] = (uint32_t)strtoul((char *)temp1, NULL, 0);

                if ((*(char *)param2 == 'A') || (*(char *)param2 == 'a')) {
                    for (i = 0; i < 8; i++) {
                        hal_captouch_set_coarse_cap((hal_captouch_channel_t)i, config_data[1]);
                    }

                    ret_len1 = snprintf((char *)resonse.response_buf, ATCI_UART_TX_FIFO_BUFFER_SIZE, \
                                        "Set all channel to %d\r\n", (int)config_data[1]);
                    atci_captouch_send_response(&ret_len1, &resonse);
                    break;
                }

                //parameter error check
                if ((config_data[0] > 7) || (config_data[1] > 0x7)) {
                    ret_len1 =  snprintf((char *)(resonse.response_buf), ATCI_UART_TX_FIFO_BUFFER_SIZE, \
                                         "The input value AT+ECPT=SET_COARSE:%d,%d is error!\r\n", (int)config_data[0], (int)config_data[1]);
                    atci_captouch_send_response(&ret_len1, &resonse);
                }

                hal_captouch_set_coarse_cap((hal_captouch_channel_t)config_data[0], config_data[1]);

                ret_len1 =  snprintf((char *)(resonse.response_buf), ATCI_UART_TX_FIFO_BUFFER_SIZE, \
                                     "Captouch set coarse_cap done! ch%d, coarse_cap:%d\r\n", (int)config_data[0], (int)config_data[1]);
                atci_captouch_send_response(&ret_len1, &resonse);

                break;
            }


            /*
            description: set the mavg_r and avg_s value
            command: AT+ECPT=SET_RS:[mavg_r],[mavg_s]\0d\0a
            example: AT+ECPT=SET_RS:10,5\0d\0a       //set the mavg_r to 10 and the avg_s to 5
            */
            if (strncmp(param1, "AT+ECPT=SET_RS", strlen("AT+ECPT=SET_RS")) == 0) {
                param2 = strtok(NULL, ",");
                param3 = strtok(NULL, ",");

                strncpy((char *)temp1, (char *)param3, strlen((char *)param3));

                config_data[0] = (uint32_t)atoi(param2);

                config_data[1] = (uint32_t)strtoul((char *)temp1, NULL, 0);

                nvdm_init();
                memset(&captouch_data, 0, sizeof(hal_captouch_nvdm_data));
                temp = sizeof(hal_captouch_nvdm_data);
                snprintf(nvkey, sizeof(nvkey), "%X", captouch_nveky);
                nvdm_read_data_item("AB15", nvkey, (uint8_t *)(&captouch_data), &temp);


                //parameter error check
                if ((config_data[0] > 15) || (config_data[1] > 5)) {
                    ret_len1 =  snprintf((char *)(resonse.response_buf), ATCI_UART_TX_FIFO_BUFFER_SIZE, \
                                         "The input value AT+ECPT=SET_RS:%d,%d is error!\r\n", (int)config_data[0], (int)config_data[1]);
                    atci_captouch_send_response(&ret_len1, &resonse);
                    break;
                }

                udata[0] = captouch_analog_read_data(TOUCH_CON1);
                udata[0] &= (~0x7f);
                udata[0] |= (config_data[0] << 3) | (config_data[1]);
                captouch_analog_write_data(TOUCH_CON1, udata[0]);

                captouch_data.is_rs  = true;
                captouch_data.mavg_r = config_data[0];
                captouch_data.avg_s  = config_data[1];

                snprintf(nvkey, sizeof(nvkey), "%X", captouch_nveky);
                nvdm_write_data_item("AB15", nvkey, NVDM_DATA_ITEM_TYPE_STRING, (uint8_t *)(&captouch_data), sizeof(hal_captouch_nvdm_data));

                ret_len1 =  snprintf((char *)(resonse.response_buf), ATCI_UART_TX_FIFO_BUFFER_SIZE, \
                                     "Captouch set the mavg_r:%d avg_s:% done!\r\n", (int)config_data[0], (int)config_data[1]);
                atci_captouch_send_response(&ret_len1, &resonse);

                break;
            }


            /*
            description: set the channel enabled
            command: AT+ECPT=SET_EN:[channel],[enable/disable]\0d\0a
            example: AT+ECPT=SET_EN:0,1\0d\0a         //set the channel 0 enable
                    AT+ECPT=SET_EN:0,0\0d\0a         //set the channel 0 disable
            */
            if (strncmp(param1, "AT+ECPT=SET_EN", strlen("AT+ECPT=SET_EN")) == 0) {
                param2 = strtok(NULL, ",");
                param3 = strtok(NULL, ",");

                config_data[0] = (uint32_t)atoi(param2);
                config_data[1] = (uint32_t)atoi(param3);

                //parameter error check
                if ((config_data[0] > 7) || (config_data[1] > 2)) {
                    ret_len1 =  snprintf((char *)(resonse.response_buf), ATCI_UART_TX_FIFO_BUFFER_SIZE, \
                                         "The input value AT+ECPT=SET_THR:%d,%d is error!\r\n", (int)config_data[0], (int)config_data[1]);
                    atci_captouch_send_response(&ret_len1, &resonse);
                }

                if (config_data[1]) {
                    hal_captouch_channel_enable((hal_captouch_channel_t)config_data[0]);
                } else {
                    hal_captouch_channel_disable((hal_captouch_channel_t)config_data[0]);
                }

                ret_len1 =  snprintf((char *)(resonse.response_buf), ATCI_UART_TX_FIFO_BUFFER_SIZE, \
                                     "Captouch set channel enable:%d done!\r\n", (int)config_data[1]);
                atci_captouch_send_response(&ret_len1, &resonse);

                break;
            }

#ifdef AIRO_KEY_EVENT_ENABLE
            /*
            description: set the sct key parameter
            command: AT+ECPT=SET_TIME:[short],[long1],[long2],[long3],[slong],[timeout1_2],[timeout3_8]\0d\0a
            */
#if 0
            airo_key_event_time_t airo_key_event;
            if (strncmp(param1, "AT+ECPT=SET_TIME", strlen("AT+ECPT=SET_TIME")) == 0) {

                param2 = strtok(NULL, ",");
                param3 = strtok(NULL, ",");
                param4 = strtok(NULL, ",");
                param5 = strtok(NULL, ",");
                param6 = strtok(NULL, ",");
                param7 = strtok(NULL, ",");
                param8 = strtok(NULL, ",");

                config_data[0] = (uint32_t)atoi(param2);
                config_data[1] = (uint32_t)atoi(param3);
                config_data[2] = (uint32_t)atoi(param4);
                config_data[3] = (uint32_t)atoi(param5);
                config_data[4] = (uint32_t)atoi(param6);
                config_data[5] = (uint32_t)atoi(param7);
                config_data[6] = (uint32_t)atoi(param8);


                airo_key_event.short_time        = config_data[0];
                airo_key_event.longpress_time_1  = config_data[1];
                airo_key_event.longpress_time_2  = config_data[2];
                airo_key_event.longpress_time_3  = config_data[3];
                airo_key_event.slong_repeat_time = config_data[4];
                airo_key_event.timeout_time_click[0] = config_data[5];
                airo_key_event.timeout_time_click[1] = config_data[5];
                airo_key_event.timeout_time_click[2] = config_data[6];
                airo_key_event.timeout_time_click[3] = config_data[6];
                airo_key_event.timeout_time_click[4] = config_data[6];
                airo_key_event.timeout_time_click[5] = config_data[6];
                airo_key_event.timeout_time_click[6] = config_data[6];
                airo_key_event.timeout_time_click[7] = config_data[6];
                airo_key_set_event_time(&sct_key_event);


                ret_len1 =  snprintf((char *)(resonse.response_buf), ATCI_UART_TX_FIFO_BUFFER_SIZE, \
                                     "set sct parameter done!\r\n");
                atci_captouch_send_response(&ret_len1, &resonse);

                break;
            }
#endif
#endif

#ifdef CAPTOUCH_SMT
            /*
            description: set the channel enabled
            command: AT+ECPT=SET_SMT\0d\0a
            */
            if (strncmp(param1, "AT+ECPT=SET_SMT", strlen("AT+ECPT=SET_SMT")) == 0) {

                config.callback.callback  = atci_captouch_interrupt_handler;
                config.callback.user_data = NULL;
                config.channel_bit_map = 0xff;
                for (i = 0; i < 8; i++) {
                    config.high_thr[i] = 200;
                    config.low_thr[i]  = 50;
                    config.coarse_cap[i] = 2;
                    config.mavg_r = 0xff;
                    config.avg_s  = 0xff;
                }
                hal_captouch_init(&config);
                captouch_int_control(false);

                ret_len1 =  snprintf((char *)(resonse.response_buf), ATCI_UART_TX_FIFO_BUFFER_SIZE, \
                                     "Captouch SMT test start, please touch the all key!\r\n");
                atci_captouch_send_response(&ret_len1, &resonse);

                temp  = 0;
                count = 0;
                while (1) {
                    count++;
                    for (i = 0; i < 8; i++) {
                        if ((temp & (1 << i)) == 0) {
                            vTaskDelay(10 / portTICK_PERIOD_MS);
                            captouch_get_tune_state(i, &tune_data[i]);
                            LOGMSGIDI("    CH%d press: coarse_cap=%d, fine_cap=%d,avg_adc=%d,vadc=%d\r\n", \
                                   5, (int)i, tune_data[i].coarse_cap, tune_data[i].fine_cap, tune_data[i].avg_adc, tune_data[i].vadc);
                            if (tune_data[i].vadc > 200) {
                                LOGMSGIDI("    CH%d press: coarse_cap=%d, fine_cap=%d,avg_adc=%d,vadc=%d\r\n", \
                                       5, (int)i, tune_data[i].coarse_cap, tune_data[i].fine_cap, tune_data[i].avg_adc, tune_data[i].vadc);
                                temp |= 1 << i;
                                ret_len1 =  snprintf((char *)(resonse.response_buf), ATCI_UART_TX_FIFO_BUFFER_SIZE, \
                                                     "  CH%d press: coarse_cap=%d, fine_cap=%d,avg_adc=%d,vadc=%d\r\n", \
                                                     (int)i, tune_data[i].coarse_cap, tune_data[i].fine_cap, tune_data[i].avg_adc, tune_data[i].vadc);
                                atci_captouch_send_response(&ret_len1, &resonse);
                            }
                        }

                    }
                    vTaskDelay(50 / portTICK_PERIOD_MS);

                    if (temp == 0xff) {
                        break;
                    }

                    if (count > 100) {
                        break;
                    }
                }

                hal_captouch_deinit();

                if ((temp == 0xff) && (count < 100)) {
                    ret_len1 =  snprintf((char *)(resonse.response_buf), ATCI_UART_TX_FIFO_BUFFER_SIZE, \
                                         "Captouch SMT test pass,temp=%x,count=%d!\r\n", (int)temp, (int)count);
                    atci_captouch_send_response(&ret_len1, &resonse);
                } else {
                    ret_len1 =  snprintf((char *)(resonse.response_buf), ATCI_UART_TX_FIFO_BUFFER_SIZE, \
                                         "Captouch SMT test fail,temp=%x,count=%d!\r\n", (int)temp, (int)count);
                    atci_captouch_send_response(&ret_len1, &resonse);
                }

                break;
            }
#endif

        default :
            /* others are invalid command format */
            strncpy((char *)resonse.response_buf, "Wrong command ERROR\r\n", strlen("Wrong command ERROR\r\n"));
            resonse.response_len = strlen((char *)resonse.response_buf);
            resonse.response_flag = ATCI_RESPONSE_FLAG_APPEND_ERROR;
            atci_send_response(&resonse);
            break;
    }

    snprintf((char *)resonse.response_buf, ATCI_UART_TX_FIFO_BUFFER_SIZE, \
             "/*****************next command line*********************\r\n");
    resonse.response_len = strlen((char *)resonse.response_buf);
    resonse.response_flag = ATCI_RESPONSE_FLAG_URC_FORMAT;
    atci_send_response(&resonse);
    return ATCI_STATUS_OK;
}

#endif

