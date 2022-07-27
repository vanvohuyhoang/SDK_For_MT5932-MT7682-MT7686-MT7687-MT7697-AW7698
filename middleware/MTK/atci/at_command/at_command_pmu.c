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
#include "hal_feature_config.h"
#ifdef HAL_PMU_MODULE_ENABLED
#include <string.h>
#include <stdio.h>
#include <ctype.h>
#include "at_command.h"
#include "hal.h"
#include "hal_pmu.h"
#if (PRODUCT_VERSION == 1552)
#include "hal_pmu_mt6388_platform.h"
#include "hal_pmu_auxadc.h"
#endif
#define STRNCPY_PMU(dest, source) strncpy(dest, source, strlen(source)+1);
//--
#if (PRODUCT_VERSION == 1552)
void pmu_pk_callback(void) {
    log_hal_msgid_info("[pmu_pk_callback]\r\n",0);
}
#endif
//--
/*--- Function ---*/
atci_status_t atci_cmd_hdlr_pmu(atci_parse_cmd_param_t *parse_cmd);
static int htoi(char s[]);
/*
 AT+EPMUREG=<op>                |   "OK"
 AT+EPMUREG=?                |   "+EPMUREG=(0,1)","OK"


 */
// AT command handler
atci_status_t atci_cmd_hdlr_pmu(atci_parse_cmd_param_t *parse_cmd) {
    atci_response_t response = { { 0 } };
    uint16_t read_value = 0;
    int input_addr = 0;
    int input_value = 0;
    char *end_pos = NULL;
    response.response_flag = 0; // Command Execute Finish.
#ifdef ATCI_APB_PROXY_ADAPTER_ENABLE
            response.cmd_id = parse_cmd->cmd_id;
#endif
    switch (parse_cmd->mode) {
    case ATCI_CMD_MODE_TESTING:    // rec: AT+EPMUREG=?
        STRNCPY_PMU((char *) response.response_buf, "+EPMUREG=(0,1)\r\nOK\r\n");

        response.response_len = strlen((const char *) response.response_buf);
        atci_send_response(&response);
        break;
    case ATCI_CMD_MODE_EXECUTION: // rec: AT+EPMUREG=<op>  the handler need to parse the parameters

        if (strncmp(parse_cmd->string_ptr, "AT+EPMUREG=0,", strlen("AT+EPMUREG=0,")) == 0) {
            /*command: AT+EPMUREG=0,1234*/
            end_pos = strchr(parse_cmd->string_ptr, ',');
            end_pos++;
            input_addr = htoi(end_pos);
            end_pos = NULL;

            /* read data of input register address */
#if (PRODUCT_VERSION == 1552)
            log_hal_msgid_info("[PMU] read register address:0x%x", 1, input_addr);
            read_value = pmu_get_register_value_2byte_mt6388(input_addr, 0xFFFF, 0);
#else
            read_value = pmu_get_register_value(input_addr, 0xFF, 0);
#endif
            snprintf((char *) response.response_buf, ATCI_UART_TX_FIFO_BUFFER_SIZE, "+EPMUREG:0x%x,0x%x\r\n", input_addr, read_value);
            /* ATCI will help append "OK" at the end of response buffer  */
            response.response_flag |= ATCI_RESPONSE_FLAG_APPEND_OK;
            response.response_len = strlen((const char *) response.response_buf);
        } else if (strncmp(parse_cmd->string_ptr, "AT+EPMUREG=1,", strlen("AT+EPMUREG=1,")) == 0) {
            /*command: AT+EPMUREG=1,1234,456*/
            char *mid_pos = NULL;
            char str[20] = { 0 };
            mid_pos = strchr(parse_cmd->string_ptr, ',');
            mid_pos++;
            end_pos = strchr(mid_pos, ',');
            memcpy(str, mid_pos, strlen(mid_pos) - strlen(end_pos));
            input_addr = htoi(mid_pos);
            end_pos++;
            input_value = htoi(end_pos);
            mid_pos = NULL;
            end_pos = NULL;

            /* write input data to input register address*/
#if (PRODUCT_VERSION == 1552)
            log_hal_msgid_info("register address:0x%x, set register value:0x%x\r\n", 2, input_addr, input_value);
            pmu_set_register_value_2byte_mt6388(input_addr, 0xFFFF, 0, input_value);
#else
            read_value = pmu_get_register_value(input_addr, 0xFF, 0);
#endif
                /* ATCI will help append "OK" at the end of response buffer */
            response.response_flag |= ATCI_RESPONSE_FLAG_APPEND_OK;
            response.response_len = strlen((const char *) response.response_buf);

        }
#if (PRODUCT_VERSION == 1552)
        else if (strncmp(parse_cmd->string_ptr, "AT+EPMUREG=PWR,", strlen("AT+EPMUREG=PWR,")) == 0) {
            end_pos = strchr(parse_cmd->string_ptr, ',');
            end_pos++;
            input_addr = htoi(end_pos);
            end_pos = NULL;
            if (input_addr == 0) {
                pmu_power_off_sequence(PMU_PWROFF);
            } else {
                pmu_power_off_sequence(PMU_RTC);
            }
            /* ATCI will help append "OK" at the end of response buffer  */
            response.response_flag |= ATCI_RESPONSE_FLAG_APPEND_OK;
            response.response_len = strlen((const char *) response.response_buf);
        } else if (strncmp(parse_cmd->string_ptr, "AT+EPMUREG=AUX,", strlen("AT+EPMUREG=AUX,")) == 0) {
            /*command: AT+EPMUREG=0,1234*/
            end_pos = strchr(parse_cmd->string_ptr, ',');
            end_pos++;
            input_addr = htoi(end_pos);
            end_pos = NULL;
            log_hal_msgid_info("Input channel : %d\r\n", 1, input_addr);
            log_hal_msgid_info("Index :%d : %lx",2,input_addr,pmu_auxadc_get_channel_value(input_addr));
            /* ATCI will help append "OK" at the end of response buffer  */
            response.response_flag |= ATCI_RESPONSE_FLAG_APPEND_OK;
            response.response_len = strlen((const char *) response.response_buf);
        } else if (strncmp(parse_cmd->string_ptr, "AT+EPMUREG=PWRKEY,", strlen("AT+EPMUREG=PWRKEY,")) == 0) {
            /*command: AT+EPMUREG=0,1234*/
            char *mid_pos = NULL;
            char str[20] = {0};
            int input_case = 0;
            int input_value = 0;
            mid_pos = strchr(parse_cmd->string_ptr, ',');
            mid_pos++;
            end_pos = strchr(mid_pos, ',');
            memcpy(str, mid_pos, strlen(mid_pos) - strlen(end_pos));
            input_case = htoi(mid_pos);
            end_pos++;
            input_value = htoi(end_pos);
            mid_pos = NULL;
            end_pos = NULL;
            log_hal_msgid_info("input_case:0x%x, input_value:0x%x\r\n",2, input_case, input_value);
            switch (input_case){
                case 0:
                    pmu_pwrkey_enable(input_value);
                    break;
                case 1:
                    pmu_pwrkey_duration_time(input_value);
                    break;
                case 2:
                    pmu_long_press_shutdown_function_sel(input_value);
                    break;
            }
            /* ATCI will help append "OK" at the end of response buffer  */
            response.response_flag |= ATCI_RESPONSE_FLAG_APPEND_OK;
            response.response_len = strlen((const char *) response.response_buf);
        }else if (strncmp(parse_cmd->string_ptr, "AT+EPMUREG=VOLT,", strlen("AT+EPMUREG=VOLT,")) == 0) {
            char *mid_pos = NULL;
            char str[20] = { 0 };
            int input_domain = 0;
            int input_value = 0;
            mid_pos = strchr(parse_cmd->string_ptr, ',');
            mid_pos++;
            end_pos = strchr(mid_pos, ',');
            memcpy(str, mid_pos, strlen(mid_pos) - strlen(end_pos));
            input_domain = htoi(mid_pos);
            end_pos++;
            input_value = htoi(end_pos);
            mid_pos = NULL;
            end_pos = NULL;
            switch (input_domain) {
            case 0: //VCORE
                pmu_set_register_value_mt6388(PMU_RG_BUCK_VCORE_VOSEL_ADDR, PMU_RG_BUCK_VCORE_VOSEL_MASK, PMU_RG_BUCK_VCORE_VOSEL_SHIFT, input_value);
                break;
            case 1: //VIO18
                pmu_voltage_selet_6388(PMU_NORMAL, PMU_BUCK_VIO18, input_value);
                break;
            case 2: //VAUD18
                pmu_vaud18_voltage_sel_6388(input_value);
                break;
            case 3: //VRF
                pmu_voltage_selet_6388(PMU_NORMAL, PMU_BUCK_VRF, input_value);
                break;
            }
            /* ATCI will help append "OK" at the end of response buffer  */
            response.response_flag |= ATCI_RESPONSE_FLAG_APPEND_OK;
            response.response_len = strlen((const char *) response.response_buf);
        } else if (strncmp(parse_cmd->string_ptr, "AT+EPMUREG=DEBUG,", strlen("AT+EPMUREG=DEBUG,")) == 0) {
            end_pos = strchr(parse_cmd->string_ptr, ',');
            end_pos++;
            input_addr = htoi(end_pos);
                end_pos = NULL;
                switch(input_addr) {
                    case 0:
                    log_hal_msgid_info("===Interrupt setting===\r\n",0);
                    log_hal_msgid_info("PMU INT MASK : COM0 :%lx\r\n",1,pmu_get_register_value_mt6388(PMU_INT_MASK_CON0, 0xffff, 0));
                    log_hal_msgid_info("PMU INT MASK : COM1 :%lx\r\n",1,pmu_get_register_value_mt6388(PMU_INT_MASK_CON1, 0xffff, 0));
                    log_hal_msgid_info("PMU INT MASK : COM2 :%lx\r\n",1,pmu_get_register_value_mt6388(PMU_INT_MASK_CON2, 0xffff, 0));
                    log_hal_msgid_info("PMU INT MASK : COM3 :%lx\r\n",1,pmu_get_register_value_mt6388(PMU_INT_MASK_CON3, 0xffff, 0));

                    log_hal_msgid_info("PMU INT status : COM0 :%lx\r\n",1,pmu_get_register_value_mt6388(PMU_INT_STATUS0, 0xffff, 0));
                    log_hal_msgid_info("PMU INT status : COM1 :%lx\r\n",1,pmu_get_register_value_mt6388(PMU_INT_STATUS1, 0xffff, 0));
                    log_hal_msgid_info("PMU INT status : COM2 :%lx\r\n",1,pmu_get_register_value_mt6388(PMU_INT_STATUS2, 0xffff, 0));
                    log_hal_msgid_info("PMU INT status : COM3 :%lx\r\n",1,pmu_get_register_value_mt6388(PMU_INT_STATUS3, 0xffff, 0));

                    log_hal_msgid_info("PMU INT enable : COM0 :%lx\r\n",1,pmu_get_register_value_mt6388(PMU_INT_CON0, 0xffff, 0));
                    log_hal_msgid_info("PMU INT enable : COM1 :%lx\r\n",1,pmu_get_register_value_mt6388(PMU_INT_CON1, 0xffff, 0));
                    log_hal_msgid_info("PMU INT enable : COM2 :%lx\r\n",1,pmu_get_register_value_mt6388(PMU_INT_CON2, 0xffff, 0));
                    log_hal_msgid_info("PMU INT enable : COM3 :%lx\r\n",1,pmu_get_register_value_mt6388(PMU_INT_CON3, 0xffff, 0));

                    log_hal_msgid_info("===VCORE Setting===\r\n",0);
                    log_hal_msgid_info("Normal : %lx\r\n",1,pmu_get_register_value_mt6388(PMU_RG_BUCK_VCORE_VOSEL_ADDR, PMU_RG_BUCK_VCORE_VOSEL_MASK, PMU_RG_BUCK_VCORE_VOSEL_SHIFT));
                    log_hal_msgid_info("00:0.7V ; 01:0.9V\r\n",0);
                    log_hal_msgid_info("Sleep voltage : %lx\r\n",1,pmu_get_register_value_mt6388(PMU_RG_VCORE_SLEEP_VOLTAGE_ADDR, PMU_RG_VCORE_SLEEP_VOLTAGE_MASK, PMU_RG_VCORE_SLEEP_VOLTAGE_SHIFT));
                    log_hal_msgid_info("Sleep vosel : %lx\r\n",1,pmu_get_register_value_mt6388(PMU_RG_BUCK_VCORE_VOSEL_SLEEP_ADDR, PMU_RG_BUCK_VCORE_VOSEL_SLEEP_MASK, PMU_RG_BUCK_VCORE_VOSEL_SLEEP_SHIFT));

                    log_hal_msgid_info("===VAUD18 Setting===\r\n",0);
                    log_hal_msgid_info("Normal : %lx\r\n",1,pmu_get_register_value_mt6388(PMU_RG_BUCK_VAUD18_VOSEL_ADDR, PMU_RG_BUCK_VAUD18_VOSEL_MASK, PMU_RG_BUCK_VAUD18_VOSEL_SHIFT));
                    log_hal_msgid_info("00: 0.8V ; 01: 0.9V ; 10: 0.85V\r\n",0);
                    log_hal_msgid_info("Sleep voltag : %lx\r\n",1,pmu_get_register_value_mt6388(PMU_RG_VAUD18_SLEEP_VOLTAGE_ADDR, PMU_RG_VAUD18_SLEEP_VOLTAGE_MASK, PMU_RG_VAUD18_SLEEP_VOLTAGE_SHIFT));

                    log_hal_msgid_info("===PMU_BUCK_VIO18===\r\n",0);
                    log_hal_msgid_info("Normal : %lx\r\n",1,pmu_get_register_value_mt6388(PMU_RG_BUCK_VIO18_VOSEL_ADDR, PMU_RG_BUCK_VIO18_VOSEL_MASK, PMU_RG_BUCK_VIO18_VOSEL_SHIFT));

                    log_hal_msgid_info("===PMU_BUCK_VRF===\r\n",0);
                    log_hal_msgid_info("Normal : %lx\r\n",1,pmu_get_register_value_mt6388(PMU_RG_BUCK_VRF_VOSEL_ADDR, PMU_RG_BUCK_VRF_VOSEL_MASK, PMU_RG_BUCK_VRF_VOSEL_SHIFT));
                    log_hal_msgid_info("Sleep : %lx\r\n",1,pmu_get_register_value_mt6388(PMU_RG_BUCK_VRF_VOSEL_SLEEP_ADDR, PMU_RG_BUCK_VRF_VOSEL_SLEEP_MASK, PMU_RG_BUCK_VRF_VOSEL_SLEEP_SHIFT));

                    /*
                     * Normal Vout = vref_lVCORE+6.25mV*X, vref_lVCORE=0.7V
                     * Sleep Vout : RG_VCORE_SLEEP_VOLTAGE  sleep voltage setting
                     * 00: 0.7V
                     * 01: 0.9V
                     *
                     * Normal Vout = vref_lVAUD18+9.375mV*X, vref_lVAUD18=1.05V
                     * Sleep Vout : RG_VAUD18_SLEEP_VOLTAGE  sleep voltage setting:
                     * 00: 0.8V
                     * 01: 0.9V
                     * 10: 0.85V
                     */
                    break;

                    case 1:
                        pmu_register_callback(RG_INT_PWRKEY, pmu_pk_callback, NULL);
                    break;
                }
            /* ATCI will help append "OK" at the end of response buffer  */
            response.response_flag |= ATCI_RESPONSE_FLAG_APPEND_OK;
            response.response_len = strlen((const char *) response.response_buf);
        }
#endif
        else {
            /*invalide parameter, return "ERROR"*/
            STRNCPY_PMU((char *) response.response_buf, "ERROR\r\n");
            response.response_len = strlen((const char *) response.response_buf);
        }
        ;

        atci_send_response(&response);

        break;
    default:
        /* others are invalid command format */
        STRNCPY_PMU((char *) response.response_buf, "ERROR\r\n");
        response.response_len = strlen((const char *) response.response_buf);
        atci_send_response(&response);
        break;
    }
    return ATCI_STATUS_OK;
}

// change the data with hex type in string to data with dec type
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

#endif

