/* Copyright Statement:
 *
 * (C) 2005-2017 MediaTek Inc. All rights reserved.
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
 * the License Agreement ("Permitted User"). If you are not a Permitted User,
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

#ifndef __HAL_AUDIO_TEST_H__
#define __HAL_AUDIO_TEST_H__

#include "hal_gpt.h"
#include "hal_log.h"
#include "hal_pmu.h"
#include "hal_audio.h"
#include "Audio_FFT.h"
#define FFT_BUFFER_SIZE 256
#define ch_idx                         (smt_curr & SMT_CH_LEFT ? 0:1)

#define KTONE_DL_ON dsp2mcu_audio_msg_t open_msg = MSG_MCU2DSP_PROMPT_OPEN;\
                    dsp2mcu_audio_msg_t start_msg = MSG_MCU2DSP_PROMPT_START;\
                    audio_message_type_t msg_type = AUDIO_MESSAGE_TYPE_PROMPT;\
                    void *p_param_share;\
                    dvfs_lock_control("AUDIO",DVFS_78M_SPEED, DVFS_LOCK);\
                    hal_audio_status_set_running_flag(AUDIO_MESSAGE_TYPE_PROMPT, true);\
                    hal_audio_status_set_running_flag(AUDIO_MESSAGE_TYPE_RECORD, true);\
                    mcu2dsp_open_param_t open_param;\
                    open_param.param.stream_in    = STREAM_IN_VP;\
                    open_param.param.stream_out = STREAM_OUT_AFE;\
                    open_param.stream_in_param.playback.bit_type = HAL_AUDIO_BITS_PER_SAMPLING_16;\
                    open_param.stream_in_param.playback.sampling_rate = HAL_AUDIO_SAMPLING_RATE_48KHZ;\
                    open_param.stream_in_param.playback.channel_number = HAL_AUDIO_MONO;\
                    open_param.stream_in_param.playback.codec_type = 0;\
                    open_param.stream_in_param.playback.p_share_info = hal_audio_query_share_info(msg_type);\
                    hal_audio_reset_share_info( open_param.stream_in_param.playback.p_share_info );\
                    open_param.stream_in_param.playback.p_share_info->sampling_rate = HAL_AUDIO_SAMPLING_RATE_48KHZ;\
                    open_param.stream_in_param.playback.p_share_info->bBufferIsFull = 1;\
                    open_param.stream_out_param.afe.audio_device    = HAL_AUDIO_DEVICE_DAC_DUAL; \
                    open_param.stream_out_param.afe.stream_channel    = HAL_AUDIO_DIRECT;\
                    open_param.stream_out_param.afe.memory            = HAL_AUDIO_MEM2;\
                    open_param.stream_out_param.afe.format            = AFE_PCM_FORMAT_S16_LE;\
                    open_param.stream_out_param.afe.sampling_rate    = HAL_AUDIO_SAMPLING_RATE_48KHZ;\
                    open_param.stream_out_param.afe.irq_period        = 10;\
                    open_param.stream_out_param.afe.frame_size        = 512;\
                    open_param.stream_out_param.afe.frame_number    = 4;\
                    open_param.stream_out_param.afe.hw_gain         = true;\
                    p_param_share = hal_audio_dsp_controller_put_paramter( &open_param, sizeof(mcu2dsp_open_param_t), msg_type);\
                    hal_audio_dsp_controller_send_message(open_msg, AUDIO_DSP_CODEC_TYPE_PCM, (uint32_t)p_param_share, true);\
                    mcu2dsp_start_param_t start_param;\
                    start_param.param.stream_in     = STREAM_IN_VP;\
                    start_param.param.stream_out    = STREAM_OUT_AFE;\
                    start_param.stream_out_param.afe.aws_flag          = false;\
                    start_param.stream_out_param.afe.aws_sync_request = true;\
                    start_param.stream_out_param.afe.aws_sync_time      = 1000;\
                    p_param_share = hal_audio_dsp_controller_put_paramter( &start_param, sizeof(mcu2dsp_start_param_t), msg_type);\
                    hal_audio_dsp_controller_send_message(start_msg, 0, (uint32_t)p_param_share, true);\
                    *(volatile uint32_t *)0x700001F0 = 0x048C28C2;\
                    *(volatile uint32_t *)0x700001DC = 0x00000024
                    //ami_hal_audio_status_set_running_flag(AUDIO_MESSAGE_TYPE_RECORD, true)


#define KTONE_DL_OFF    dsp2mcu_audio_msg_t stop_msg = MSG_MCU2DSP_PROMPT_STOP;\
                        dsp2mcu_audio_msg_t close_msg = MSG_MCU2DSP_PROMPT_CLOSE;\
                        hal_audio_dsp_controller_send_message(stop_msg, AUDIO_DSP_CODEC_TYPE_PCM, 0, true);\
                        hal_audio_dsp_controller_send_message(close_msg, AUDIO_DSP_CODEC_TYPE_PCM, 0, true);\
                        dvfs_lock_control("AUDIO",DVFS_78M_SPEED, DVFS_UNLOCK);\
                        hal_audio_status_set_running_flag(AUDIO_MESSAGE_TYPE_PROMPT, false);\
                        hal_audio_status_set_running_flag(AUDIO_MESSAGE_TYPE_RECORD, false)

static const uint16_t empty_feature[2] = {0x0,0x0};
typedef enum smt_chennel_e{
    SMT_CH_LEFT = 0x1,
    SMT_CH_RIGHT = 0x2,
    SMT_CH_NONE = 0x4,
}smt_ch;

typedef union {
    uint8_t value;
    struct {
        uint8_t curr_ch:4;
        uint8_t curr_mic:4;
    }filed;
}smt_status;

typedef struct {
    kal_uint32 u4Freq_data;
    kal_uint32 u4Mag_data;
    uint16_t bitstream_buf[FFT_BUFFER_SIZE];
    uint16_t *cpyIdx;
}fft_buf_t ;


void audio_smt_test(bool enable,smt_ch);
void audio_smt_test_pure_on_off(bool ,smt_ch );
void hal_audio_init_stream_buf(fft_buf_t *);
bool audio_stream_in(bool,smt_ch,fft_buf_t *);

#endif /*defined(__GNUC__)*/
