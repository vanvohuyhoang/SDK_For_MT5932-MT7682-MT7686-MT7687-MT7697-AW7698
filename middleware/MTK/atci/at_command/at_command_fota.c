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

#ifdef MTK_FOTA_ENABLE

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <inttypes.h>

/* FreeRTOS */
#include "FreeRTOS.h"
#include "task.h"

/* Other modules */
#include "toi.h"
#include "syslog.h"
#include "task_def.h"
#include "httpclient.h"
#include "hal_wdt.h"
#include "hal_flash.h"

#include "at_command.h"
//#include "at_command_fota.h"
#include "fota.h"

#ifndef MTK_FOTA_ON_7687
#include "memory_map.h"
#else
#include "flash_map.h"
#endif

/****************************************************************************
 * Macros
 ****************************************************************************/
#define FLASH_BLOCK_SIZE        (1 << 12)
#define FOTA_BUF_SIZE    (1024 * 4 + 1)
#define FOTA_URL_BUF_LEN    (256)

#define FOTA_DEBUG_SYSLOG
//#define FOTA_DEBUG_PRINT

log_create_module(fota_atci, PRINT_LEVEL_INFO);

#ifdef FOTA_DEBUG_SYSLOG
#define LOGE(fmt,arg...)   LOG_E(fota_atci, fmt,##arg)
#define LOGW(fmt,arg...)   LOG_W(fota_atci, fmt,##arg)
#define LOGI(fmt,arg...)   LOG_I(fota_atci, fmt,##arg)

#define LOGMSGIDE(fmt,cnt,arg...)   LOG_MSGID_E(fota_atci ,fmt,cnt,##arg)
#define LOGMSGIDW(fmt,cnt,arg...)   LOG_MSGID_W(fota_atci ,fmt,cnt,##arg)
#define LOGMSGIDI(fmt,cnt,arg...)   LOG_MSGID_I(fota_atci ,fmt,cnt,##arg)

#else
#define LOGE(fmt,arg...)   printf(fmt,##arg)
#define LOGW(fmt,arg...)   printf(fmt,##arg)
#define LOGI(fmt,arg...)  

#endif

#ifdef FOTA_DEBUG_PRINT
#define DEBUG_PRINT(fmt, arg...)    printf(fmt,##arg)
#else
#define DEBUG_PRINT(fmt, arg...)
#endif


typedef struct {
    uint32_t start_address;
    uint32_t end_address;
    uint32_t write_ptr;
    uint32_t reserved_size;
    uint32_t total_received;
    uint32_t block_count;
    uint32_t block_size;
    hal_flash_block_t block_type;
} fota_buffer_info_t;


/****************************************************************************
 * Static variables
 ****************************************************************************/
static httpclient_t _s_fota_httpclient = {0};
static fota_buffer_info_t s_fota_buffer;


/*--- Function ---*/
atci_status_t atci_cmd_hdlr_fota_cmd(atci_parse_cmd_param_t *parse_cmd);


static void fota_init_fota_buffer()
{
#ifdef MTK_FOTA_ON_7687
    s_fota_buffer.start_address = FOTA_BASE;
    s_fota_buffer.end_address = FOTA_BASE + FOTA_LENGTH;
    s_fota_buffer.reserved_size = FOTA_LENGTH;
    s_fota_buffer.block_count = FOTA_LENGTH >> 12;
#else
#ifndef ROM_BASE
    s_fota_buffer.start_address = FOTA_RESERVED_BASE - BL_BASE;
    s_fota_buffer.end_address = FOTA_RESERVED_BASE + FOTA_RESERVED_LENGTH - BL_BASE;
#else
    s_fota_buffer.start_address = FOTA_RESERVED_BASE - ROM_BASE;
    s_fota_buffer.end_address = FOTA_RESERVED_BASE + FOTA_RESERVED_LENGTH - ROM_BASE;
#endif
    s_fota_buffer.reserved_size = FOTA_RESERVED_LENGTH;
    s_fota_buffer.block_count = FOTA_RESERVED_LENGTH >> 12;
#endif
    s_fota_buffer.block_size = FLASH_BLOCK_SIZE;
    s_fota_buffer.block_type = HAL_FLASH_BLOCK_4K;
    s_fota_buffer.total_received = 0;
    s_fota_buffer.write_ptr = s_fota_buffer.start_address;
}


static int32_t _fota_http_retrieve_get(char* get_url, char* buf, uint32_t len)
{
    int32_t ret = HTTPCLIENT_ERROR_CONN;
    hal_flash_status_t write_ret;
    httpclient_data_t client_data = {0};
    uint32_t count = 0;
    uint32_t recv_temp = 0;
    uint32_t data_len = 0;

    client_data.response_buf = buf;
    client_data.response_buf_len = len;

    LOGMSGIDI("[FOTA DL] fota_http_retrieve_get enter.\n", 0);
    ret = httpclient_send_request(&_s_fota_httpclient, get_url, HTTPCLIENT_GET, &client_data);

    LOGMSGIDI("[FOTA DL] httpclient send request ret = %d\n",1, ret);
    if (ret < 0) {
        
        LOGMSGIDE("[FOTA DL] http client fail to send request.\n", 0);
        return ret;
    }

    do {
        LOGMSGIDI("[FOTA DL] recv_reponse loop.\n", 0);
        
        ret = httpclient_recv_response(&_s_fota_httpclient, &client_data);
        if (ret < 0) {
            LOGMSGIDE("[FOTA DL] error!!! recve response ret = %d",1, ret);
            return ret;
        }

        if (recv_temp == 0)
        {
            recv_temp = client_data.response_content_len;
        }

        LOGMSGIDI("[FOTA DL] to be retrieved len: %d \n",1, client_data.retrieve_len);
        
        data_len = recv_temp - client_data.retrieve_len;
        LOGMSGIDI("[FOTA DL] current pack data len: %u \n",1, data_len);
        
        count += data_len;
        recv_temp = client_data.retrieve_len;
        //vTaskDelay(100);/* Print log may block other task, so sleep some ticks */
        LOGMSGIDI("[FOTA DL] total data received: %u\n",1, count);

        if (s_fota_buffer.write_ptr >= s_fota_buffer.start_address &&
            s_fota_buffer.write_ptr < s_fota_buffer.end_address) {
            if (!(s_fota_buffer.write_ptr % (1 << 12))) {
                DEBUG_PRINT("[FOTA DL] erase flash addr = 0x%x \n", s_fota_buffer.write_ptr);
                write_ret = hal_flash_erase(s_fota_buffer.write_ptr, s_fota_buffer.block_size);
                LOGMSGIDI("\n[FOTA DL] erase flash, ret = %d, address = 0x%x\n",2, write_ret, s_fota_buffer.write_ptr);
            }
            DEBUG_PRINT("[FOTA DL] write flash addr = 0x%x \n", s_fota_buffer.write_ptr);
            write_ret = hal_flash_write(s_fota_buffer.write_ptr, (const uint8_t*)client_data.response_buf, data_len);
            
            if (HAL_FLASH_STATUS_OK == write_ret) {
                s_fota_buffer.write_ptr += data_len;
                DEBUG_PRINT("[FOTA DL]write data len = %d", data_len);
            }else {
                LOGMSGIDE("[FOTA DL]fail to write flash, write_ret = %d\n",1, write_ret);
                return ret;
            }
        } else {
            LOGMSGIDE("[FOTA DL] out of partition size\n", 0);
            return -3;
        }
           

        LOGMSGIDI("[FOTA DL] download progrses = %u\n",1, count * 100 / client_data.response_content_len);
        
    } while (ret == HTTPCLIENT_RETRIEVE_MORE_DATA);

    LOGMSGIDI("[FOTA DL] total length: %d\n",1, client_data.response_content_len);
    if (count != client_data.response_content_len || httpclient_get_response_code(&_s_fota_httpclient) != 200) {
        LOGMSGIDE("[FOTA DL]data received not completed, or invalid error code\r\n", 0);
        
        return -1;
    }else if (count == 0) {
        LOGMSGIDE("[FOTA DL]receive data length is zero, file not found\r\n", 0);
        return -2;
    }else {
        LOGMSGIDI("[FOTA DL]download success\n", 0);
        return ret;
    }
    
}


static int8_t fota_download_by_http(char *param)
{
    char get_url[FOTA_URL_BUF_LEN];
    int32_t ret = HTTPCLIENT_ERROR_CONN;
    uint32_t len_param = strlen(param);

    if (len_param < 1) {
      return -1;
    }
    memset(get_url, 0, FOTA_URL_BUF_LEN);
    LOGMSGIDI("length: %d\n",1, strlen(param));
    strcpy(get_url, param);

    fota_init_fota_buffer();
    

    char* buf = pvPortMalloc(FOTA_BUF_SIZE);
    if (buf == NULL) {
        LOGMSGIDE("buf malloc failed.\r\n", 0);
        return -3;
    }
    ret = httpclient_connect(&_s_fota_httpclient, get_url);
    if (!ret) {
        ret = _fota_http_retrieve_get(get_url, buf, FOTA_BUF_SIZE);
    }else {
        LOGMSGIDE("[FOTA DL] http client connect error. \r", 0);
    }
    LOGMSGIDI("Download result = %d \r\n",1, (int)ret);

    httpclient_close(&_s_fota_httpclient);

    vPortFree(buf);
    buf = NULL;

    //Delay to make key trace output .
    vTaskDelay(500);
    if ( 0 == ret) {
        fota_trigger_update();
        fota_ret_t err;
        err = fota_trigger_update();
        if (0 == err ){
            //reboot device

        #if 0 // TODO: how to reboot in 7686 platform
            hal_wdt_config_t wdt_config;
            wdt_config.mode = HAL_WDT_MODE_RESET;
            wdt_config.seconds = 1;
            hal_wdt_init(&wdt_config);
            hal_wdt_software_reset();
        #endif
        
            LOGMSGIDI("Reboot device!", 0);
            return 0;
        } else {
            LOGMSGIDE("Trigger FOTA error!", 0);
            return -1;
        }
    } else {
        return -1;
    }
}


static void fota_download_task(void *param)
{
    int8_t ret;
    
    LOGI("[FOTA DL] fota download task, param_addr = 0x%x, param = %s\r\n", (uint32_t)param, param);
    ret = fota_download_by_http(param);
    LOGMSGIDI("[FOTA DL] exit download function!, ret = %d\r\n",1, ret);
    vPortFree(param);
    vTaskDelete(NULL);
}


/* AT command handler  */
atci_status_t atci_cmd_hdlr_fota_cmd(atci_parse_cmd_param_t *parse_cmd)
{
    static int test_param1 = 0;
    atci_response_t resonse = {{0}};
    char *param = NULL;
    int  param1_val = -1;
    //uint8_t ret;  //Unused value

    LOGMSGIDW("atci_cmd_hdlr_fota_cmd\n", 0);

    resonse.response_flag = 0; // Command Execute Finish.
    #ifdef ATCI_APB_PROXY_ADAPTER_ENABLE
    resonse.cmd_id = parse_cmd->cmd_id;
    #endif

    switch (parse_cmd->mode) {
        case ATCI_CMD_MODE_TESTING:    // rec: AT+FOTA=?
            LOGMSGIDW("AT FOTA OK.\n", 0);
            strcpy((char *)resonse.response_buf, "+FOTA:(0,1)\r\nOK\r\n");
            break;
            
       case ATCI_CMD_MODE_READ:    // rec: AT+FOTA?
            LOGMSGIDW("AT Read done.\n", 0);
            sprintf((char *)resonse.response_buf,"+FOTA:%d\r\n", test_param1);
            resonse.response_len = strlen((char *)resonse.response_buf);
            break;

        case ATCI_CMD_MODE_ACTIVE:  // rec: AT+FOTA
            LOGMSGIDW("AT Active OK.\n", 0);
            // assume the active mode is invalid and we will return "ERROR"
            strcpy((char *)resonse.response_buf, "ERROR\r\n");
            resonse.response_len = strlen((char *)resonse.response_buf);
            atci_send_response(&resonse);
            break;

        case ATCI_CMD_MODE_EXECUTION: // rec: AT+FOTA=<p1>  the handler need to parse the parameters
            LOGMSGIDW("AT Executing...\r\n", 0);
            //parsing the parameter
            param = strtok(parse_cmd->string_ptr, ",\n\r");
            param = strtok(parse_cmd->string_ptr, "AT+FOTA=");
            param1_val = atoi(param);

            if (param != NULL && (param1_val == 0 || param1_val == 1)){

                // valid parameter, update the data and return "OK"
                char *url = NULL;
                url = pvPortMalloc(FOTA_URL_BUF_LEN);
                if (url != NULL) {
                    strncpy(url, param, FOTA_URL_BUF_LEN);
                    LOG_I(fota_atci, "[FOTA AT] url = %s\r\n", url);
                    xTaskCreate(fota_download_task, FOTA_DOWNLOAD_TASK_NAME, FOTA_DOWNLOAD_TASK_STACKSIZE / (( uint32_t )sizeof( StackType_t )), url, FOTA_DOWNLOAD_TASK_PRIO, NULL);
                    LOG_MSGID_I(fota_atci, "[FOTA AT] create fota download task \r\n", 0);

                    resonse.response_len = 0;
                    resonse.response_flag |= ATCI_RESPONSE_FLAG_APPEND_OK; // ATCI will help append "OK" at the end of resonse buffer
                } else {
                    LOG_MSGID_I(fota_atci, "[FOTA AT] alloc buffer failed.\r\n", 0);
                    resonse.response_len = 0;
                    resonse.response_flag |= ATCI_RESPONSE_FLAG_APPEND_ERROR; // ATCI will help append "ERROR" at the end of resonse buffer
                }
            } else {
                // invalide parameter, return "ERROR"
                resonse.response_len = 0;
                resonse.response_flag |= ATCI_RESPONSE_FLAG_APPEND_ERROR; // ATCI will help append "ERROR" at the end of resonse buffer
            };
            atci_send_response(&resonse);
            param = NULL;
            break;

        default :
            strcpy((char *)resonse.response_buf, "ERROR\r\n");
            resonse.response_len = strlen((char *)resonse.response_buf);
            atci_send_response(&resonse);
            break;
    }
    return ATCI_STATUS_OK;
}


#endif /* MTK_FOTA_ENABLE */

