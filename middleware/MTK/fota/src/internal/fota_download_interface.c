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

#ifndef MOD_CFG_FOTA_DISABLE_OS

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <inttypes.h>

/* FreeRTOS */
#include "FreeRTOS.h"
#include "task.h"

/* Other modules */
#include "toi.h"

#include "httpclient.h"
#include "tftpc.h"

#include "fota_platform.h"
#include "fota_download_interface.h"
#include "fota.h"
#include "fota_config.h"
#include "fota_internal.h"
#include "hal_sys.h"


/****************************************************************************
 * Macros
 ****************************************************************************/
#define FOTA_BUF_SIZE    (1024 * 4 + 1)
#define FOTA_URL_BUF_LEN    (256)


/****************************************************************************
 * Static variables
 ****************************************************************************/
httpclient_t _s_fota_httpclient = {0};

#ifdef ANDLINK_ENABLE
fota_progress_update_cb g_fota_progress_update_cb = NULL;


void fota_download_register_progress_update_cb(fota_progress_update_cb progress_update_cb)
{
    if (progress_update_cb != NULL) {
        g_fota_progress_update_cb = progress_update_cb;
    }
}
#endif

static int32_t _fota_http_retrieve_get(char* get_url, char* buf, uint32_t len)
{
    int32_t ret = HTTPCLIENT_ERROR_CONN;
    fota_status_t write_ret;
    httpclient_data_t client_data = {0};
    uint32_t count = 0;
    uint32_t recv_temp = 0;
    uint32_t data_len = 0;

    client_data.response_buf = buf;
    client_data.response_buf_len = len;

    ret = httpclient_send_request(&_s_fota_httpclient, get_url, HTTPCLIENT_GET, &client_data);
    if (ret < 0) {
        
        FOTA_LOG_MSGID_E("[FOTA DL] http client fail to send request", 0);
        return ret;
    }

    do {
        ret = httpclient_recv_response(&_s_fota_httpclient, &client_data);
        if (ret < 0) {
            FOTA_LOG_MSGID_E("[FOTA DL] http client recv response error, ret = %d",1, ret);
            return ret;
        }

        if (recv_temp == 0)
        {
            recv_temp = client_data.response_content_len;
        }

        FOTA_LOG_MSGID_I("[FOTA DL] retrieve_len = %d",1, client_data.retrieve_len);
        
        data_len = recv_temp - client_data.retrieve_len;
        FOTA_LOG_MSGID_I("[FOTA DL] data_len = %u",1, data_len);
        
        count += data_len;
        recv_temp = client_data.retrieve_len;
        
        //vTaskDelay(100);/* Print log may block other task, so sleep some ticks */
        FOTA_LOG_MSGID_I("[FOTA DL] total data received %u",1, count);

        write_ret = fota_write(FOTA_PARITION_TMP, (const uint8_t*)client_data.response_buf, data_len);
        if (FOTA_STATUS_OK != write_ret) {
            FOTA_LOG_MSGID_E("[FOTA DL] fail to write flash, write_ret = %d",1, write_ret);
            return ret;
        }

        FOTA_LOG_MSGID_I("[FOTA DL] download progrses = %u",1, count * 100 / client_data.response_content_len);

#ifdef ANDLINK_ENABLE
        if (g_fota_progress_update_cb != NULL) {
            g_fota_progress_update_cb((count * 100) / client_data.response_content_len);
        }
#endif

    } while (ret == HTTPCLIENT_RETRIEVE_MORE_DATA);

    FOTA_LOG_MSGID_I("[FOTA DL] total length: %d",1, client_data.response_content_len);
    if (count != client_data.response_content_len || httpclient_get_response_code(&_s_fota_httpclient) != 200) {
        FOTA_LOG_MSGID_E("[FOTA DL] data received not completed, or invalid error code", 0);
        return -1;
    }
    else if (count == 0) {
        FOTA_LOG_MSGID_E("[FOTA DL] receive length is zero, file not found", 0);
        return -2;
    }
    else {
        FOTA_LOG_MSGID_I("[FOTA DL] download success", 0);
        return ret;
    }
        

}

int8_t fota_download_by_http(char *param)
{
    char get_url[FOTA_URL_BUF_LEN];
    int32_t ret = HTTPCLIENT_ERROR_CONN;
    uint32_t len_param = strlen(param);

    if (len_param < 1) {
      return -1;
    }
    memset(get_url, 0, FOTA_URL_BUF_LEN);
    FOTA_LOG_MSGID_I("[FOTA DL] url length: %d",1, strlen(param));
    strcpy(get_url, param);
    if (fota_init(&fota_flash_default_config) != FOTA_STATUS_OK) {
        FOTA_LOG_MSGID_E("[FOTA DL] fota init fail.", 0);
        return -2;
    }
    if (fota_seek(FOTA_PARITION_TMP, 0) != FOTA_STATUS_OK) {
        FOTA_LOG_MSGID_E("[FOTA DL] fota seek partition fail.", 0);
        return -2;
    }
    char* buf = pvPortMalloc(FOTA_BUF_SIZE);
    if (buf == NULL) {
        FOTA_LOG_MSGID_E("[FOTA DL] buf malloc failed.", 0);
        return -3;
    }
    ret = httpclient_connect(&_s_fota_httpclient, get_url);
    if (!ret) {
        ret = _fota_http_retrieve_get(get_url, buf, FOTA_BUF_SIZE);
    }else {
        FOTA_LOG_MSGID_E("[FOTA DL] http client connect error.", 0);
    }
    FOTA_LOG_MSGID_I("[FOTA DL] Download result = %d",1, (int)ret);

    httpclient_close(&_s_fota_httpclient);

    vPortFree(buf);
    buf = NULL;
    if ( HTTPCLIENT_OK == ret) {
        return 0;
    } else {
        return -1;
    }
}


ssize_t fota_download_by_tftp(char *address, char *filename, bool writing,
                              uint32_t partition)
{
    tftpc_t         *tftpc;
    ssize_t         total_len = 0;
    tftpc_status_t  status;
    uint16_t        pkt_len;

    tftpc = tftpc_read_init(address, TFTP_PORT, 21000, filename);

    if (tftpc == NULL) {
        FOTA_LOG_MSGID_I("[FOTA DL] tftpc init failed", 0);
        return -1;
    }

    if (writing) {
        if (fota_init(&fota_flash_default_config) != FOTA_STATUS_OK) {
            return -2;
        }

        if (fota_seek(partition, 0) != FOTA_STATUS_OK) {
            return -3;
        }
    }

    do {
        status = tftpc_read_data(tftpc, &pkt_len);

        if (status == TFTPC_STATUS_MORE || status == TFTPC_STATUS_DONE) {
            if (pkt_len != 0) {
                pkt_len     -= 4;
                total_len   += pkt_len;

                if (writing) {
                    if (fota_write(partition, &tftpc->buf[0] + 4, pkt_len) != FOTA_STATUS_OK) {
                        total_len = -4;
                        break;
                    }
                }
            }
        }
        else
        {
            FOTA_LOG_MSGID_E("[FOTA DL] update bin download fail! status: %d",1, status);
        }
    } while (status == TFTPC_STATUS_MORE);

    tftpc_read_done(tftpc);

    if (total_len < 0) {
        FOTA_LOG_MSGID_E("[FOTA DL] download error", 0);
    } else if (total_len > 0) {
        FOTA_LOG_MSGID_I("[FOTA DL] total_len %d",1, total_len); // todo: remove me.
    }

    return total_len;
}

#endif /* MOD_CFG_FOTA_DISABLE_OS */

