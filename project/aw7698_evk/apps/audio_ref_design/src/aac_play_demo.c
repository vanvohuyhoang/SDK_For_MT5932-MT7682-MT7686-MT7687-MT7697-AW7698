/* Copyright Statement:
 *
 * (C) 2019  Airoha Technology Corp. All rights reserved.
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
 
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <stdlib.h>
//#include "debug_interaction.h"
#include "FreeRTOS.h"
#include "task.h"
#include "hal_log.h"
#include "ff.h"
#include "aac_decoder_api.h"
#include "aac_play_demo.h"
#include "hal_audio.h"

#define SHARE_BUF_SIZE  8192
#define DELAY_TIME_IN_MS 400

static FATFS fatfs;
static FIL fdst;
static FRESULT res;
static UINT length_read;
static uint8_t share_buffer[SHARE_BUF_SIZE];  //share buffer


static int16_t test_open_file_from_sd(FIL *fp, const TCHAR *path)
{
    FRESULT res;
    res = f_mount(&fatfs, _T("0:"), 1);

    if (!res) {
        log_hal_info("fmount ok \n");
        res = f_open(fp, path, FA_OPEN_EXISTING | FA_READ);
        if (!res) {
            log_hal_info("fopen ok \n");
        } else {
            log_hal_error("fopen error , res=%d\n", res);
            return -1;
        }
    } else {
        log_hal_info("fmount error \n");
    }

    if (res != FR_OK) {
        return -1;
    } else {
        return 0;
    }
}

static void test_stop_read_from_sd(aac_decoder_api_media_handle_t *hdl)
{
    hdl->stop(hdl);
    res = f_close(&fdst);
    if (!res) {
        log_hal_info("fclose success \n");
    } else {
        log_hal_error("fclose error \n");
    }
}


static void test_sd_event_callback(aac_decoder_api_media_handle_t *hdl, aac_decoder_api_media_event_t event)
{
    uint8_t *share_buf;
    uint32_t share_buf_len;

    switch (event) {
        case AAC_DECODER_API_MEDIA_EVENT_REQUEST:
            if (f_eof(&fdst)) {
                log_hal_info("[EOF]End of this file. Stop play\n");
                test_stop_read_from_sd(hdl);
                break;
            }

            uint32_t loop_idx;
            uint32_t loop_cnt = 2;
            for (loop_idx = 0; loop_idx < loop_cnt; loop_idx++) {
                hdl->get_write_buffer(hdl, &share_buf, &share_buf_len);
                f_read(&fdst, share_buf, share_buf_len, &length_read);
                //log_hal_info("SD_EVENT:length_read from SD=%d", length_read);
                hdl->write_data_done(hdl, length_read);
            }
            hdl->finish_write_data(hdl);
            break;
    }
}

void play_once(void)
{
    log_hal_info("play_once start\n");

    aac_decoder_api_media_handle_t *hdl = NULL;
    uint8_t *share_buf;
    uint32_t share_buf_len;

    if (test_open_file_from_sd(&fdst, _T("SD:/AAC/lc.aac")) < 0) {
        log_hal_error("Fail to open file in sd card\n");
        while (1) {
            vTaskDelay(DELAY_TIME_IN_MS / portTICK_RATE_MS);
        }
    }

    hdl = aac_deocder_api_open(test_sd_event_callback);
    if (hdl == NULL) {
        log_hal_error("aac_deocder_api_open fail.\n");
        while (1) {
            vTaskDelay(DELAY_TIME_IN_MS / portTICK_RATE_MS);
        }
    }

    hdl->set_share_buffer(hdl, share_buffer, SHARE_BUF_SIZE);

    /* prefill data to share  buffer */
    hdl->get_write_buffer(hdl, &share_buf, &share_buf_len);
    f_read(&fdst, share_buf, share_buf_len, &length_read);
    log_hal_info("share_buf_len=%d", share_buf_len);
    log_hal_info("length_read from SD=%d", length_read);
    hdl->write_data_done(hdl, length_read);
    hdl->finish_write_data(hdl);

    hdl->play(hdl);
    vTaskDelay(20000 / portTICK_RATE_MS);
    /*
        hdl->pause(hdl);
        vTaskDelay(5000 / portTICK_RATE_MS);

        hdl->resume(hdl);
        vTaskDelay(12000 / portTICK_RATE_MS);

        hdl->stop(hdl);
        aac_decoder_api_close(hdl);
        log_hal_info("play_once done\n");
    */

}

void aac_play_demo(void *pvParameters)
{

    play_once();

    while (1) {
        vTaskDelay(DELAY_TIME_IN_MS / portTICK_RATE_MS);//enter sleep requirement , >20ms
    }
}



