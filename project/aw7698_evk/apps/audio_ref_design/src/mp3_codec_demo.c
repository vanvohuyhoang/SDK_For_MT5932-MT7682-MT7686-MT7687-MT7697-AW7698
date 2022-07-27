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
#include "mp3_codec_demo.h"
#include "mp3_codec.h"
#include "hal_audio.h"
#include "hal_log.h"
#include "ff.h"
#include "hal_gpt.h"

#include "mp3_codec_internal_7682.h"
#include "mp3_codec.h"

#define MP3_CODEC_UT_PLAY_PAUSE_RESUME_10TIMES (0) // for QA test only SDK v4.5.0
#define MP3_CODEC_DEMO_PLAY_STOP_STEREOMONO_SR_8K_TO_48K (0)

static FATFS fatfs;
static FIL fdst;
static FRESULT res;
static UINT length_read;
static uint8_t first_time_f_mount = 1;

static int16_t test_open_file_from_sd(FIL *fp, const TCHAR *path)
{
    FRESULT res;
    printf("test_open_file_from_sd() +\r\n");
    if (first_time_f_mount){
        printf("f_mount() +\r\n");
        res = f_mount(&fatfs, _T("0:"), 1);
        printf("f_mount() -\r\n");
        first_time_f_mount = 0;
    } else {
        printf("already fmount\r\n");
        res = FR_OK;
    }
    if (!res) {
        printf("fmount ok \r\n");
        res = f_open(fp, path, FA_OPEN_EXISTING | FA_WRITE | FA_READ);
        if (!res) {
            printf("fopen ok \r\n");
        } else {
            printf("fopen error \r\n");
            return -1;
        }
    } else {
        printf("fmount error \r\n");
    }
    printf("test_open_file_from_sd() -\r\n");
    if (res != FR_OK) {
        return -1;
    } else {
        return 0;
    }
}

static void test_stop_read_from_sd(mp3_codec_media_handle_t *hdl)
{
    hdl->stop(hdl);
    res = f_close(&fdst);
    if (!res) {
        printf("[MP3 Codec Demo] fclose success \r\n");
    } else {
        printf("[MP3 Codec Demo] fclose error \r\n");
    }
    hdl->close_codec(hdl);
    printf("[MP3 Codec Demo] close codec() - \r\n");
}

static void test_sd_event_callback(mp3_codec_media_handle_t *hdl, mp3_codec_event_t event)
{
    uint8_t *share_buf;
    uint32_t share_buf_len;
    switch (event) {
        case MP3_CODEC_MEDIA_JUMP_FILE_TO:
            res = f_lseek(&fdst, (DWORD)hdl->jump_file_to_specified_position);
            // printf("[MP3 Codec Demo] MP3_CODEC_MEDIA_JUMP_FILE_TO: fdst read/write pointer=%x\r\n", f_tell(&fdst));
            break;
        case MP3_CODEC_MEDIA_BITSTREAM_END:
            printf("[MP3 Codec ut] [MP3_CODEC_MEDIA_BITSTREAM_END] Stop mp3\n");
            test_stop_read_from_sd(hdl);
            break;
        case MP3_CODEC_MEDIA_REQUEST:
        case MP3_CODEC_MEDIA_UNDERFLOW:
            if (f_eof(&fdst)) {
                printf("[MP3 Codec ut] [EOF] End of this mp3 file. Flush mp3\n");
                hdl->flush(hdl, 1);
                break;
            }
            uint32_t loop_idx;
            uint32_t loop_cnt = 2;
            for (loop_idx = 0; loop_idx < loop_cnt; loop_idx++) {
                hdl->get_write_buffer(hdl, &share_buf, &share_buf_len);
                f_read(&fdst, share_buf, share_buf_len, &length_read);
                hdl->write_data_done(hdl, share_buf_len);
            }
            hdl->finish_write_data(hdl);
            break;
    }
}

void mp3_codec_demo_play_stop()
{
    mp3_codec_media_handle_t *hdl = NULL;
    uint8_t *share_buf;
    uint32_t share_buf_len;

    // Open the mp3 codec.
    printf("[MP3 Codec Demo] open mp3 codec\r\n");
    hdl = mp3_codec_open(test_sd_event_callback);
    if (hdl == NULL) {
        printf("[MP3 Codec Demo] Fail to open the codec.");
        while (1) {
            vTaskDelay(10 / portTICK_RATE_MS);
        }
    }

    printf("[MP3 Codec Demo] mp3_codec_set_memory2() +\r\n");
    mp3_codec_set_memory2();
    printf("[MP3 Codec Demo] mp3_codec_set_memory2() -\r\n");

    if (test_open_file_from_sd(&fdst, _T("SD:/temp/1.mp3")) < 0) {
        printf("[MP3 Codec Demo] Fail to open SD:/temp/1.mp3.");
        while (1) {
            vTaskDelay(10 / portTICK_RATE_MS);
        }
    }

    /* prefill data to share  buffer */
    hdl->get_write_buffer(hdl, &share_buf, &share_buf_len);
    f_read(&fdst, share_buf, share_buf_len, &length_read);
    hdl->write_data_done(hdl, share_buf_len);
    hdl->finish_write_data(hdl);
    hdl->skip_id3v2_and_reach_next_frame(hdl, (uint32_t)f_size(&fdst));

    printf("[MP3 Codec Demo] play +\r\n");
    hdl->play(hdl);
    printf("[MP3 Codec Demo] play -\r\n");
}

void mp3_codec_demo_play_pause_resume()
{
    mp3_codec_media_handle_t *hdl = NULL;
    uint8_t *share_buf;
    uint32_t share_buf_len;

    // Open the mp3 codec.
    printf("[MP3 Codec Demo] open mp3 codec\r\n");
    hdl = mp3_codec_open(test_sd_event_callback);
    if (hdl == NULL) {
        log_hal_error("[MP3 Codec Demo] Fail to open the codec.");
        while (1) {
            vTaskDelay(10 / portTICK_RATE_MS);
        }
    }

    mp3_codec_set_memory2();

    if (test_open_file_from_sd(&fdst, _T("SD:/temp/1.mp3")) < 0) {
        log_hal_error("[MP3 Codec Demo] Fail to open SD:/temp/1.mp3.");
        while (1) {
            vTaskDelay(10 / portTICK_RATE_MS);
        }
    }

    /* prefill data to share  buffer */
    hdl->get_write_buffer(hdl, &share_buf, &share_buf_len);
    f_read(&fdst, share_buf, share_buf_len, &length_read);
    hdl->write_data_done(hdl, share_buf_len);
    hdl->finish_write_data(hdl);
    hdl->skip_id3v2_and_reach_next_frame(hdl, (uint32_t)f_size(&fdst));

    printf("[MP3 Codec Demo] play +\r\n");
    hdl->play(hdl);
    printf("[MP3 Codec Demo] play -\r\n");

    printf("[MP3 Codec Demo] vTaskDelay(5000 / portTICK_RATE_MS); +\r\n");
    vTaskDelay(5000 / portTICK_RATE_MS);
    printf("[MP3 Codec Demo] vTaskDelay(5000 / portTICK_RATE_MS); -\r\n");

    printf("[MP3 Codec Demo] pause +\r\n");
    hdl->pause(hdl);
    printf("[MP3 Codec Demo] pause -\r\n");    

    printf("[MP3 Codec Demo] hal_gpt_delay_ms(3000); +\r\n");
    hal_gpt_delay_ms(3000);
    printf("[MP3 Codec Demo] hal_gpt_delay_ms(3000); -\r\n");


    printf("[MP3 Codec Demo] resume() +\r\n");
    hdl->resume(hdl);
    printf("[MP3 Codec Demo] resume() -\r\n");

}

void mp3_codec_demo(void *pvParameters)
{
#if MP3_CODEC_DEMO_PLAY_STOP_STEREOMONO_SR_8K_TO_48K == 1
    mp3_codec_demo_play_stop_StereoMono_SR_8K_to_48K();
#endif
    mp3_codec_demo_play_stop();
    // mp3_codec_demo_play_pause_resume();

#if MP3_CODEC_UT_PLAY_PAUSE_RESUME_10TIMES == 1
    mp3_codec_demo_play_pause_resume_10times();
#endif

#if 0
#if 1
        uint32_t i = 0;
        uint32_t j = 0;
        uint32_t k = 0;
        uint32_t l = 0;    
#endif

    printf("[MP3 Codec Demo] pause +\r\n");
    hdl->pause(hdl);
    printf("[MP3 Codec Demo] pause -\r\n");
    
    printf("[MP3 Codec Demo] \r\n loop +\r\n");
    for(i = 0; i < (3 * 700); i++) { // 3 * 700 for 3 sec
//        for (j = 0; j < 0xFFFF; j++) {
//            for (k = 0; k < 0xFFFF; k++) {
                l = ((i / 3) + (j / 5) + (k / 7)) / 9;
                printf("[MP3 Codec Demo] %d", (int)l);
//            }
//        }
    }
    printf("[MP3 Codec Demo] \r\n loop -\r\n");

//    vTaskDelay(5000 / portTICK_RATE_MS);
//    printf("[MP3 Codec Demo] stop\n");
//    test_stop_read_from_sd(hdl);
#endif
#if 1
//    vTaskDelay(5000 / portTICK_RATE_MS);
    while (1) {
        vTaskDelay(100 / portTICK_RATE_MS);
    }
#endif
}
