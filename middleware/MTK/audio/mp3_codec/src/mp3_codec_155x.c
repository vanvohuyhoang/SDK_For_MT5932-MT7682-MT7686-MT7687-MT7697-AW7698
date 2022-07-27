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
 
#include "mp3_codec.h"
#include "mp3_codec_internal.h"
#include "syslog.h"

#include "hal_audio_cm4_dsp_message.h"
#include "hal_audio_message_struct.h"
#include "hal_audio_internal.h"


#define ID3V2_HEADER_LENGTH 10  // in bytes
#define MPEG_AUDIO_FRAME_HEADER_LENGTH 4 // in bytes

#define SHARE_BUFFER_TOO_LESS_AMOUNT 1000        // maybe can be more precisely

static const char mp3_silence_frame_pattern[] = "This is a silence frame."; // sizeof(mp3_silence_frame_pattern) = 25, sizeof(char) = 1
#define MP3_SILENCE_FRAME_PATTERN_LENGTH (25) // in bytes

#define MAXIMUM(a,b)            ((a) > (b) ? (a) : (b))
#define MINIMUM(a,b)            ((a) < (b) ? (a) : (b))
#define UNUSED(x)  ((void)(x))

#define IS_MP3_HEAD(head) (!( (((head & 0xfff00000) != 0xfff00000) && ((head & 0xfff80000) != 0xffe00000) ) || \
  ( ((head>>17)&3)== 3) || (((head>>17)&3)== 0) || \
  ( ((head>>12)&0xf) == 0xf) || ( ((head>>12)&0xf) == 0x0) || \
( ((head>>10)&0x3) == 0x3 )))

//======== Log related ========
#define DEBUG_USE_MSGID_LOG
#ifdef DEBUG_USE_MSGID_LOG
#define MP3_LOG_D(fmt, arg...)          LOG_MSGID_D(MP3_CODEC, fmt, ##arg);
#define MP3_LOG_I(fmt, arg...)          LOG_MSGID_I(MP3_CODEC, fmt, ##arg);
#define MP3_LOG_W(fmt, arg...)          LOG_MSGID_W(MP3_CODEC, fmt, ##arg);
#define MP3_LOG_E(fmt, arg...)          LOG_MSGID_E(MP3_CODEC, fmt, ##arg);
#else
#define MP3_LOG_D(fmt, arg...)          LOG_E(MP3_CODEC, fmt, ##arg);
#define MP3_LOG_I(fmt, arg...)          LOG_I(MP3_CODEC, fmt, ##arg);
#define MP3_LOG_W(fmt, arg...)          LOG_W(MP3_CODEC, fmt, ##arg);
#define MP3_LOG_E(fmt, arg...)          LOG_E(MP3_CODEC, fmt, ##arg);
#endif

//======== Private API ========
static void mp3_dsp_codec_reset_share_buffer(mp3_codec_media_handle_t *handle);
static int32_t mp3_dsp_codec_get_share_buffer_data_count(mp3_codec_media_handle_t *handle);
static mp3_codec_function_return_state_t mp3_dsp_codec_request_data_to_share_buffer(mp3_codec_media_handle_t *handle);
static uint32_t mp3_dsp_codec_get_bytes_from_share_buffer(mp3_codec_media_handle_t *handle, uint8_t *destination_buffer, uint32_t get_bytes_amount, bool want_move_read_ptr);
static void mp3_dsp_codec_get_share_buffer_read_information(mp3_codec_media_handle_t *handle, uint8_t **buffer, uint32_t *length);
static void mp3_dsp_codec_share_buffer_read_data_done(mp3_codec_media_handle_t *handle, uint32_t length);


//==== ID3 tag related ====
static uint32_t mp3_dsp_codec_combine_four_bytes_buffer_to_uint32_value(uint8_t *buffer)
{
    uint32_t uint32_value = 0;

    uint32_value = *buffer;
    uint32_value = uint32_value << 8 | *(buffer + 1);
    uint32_value = uint32_value << 8 | *(buffer + 2);
    uint32_value = uint32_value << 8 | *(buffer + 3);

    return uint32_value;
}

static uint32_t mp3_dsp_codec_discard_bytes_of_share_buffer(mp3_codec_media_handle_t *handle, uint32_t discard_bytes_amount)
{
    uint8_t *share_buffer_read_address;
    uint32_t share_buffer_data_length;
    uint32_t bytes_amount_temp = discard_bytes_amount;
    uint32_t discarded_bytes_amount = 0;


    uint16_t loop_idx = 0;
    for (loop_idx = 0; loop_idx < 2; loop_idx++) {
        mp3_dsp_codec_get_share_buffer_read_information(handle, &share_buffer_read_address, &share_buffer_data_length);
        if (share_buffer_data_length > 0) {
            uint32_t get_bytes_amount = MINIMUM(bytes_amount_temp, share_buffer_data_length);
            bytes_amount_temp -= get_bytes_amount;
            mp3_dsp_codec_share_buffer_read_data_done(handle, get_bytes_amount);

            if (bytes_amount_temp == 0) {
                break;
            }
        } else {
            // share buffer empty
            break;
        }
    }

    discarded_bytes_amount = discard_bytes_amount - bytes_amount_temp;  // real read amount

    if (discarded_bytes_amount != discard_bytes_amount) {
        MP3_LOG_I("[MP3 Codec]mp3_codec_discard_bytes_of_share_buffer : discarded_bytes_amount(%ld) != discard_bytes_amount(%ld)\n",2, discarded_bytes_amount, discard_bytes_amount);
    }


    return discarded_bytes_amount;
}

static void mp3_dsp_codec_get_id3v2_info_and_skip(mp3_codec_media_handle_t *handle, uint32_t file_size)
{
    uint32_t want_get_bytes = 0;
    uint8_t temp_buffer[10] = {0};
    uint32_t id3v2_remain_tagesize = 0; // not include ID3v2 header size, refert to ID3v2 spec
    uint32_t id3v2_tage_size = 0;   // total ID3v2 tage size which include ID3v2 header
    uint32_t remain_file_data_size = file_size; // in bytes

    handle->jump_file_to_specified_position = 0;    // asume from file begin
    handle->id3v2_information.has_id3v2 = false;
    handle->id3v2_information.id3v2_tage_length = 0;

    while (1) {
        want_get_bytes = ID3V2_HEADER_LENGTH;
        if (mp3_dsp_codec_get_bytes_from_share_buffer(handle, temp_buffer, want_get_bytes, 0) != want_get_bytes) {
            MP3_LOG_I("[MP3 Codec]mp3_dsp_codec_get_id3v2_info_and_skip: share buffer data amount less than ID3v2 header length\n", 0);
            return;    // just return
        }

        if (strncmp((const char *)temp_buffer, "ID3", 3) == 0) {
            id3v2_remain_tagesize = ((temp_buffer[6] & 0x7f) << 21) | ((temp_buffer[7] & 0x7f) << 14) | ((temp_buffer[8] & 0x7f) <<  7) | ((temp_buffer[9] & 0x7f) <<  0);
            id3v2_tage_size = id3v2_remain_tagesize + ID3V2_HEADER_LENGTH;
            MP3_LOG_I("[MP3 Codec]find id3v2: id3v2_tagesize=%ld, id3v2_remain_tagesize =%ld\n",2, id3v2_tage_size, id3v2_remain_tagesize);


            if (remain_file_data_size < id3v2_tage_size) {
                // the tag size calculate form ID3v2 may wrong
                return;
            }


            handle->id3v2_information.has_id3v2 = true;
            handle->id3v2_information.id3v2_tage_length += id3v2_tage_size;


            // Although the remaing data in share buffer can be used,
            // but the fast and clear way to skip ID3v2 is just ask user to jump file to specific position and refill the share buffer
            mp3_dsp_codec_reset_share_buffer(handle);   // since we want to ask user to jump file to specific position and get data, thus remaining data is no use.
            handle->jump_file_to_specified_position += id3v2_tage_size;
            handle->handler(handle, MP3_CODEC_MEDIA_JUMP_FILE_TO);

            mp3_dsp_codec_request_data_to_share_buffer(handle);


            remain_file_data_size -= id3v2_tage_size;


        } else {
            MP3_LOG_I("[MP3 Codec]done skip ID3v2, has_id3v2=%d, id3v2_tage_length=%d\n",2, (uint32_t)handle->id3v2_information.has_id3v2, handle->id3v2_information.id3v2_tage_length);
            return;
        }
    }
}

static mp3_codec_function_return_state_t mp3_dsp_codec_reach_next_frame_and_get_audio_frame_header(mp3_codec_media_handle_t *handle, uint32_t *audio_frame_header, uint32_t maximum_check_bytes, uint32_t want_skip_frame_after_got_header)
{
    uint8_t check_mpeg_header_buffer[MPEG_AUDIO_FRAME_HEADER_LENGTH] = {0};
    uint32_t temp_mpeg_header = 0;
    uint32_t discard_bytes_amount = 0;
    uint32_t temp_uint32_t = 0;
    uint32_t temp_maximum_check_bytes = maximum_check_bytes;
    uint32_t maximum_request_data_time = maximum_check_bytes / handle->share_buff.buffer_size + 2;  // 2: arbitrarily selected


    do {
        if (mp3_dsp_codec_get_share_buffer_data_count(handle) < MPEG_AUDIO_FRAME_HEADER_LENGTH) {

            if (mp3_dsp_codec_request_data_to_share_buffer(handle) == MP3_CODEC_RETURN_OK) {
                maximum_request_data_time--;
            }

            if (mp3_dsp_codec_get_share_buffer_data_count(handle) < MPEG_AUDIO_FRAME_HEADER_LENGTH) {
                return MP3_CODEC_RETURN_ERROR;
            }
        }

        mp3_dsp_codec_get_bytes_from_share_buffer(handle, check_mpeg_header_buffer, MPEG_AUDIO_FRAME_HEADER_LENGTH, 0);
        temp_mpeg_header = mp3_dsp_codec_combine_four_bytes_buffer_to_uint32_value(check_mpeg_header_buffer);

        if (IS_MP3_HEAD(temp_mpeg_header)) {
            // find MP3 HEAD
            *audio_frame_header = temp_mpeg_header;

            if (want_skip_frame_after_got_header) {
                discard_bytes_amount = 4;
                temp_uint32_t = mp3_dsp_codec_discard_bytes_of_share_buffer(handle, discard_bytes_amount);
                if (temp_uint32_t < discard_bytes_amount) {  // share buffer didn't have enoungh data to discared
                    return MP3_CODEC_RETURN_ERROR;
                }
            }
            MP3_LOG_I("[MP3 Codec]mp3_codec_reach_next_frame: find mp3 header=%x\n",1, *audio_frame_header);
            return MP3_CODEC_RETURN_OK;
        }

        discard_bytes_amount = 1;
        temp_uint32_t = MINIMUM(discard_bytes_amount, temp_maximum_check_bytes);
        temp_uint32_t = mp3_dsp_codec_discard_bytes_of_share_buffer(handle, temp_uint32_t);
        if (temp_uint32_t < discard_bytes_amount) {  // share buffer didn't have enoungh data to discared
            return MP3_CODEC_RETURN_ERROR;
        }

        temp_maximum_check_bytes -= temp_uint32_t;

    } while (temp_maximum_check_bytes != 0 && maximum_request_data_time != 0);


    MP3_LOG_I("[MP3 Codec]mp3_codec_reach_next_frame: not find mp3 header\n", 0);
    *audio_frame_header = 0;

    return MP3_CODEC_RETURN_ERROR;
}

static mp3_codec_function_return_state_t mp3_dsp_codec_skip_id3v2_and_reach_next_frame(mp3_codec_media_handle_t *handle, uint32_t file_size)
{
    uint32_t auido_frame_header = 0;
    
    if (mp3_dsp_codec_get_share_buffer_data_count(handle) < ID3V2_HEADER_LENGTH) {
        return MP3_CODEC_RETURN_ERROR;
    }

    mp3_dsp_codec_get_id3v2_info_and_skip(handle, file_size);

    if (mp3_dsp_codec_reach_next_frame_and_get_audio_frame_header(handle, &auido_frame_header, 2048, 0) != MP3_CODEC_RETURN_OK) {
        return MP3_CODEC_RETURN_ERROR;
    }

    return MP3_CODEC_RETURN_OK;
}

//== HAL callback function ==
static void mp3_dsp_codec_isr_callback(hal_audio_event_t event, void *data)
{
    mp3_codec_media_handle_t *handle = (mp3_codec_media_handle_t *)data;
    audio_message_type_t type;
    n9_dsp_share_info_t *p_info;
    
    // Decide the type according to application
    type = AUDIO_MESSAGE_TYPE_PLAYBACK;
#if defined(MTK_AUDIO_MIXER_SUPPORT)
    if (handle->mixer_track_role != AUDIO_MIXER_TRACK_ROLE_MAIN) {
        type = AUDIO_MESSAGE_TYPE_PROMPT;
    }
#endif
    p_info = hal_audio_query_share_info(type);
    
    switch (event) {
        case HAL_AUDIO_EVENT_UNDERFLOW:
        case HAL_AUDIO_EVENT_DATA_REQUEST:
            {
                uint32_t i;
            
                for (i=0; i<2; i++) {
                    uint32_t free_space_count;
                    uint8_t *p_source_buf;
                    uint32_t source_data_count;
                    
                    // Get bitstream buffer
                     mp3_dsp_codec_get_share_buffer_read_information(handle, &p_source_buf, &source_data_count);
                    
                    // Query the free space of share buffer
                    free_space_count = hal_audio_buf_mgm_get_free_byte_count(p_info);
                    
                    // Write data to share buffer
                    if (source_data_count > free_space_count) {
                        source_data_count = free_space_count;
                    }
                    hal_audio_write_stream_out_by_type(type, p_source_buf, source_data_count);
                    mp3_dsp_codec_share_buffer_read_data_done(handle, source_data_count);
                
                    if (!handle->waiting) {
                        // Callback and notify task
                        handle->waiting = true;
                        handle->handler(handle, MP3_CODEC_MEDIA_REQUEST);
                    }
                }
            }
            
            break;

        case HAL_AUDIO_EVENT_END:
            handle->handler(handle, MP3_CODEC_MEDIA_BITSTREAM_END);
            break;
    }
}

static void mp3_dsp_codec_play_internal(mp3_codec_media_handle_t *handle)
{
    audio_message_type_t msg_type = AUDIO_MESSAGE_TYPE_PLAYBACK;
    mcu2dsp_audio_msg_t open_msg = MSG_MCU2DSP_PLAYBACK_OPEN;
    mcu2dsp_audio_msg_t start_msg = MSG_MCU2DSP_PLAYBACK_START;
    void *p_param_share;

#if defined(MTK_AUDIO_MIXER_SUPPORT)
    if (handle->mixer_track_role != AUDIO_MIXER_TRACK_ROLE_MAIN) {
        msg_type = AUDIO_MESSAGE_TYPE_PROMPT;
        open_msg = MSG_MCU2DSP_PROMPT_OPEN;
        start_msg = MSG_MCU2DSP_PROMPT_START;
    }
#endif

    mcu2dsp_open_param_t open_param;

    open_param.param.stream_in  = STREAM_IN_PLAYBACK;
    open_param.param.stream_out = STREAM_OUT_AFE;

    open_param.stream_in_param.playback.bit_type = HAL_AUDIO_BITS_PER_SAMPLING_16;
    open_param.stream_in_param.playback.sampling_rate = 0;
    open_param.stream_in_param.playback.channel_number = 0;
    open_param.stream_in_param.playback.codec_type = 0;  //KH: should use AUDIO_DSP_CODEC_TYPE_PCM
    open_param.stream_in_param.playback.p_share_info = hal_audio_query_share_info(msg_type);
	open_param.stream_in_param.playback.p_share_info->bBufferIsFull = true;

    hal_audio_reset_share_info( open_param.stream_in_param.playback.p_share_info );

    open_param.stream_out_param.afe.audio_device    = HAL_AUDIO_DEVICE_HEADSET;
    open_param.stream_out_param.afe.stream_channel  = HAL_AUDIO_STEREO;
    if(msg_type == AUDIO_MESSAGE_TYPE_PROMPT){
        open_param.stream_out_param.afe.memory          = HAL_AUDIO_MEM2;
    }else{
        open_param.stream_out_param.afe.memory          = HAL_AUDIO_MEM1;
    }
    open_param.stream_out_param.afe.format          = AFE_PCM_FORMAT_S16_LE;
    open_param.stream_out_param.afe.sampling_rate   = 0;
    open_param.stream_out_param.afe.irq_period      = 10;
    open_param.stream_out_param.afe.frame_size      = 512;
    open_param.stream_out_param.afe.frame_number    = 4;
    open_param.stream_out_param.afe.hw_gain         = false;
    p_param_share = hal_audio_dsp_controller_put_paramter( &open_param, sizeof(mcu2dsp_open_param_t), msg_type);
    // Notify to do dynamic download. Use async wait.
    hal_audio_dsp_controller_send_message(open_msg, AUDIO_DSP_CODEC_TYPE_PCM, (uint32_t)p_param_share, true);

    // Register callback
    hal_audio_service_hook_callback(msg_type, mp3_dsp_codec_isr_callback, handle);

    // Fill data into share buffer
    {
        uint32_t i;

        n9_dsp_share_info_t *p_dsp_info;

        p_dsp_info = hal_audio_query_share_info(msg_type);

        for (i=0; i<2; i++) {
            uint32_t free_space_count;
            uint8_t *p_source_buf;
            uint32_t source_data_count;

            // Get bitstream buffer
            mp3_dsp_codec_get_share_buffer_read_information(handle, &p_source_buf, &source_data_count);

            // Query the free space of share buffer
            free_space_count = hal_audio_buf_mgm_get_free_byte_count(p_dsp_info);

            // Write data to share buffer
            if (source_data_count > free_space_count) {
                source_data_count = free_space_count;
            }

            // Update pointer
            hal_audio_write_stream_out_by_type(msg_type, p_source_buf, source_data_count);
            mp3_dsp_codec_share_buffer_read_data_done(handle, source_data_count);
        }
    }

    // Notify to play
    {
        mcu2dsp_start_param_t start_param;

        // Collect parameters
        start_param.param.stream_in     = STREAM_IN_PLAYBACK;
        start_param.param.stream_out    = STREAM_OUT_AFE;

        start_param.stream_out_param.afe.aws_flag   =  false;

        p_param_share = hal_audio_dsp_controller_put_paramter( &start_param, sizeof(mcu2dsp_start_param_t), msg_type);
        hal_audio_dsp_controller_send_message(start_msg, 0, (uint32_t)p_param_share, true);
    }
}

static void mp3_dsp_codec_stop_internal(mp3_codec_media_handle_t *handle)
{
    audio_message_type_t msg_type = AUDIO_MESSAGE_TYPE_PLAYBACK;
    mcu2dsp_audio_msg_t stop_msg = MSG_MCU2DSP_PLAYBACK_STOP;
    mcu2dsp_audio_msg_t close_msg = MSG_MCU2DSP_PLAYBACK_CLOSE;

#if defined(MTK_AUDIO_MIXER_SUPPORT)
    if (handle->mixer_track_role != AUDIO_MIXER_TRACK_ROLE_MAIN) {
        msg_type = AUDIO_MESSAGE_TYPE_PROMPT;
        stop_msg = MSG_MCU2DSP_PROMPT_STOP;
        close_msg = MSG_MCU2DSP_PROMPT_CLOSE;
    }
#endif

    // Notify to stop
    hal_audio_dsp_controller_send_message(stop_msg, AUDIO_DSP_CODEC_TYPE_MP3, 0, true);

    // Unregister callback
    hal_audio_service_unhook_callback(msg_type);

    // Notify to release dynamic download
    hal_audio_dsp_controller_send_message(close_msg, AUDIO_DSP_CODEC_TYPE_MP3, 0, true);

}

//==== Member function ====
static mp3_codec_function_return_state_t mp3_dsp_codec_play(mp3_codec_media_handle_t *handle)
{
    mp3_dsp_codec_play_internal(handle);

    handle->flush_data_flag = 0;
    handle->state = MP3_CODEC_STATE_PLAY;

    return MP3_CODEC_RETURN_OK;
}

static mp3_codec_function_return_state_t mp3_dsp_codec_pause(mp3_codec_media_handle_t *handle)
{
    if (handle->state != MP3_CODEC_STATE_PLAY) {
        MP3_LOG_I("[MP3 Codec] pause --: cannot pause because state(%d)\r\n",1, handle->state);
        return MP3_CODEC_RETURN_ERROR;
    }
    
    mp3_dsp_codec_stop_internal(handle);

    handle->state = MP3_CODEC_STATE_PAUSE;

    return MP3_CODEC_RETURN_OK;
}

static mp3_codec_function_return_state_t mp3_dsp_codec_resume(mp3_codec_media_handle_t *handle)
{
    if (handle->state != MP3_CODEC_STATE_PAUSE) {
        MP3_LOG_I("[MP3 Codec] resume --: cannot resume because state(%d)\r\n",1, handle->state);

        return MP3_CODEC_RETURN_ERROR;
    }

    mp3_dsp_codec_play_internal(handle);

    handle->flush_data_flag = 0;
    handle->state = MP3_CODEC_STATE_PLAY;

    return MP3_CODEC_RETURN_OK;
}

static mp3_codec_function_return_state_t mp3_dsp_codec_stop(mp3_codec_media_handle_t *handle)
{
    if (handle->state != MP3_CODEC_STATE_PLAY && handle->state != MP3_CODEC_STATE_PAUSE) {
        MP3_LOG_I("[MP3 Codec] stop --: cannot stop because state(%d)\r\n",1, handle->state);
        return MP3_CODEC_RETURN_ERROR;
    }
    
    mp3_dsp_codec_stop_internal(handle);

    handle->state = MP3_CODEC_STATE_STOP;

    return MP3_CODEC_RETURN_OK;
}

static void mp3_dsp_codec_set_share_buffer(mp3_codec_media_handle_t *handle, uint8_t *buffer, uint32_t length)
{
    handle->share_buff.buffer_base = buffer;
    handle->share_buff.buffer_size = length;
    handle->share_buff.write = 0;
    handle->share_buff.read = 0;
    handle->waiting = false;
    handle->underflow = false;
    handle->linear_buff = false;
}

static void mp3_dsp_codec_get_share_buffer_write_information(mp3_codec_media_handle_t *handle, uint8_t **buffer, uint32_t *length)
{
    int32_t count = 0;

    if (handle->share_buff.read > handle->share_buff.write) {
        count = handle->share_buff.read - handle->share_buff.write - 1;
    } else if (handle->share_buff.read == 0) {
        count = handle->share_buff.buffer_size - handle->share_buff.write - 1;
    } else {
        count = handle->share_buff.buffer_size - handle->share_buff.write;
    }
    *buffer = handle->share_buff.buffer_base + handle->share_buff.write;
    *length = count;
}

static void mp3_dsp_codec_get_share_buffer_read_information(mp3_codec_media_handle_t *handle, uint8_t **buffer, uint32_t *length)
{
    int32_t count = 0;

    if (handle->share_buff.write >= handle->share_buff.read) {
        count = handle->share_buff.write - handle->share_buff.read;
    } else {
        count = handle->share_buff.buffer_size - handle->share_buff.read;
    }
    *buffer = handle->share_buff.buffer_base + handle->share_buff.read;
    *length = count;
}

static void mp3_dsp_codec_share_buffer_write_data_done(mp3_codec_media_handle_t *handle, uint32_t length)
{
    handle->share_buff.write += length;
    if (handle->share_buff.write == handle->share_buff.buffer_size) {
        handle->share_buff.write = 0;
    }
}

static void mp3_dsp_codec_finish_write_data(mp3_codec_media_handle_t *handle)
{
    handle->waiting = false;
    handle->underflow = false;

    if (MP3_CODEC_STATE_PLAY == handle->state) {
        int32_t share_buffer_data_amount = mp3_dsp_codec_get_share_buffer_data_count(handle);
        if (mp3_handle_ptr_to_previous_mp3_frame_size(handle) > 0) {
            if (share_buffer_data_amount > mp3_handle_ptr_to_previous_mp3_frame_size(handle)) {
//KH                mp3_codec_event_send_from_isr(MP3_CODEC_QUEUE_EVENT_DECODE, handle);
            }
        } else if (share_buffer_data_amount > SHARE_BUFFER_TOO_LESS_AMOUNT) {
//KH            mp3_codec_event_send_from_isr(MP3_CODEC_QUEUE_EVENT_DECODE, handle);
        }
    }

    uint32_t current_cnt = 0;
    hal_gpt_get_free_run_count(HAL_GPT_CLOCK_SOURCE_1M, &current_cnt);
#if defined(MTK_AUDIO_MIXER_SUPPORT)
    MP3_LOG_I("[MP3 Codec] finish_write_data role%d: %d ms\n",2, handle->mixer_track_role, (current_cnt - mp3_handle_ptr_to_share_buffer_waiting_cnt(handle)) / 1000);
#else
    MP3_LOG_I("[MP3 Codec] finish_write_data: %d ms\n",1, (current_cnt - mp3_handle_ptr_to_share_buffer_waiting_cnt(handle)) / 1000);
#endif

}

static void mp3_dsp_codec_reset_share_buffer(mp3_codec_media_handle_t *handle)
{
    memset(handle->share_buff.buffer_base, 0, handle->share_buff.buffer_size);  // do this or it will have previous data. we met that when change to bad mp3 file, but at this time app still use write_data_done, it will have previous header

    handle->share_buff.write = 0;
    handle->share_buff.read = 0;
    handle->waiting = false;
    handle->underflow = false;
}

static void mp3_dsp_codec_share_buffer_read_data_done(mp3_codec_media_handle_t *handle, uint32_t length)
{
    handle->share_buff.read += length;
    if (handle->share_buff.read == handle->share_buff.buffer_size) {
        handle->share_buff.read = 0;
    }
}

static int32_t mp3_dsp_codec_get_share_buffer_free_space(mp3_codec_media_handle_t *handle)
{
    int32_t count = 0;

    count = handle->share_buff.read - handle->share_buff.write - 2;
    if (count < 0) {
        count += handle->share_buff.buffer_size;
    }
    return count;
}

static int32_t mp3_dsp_codec_get_share_buffer_data_count(mp3_codec_media_handle_t *handle)
{
    int32_t count = 0;

    count = handle->share_buff.write - handle->share_buff.read;
    if (count < 0) {
        count += handle->share_buff.buffer_size;
    }
    return count;
}

static mp3_codec_function_return_state_t mp3_dsp_codec_request_data_to_share_buffer(mp3_codec_media_handle_t *handle)
{
    // return MP3_CODEC_RETURN_OK:          request success
    // return  MP3_CODEC_RETURN_ERROR:    already request and waiting feed back

    mp3_codec_internal_handle_t *internal_handle = mp3_handle_ptr_to_internal_ptr(handle);

    if (handle->linear_buff == 1) {
        return MP3_CODEC_RETURN_OK; // Just return in Linear Buffer 
    }

    if (!handle->waiting) {
#if defined(MTK_AUDIO_MIXER_SUPPORT)
        MP3_LOG_I("[MP3 Codec] role%d request data: share = %d, pcm = %d\r\n",3, handle->mixer_track_role, mp3_dsp_codec_get_share_buffer_data_count(handle), ring_buffer_get_space_byte_count(&internal_handle->stream_out_pcm_buff));
#else
        MP3_LOG_I("[MP3 Codec] request data: share = %d, pcm = %d\r\n",2, mp3_dsp_codec_get_share_buffer_data_count(handle), ring_buffer_get_space_byte_count(&internal_handle->stream_out_pcm_buff));
#endif
        hal_gpt_get_free_run_count(HAL_GPT_CLOCK_SOURCE_1M, &(mp3_handle_ptr_to_share_buffer_waiting_cnt(handle)));
        handle->waiting = true;
        handle->handler(handle, MP3_CODEC_MEDIA_REQUEST);
        return MP3_CODEC_RETURN_OK;
    } else {
#if defined(MTK_AUDIO_MIXER_SUPPORT)
        MP3_LOG_I("[MP3 Codec] role%d request data again: share = %d, pcm = %d\r\n",3, handle->mixer_track_role, mp3_dsp_codec_get_share_buffer_data_count(handle), ring_buffer_get_space_byte_count(&internal_handle->stream_out_pcm_buff));
#else
        MP3_LOG_I("[MP3 Codec] request data again: share = %d, pcm = %d\r\n",2, mp3_dsp_codec_get_share_buffer_data_count(handle), ring_buffer_get_space_byte_count(&internal_handle->stream_out_pcm_buff));
#endif
        return MP3_CODEC_RETURN_ERROR;
    }
}

static void mp3_dsp_codec_flush(mp3_codec_media_handle_t *handle, int32_t flush_data_flag)
{
    handle->flush_data_flag = flush_data_flag;
    MP3_LOG_I("[MP3 Codec] flush_data_flag = %d\n",1, handle->flush_data_flag);
}

static void mp3_dsp_codec_get_data_status(mp3_codec_media_handle_t *handle, mp3_codec_data_type_t data_type, int32_t *data_status)
{
    //audio_message_type_t type;
    n9_dsp_share_info_t *p_info;
    
    // KH ToDo: Should according application
    //type = AUDIO_MESSAGE_TYPE_PLAYBACK;
    p_info = hal_audio_query_playback_share_info();
    
    *data_status = 0;

    switch (data_type) {
        case MP3_CODEC_DATA_TYPE_AUDIO:
            if (handle->state == MP3_CODEC_STATE_PLAY) {
                mp3_codec_internal_handle_t *internal_handle = mp3_handle_ptr_to_internal_ptr(handle);
                uint8_t *out_buf_ptr    = NULL;
                uint32_t stream_out_pcm_buffer_data_count = 0;

#if defined(MP3_SW_DECODE_SUPPORT)
                ring_buffer_get_read_information(&internal_handle->stream_out_pcm_buff, &out_buf_ptr, &stream_out_pcm_buffer_data_count);
#endif
                if (stream_out_pcm_buffer_data_count == 0) {
                    uint32_t sample_count = 0;

                    sample_count = hal_audio_buf_mgm_get_free_byte_count(p_info);

                    *data_status = 1;
                    UNUSED(sample_count);
                }
                UNUSED(internal_handle);
                UNUSED(out_buf_ptr);
            }
            break;
        case MP3_CODEC_DATA_TYPE_SAMPLES_PER_CHANNEL:
            if (handle->state == MP3_CODEC_STATE_PLAY) {
#if defined(MP3_SW_DECODE_SUPPORT)
                mp3_codec_internal_handle_t *internal_handle = (mp3_codec_internal_handle_t *)handle;
                *data_status = internal_handle->mp3_handle->PCMSamplesPerCH;
#endif
            }
            break;
    }
}

static uint32_t mp3_dsp_codec_get_bytes_from_share_buffer(mp3_codec_media_handle_t *handle, uint8_t *destination_buffer, uint32_t get_bytes_amount, bool want_move_read_ptr)
{
    uint8_t *share_buffer_read_address;
    uint32_t share_buffer_data_length;
    uint32_t share_buffer_read_index_original = 0;
    uint32_t bytes_amount_temp = get_bytes_amount;
    uint32_t got_bytes_amount = 0;  // real got bytes amount from share buffer
    uint32_t get_bytes_amount_mini;

    share_buffer_read_index_original = handle->share_buff.read; // store original share_buffer read pointer


    uint16_t loop_idx = 0;
    for (loop_idx = 0; loop_idx < 2; loop_idx++) {
        mp3_dsp_codec_get_share_buffer_read_information(handle, &share_buffer_read_address, &share_buffer_data_length);
        if (share_buffer_data_length > 0) {
            get_bytes_amount_mini = MINIMUM(bytes_amount_temp, share_buffer_data_length);
            memcpy(destination_buffer, share_buffer_read_address, get_bytes_amount_mini);
            bytes_amount_temp -= get_bytes_amount_mini;
            destination_buffer += get_bytes_amount_mini;
            mp3_dsp_codec_share_buffer_read_data_done(handle, get_bytes_amount_mini);

            if (bytes_amount_temp == 0) {
                break;
            }
        } else {
            // share buffer empty
            break;
        }
    }


    got_bytes_amount = get_bytes_amount - bytes_amount_temp;  // real read amount

    if (got_bytes_amount != get_bytes_amount) {
        MP3_LOG_I("[MP3 Codec]mp3_codec_get_bytes_from_share_buffer: got_bytes_amount(%ld) != get_bytes_amount(%ld)\n",2, got_bytes_amount, get_bytes_amount);
    }


    if (want_move_read_ptr == false) {
        handle->share_buff.read = share_buffer_read_index_original;
    }

    return got_bytes_amount;
}

static mp3_codec_function_return_state_t mp3_dsp_codec_set_bitstream_buffer(mp3_codec_media_handle_t *handle, uint8_t *buffer, uint32_t length)
{
    length = (length + 3) & ~0x3;
    handle->share_buff.buffer_base = buffer;
    handle->share_buff.buffer_size = length + 1;
    handle->share_buff.write = length;
    handle->share_buff.read = 0;
    handle->waiting = false;
    handle->underflow = false;
    handle->linear_buff = true;

    return MP3_CODEC_RETURN_OK;
}

static int32_t mp3_dsp_codec_set_silence_frame_information(mp3_codec_media_handle_t *handle, silence_frame_information_t *frm_info)
{
    mp3_codec_internal_handle_t *internal_handle = (mp3_codec_internal_handle_t *)handle;
    
    if (frm_info->frame_size > 0) {
        internal_handle->MP3SilenceFrameSize = frm_info->frame_size;
        internal_handle->MP3SilenceFrameHeader[0] = frm_info->frame[0];
        internal_handle->MP3SilenceFrameHeader[1] = frm_info->frame[1];
        internal_handle->MP3SilenceFrameHeader[2] = frm_info->frame[2];
        internal_handle->MP3SilenceFrameHeader[3] = frm_info->frame[3];

        MP3_LOG_I("MP3SilenceFrameSize = %d, MP3SilenceFrameHeader = 0x%X %X %X %X \n",5, internal_handle->MP3SilenceFrameSize, 
            internal_handle->MP3SilenceFrameHeader[0], 
            internal_handle->MP3SilenceFrameHeader[1],
            internal_handle->MP3SilenceFrameHeader[2],
            internal_handle->MP3SilenceFrameHeader[3]);

        return 0;
    }

    return -1;
}

static int32_t mp3_dsp_codec_get_silence_frame_information(mp3_codec_media_handle_t *handle, int32_t *byte_count)
{
    mp3_codec_internal_handle_t *internal_handle = (mp3_codec_internal_handle_t *)handle;
    
    if (internal_handle->MP3SilenceFrameSize > 0) {
        *byte_count = internal_handle->MP3SilenceFrameSize;
    }

    return 0;
}

static int32_t mp3_dsp_codec_fill_silence_frame(mp3_codec_media_handle_t *handle, uint8_t *buffer)
{
    mp3_codec_internal_handle_t *internal_handle = (mp3_codec_internal_handle_t *)handle;

    if (internal_handle->MP3SilenceFrameSize > 0) {
        memset(buffer, 0, internal_handle->MP3SilenceFrameSize);
        buffer[0] = internal_handle->MP3SilenceFrameHeader[0];
        buffer[1] = internal_handle->MP3SilenceFrameHeader[1];
        buffer[2] = internal_handle->MP3SilenceFrameHeader[2];
        buffer[3] = internal_handle->MP3SilenceFrameHeader[3];
        strncpy((char *)(buffer + MPEG_AUDIO_FRAME_HEADER_LENGTH), mp3_silence_frame_pattern, sizeof(mp3_silence_frame_pattern));
    }

    return 0;
}

#ifdef MTK_BT_AWS_ENABLE
// Fake function
mp3_codec_function_return_state_t mp3_dsp_codec_aws_init(mp3_codec_media_handle_t *handle)
{
    return MP3_CODEC_RETURN_OK;
}

mp3_codec_function_return_state_t mp3_dsp_codec_aws_deinit(mp3_codec_media_handle_t *handle)
{
    return MP3_CODEC_RETURN_OK;
}

mp3_codec_function_return_state_t mp3_dsp_codec_aws_set_clock_skew_compensation_value(mp3_codec_media_handle_t *handle, int32_t sample_count)
{
    return MP3_CODEC_RETURN_OK;
}

mp3_codec_function_return_state_t mp3_dsp_codec_aws_get_clock_skew_status(mp3_codec_media_handle_t *handle, int32_t *aws_clock_skew_status)
{
    *aws_clock_skew_status = AWS_CLOCK_SKEW_STATUS_IDLE;

    return MP3_CODEC_RETURN_OK;
}

mp3_codec_function_return_state_t mp3_dsp_codec_aws_set_clock_skew(mp3_codec_media_handle_t *handle, bool flag)
{
    return MP3_CODEC_RETURN_OK;
}
#endif

#if defined(MTK_AUDIO_MIXER_SUPPORT)
void mp3_dsp_codec_set_track_role(mp3_codec_media_handle_t *handle, audio_mixer_role_t role)
{
    handle->mixer_track_role = role;
}
#endif

static void mp3_dsp_codec_buffer_function_init(mp3_codec_media_handle_t *handle)
{
    handle->set_share_buffer   = mp3_dsp_codec_set_share_buffer;
    handle->get_write_buffer   = mp3_dsp_codec_get_share_buffer_write_information;
    handle->get_read_buffer    = mp3_dsp_codec_get_share_buffer_read_information;
    handle->write_data_done    = mp3_dsp_codec_share_buffer_write_data_done;
    handle->finish_write_data  = mp3_dsp_codec_finish_write_data;
    handle->reset_share_buffer = mp3_dsp_codec_reset_share_buffer;
    handle->read_data_done     = mp3_dsp_codec_share_buffer_read_data_done;
    handle->get_free_space     = mp3_dsp_codec_get_share_buffer_free_space;
    handle->get_data_count     = mp3_dsp_codec_get_share_buffer_data_count;
    handle->flush              = mp3_dsp_codec_flush;
    handle->get_data_status    = mp3_dsp_codec_get_data_status;
    handle->set_bitstream_buffer = mp3_dsp_codec_set_bitstream_buffer;
    handle->set_silence_frame_information = mp3_dsp_codec_set_silence_frame_information;
    handle->get_silence_frame_information = mp3_dsp_codec_get_silence_frame_information;
    handle->fill_silence_frame = mp3_dsp_codec_fill_silence_frame;
#ifdef MTK_BT_AWS_ENABLE
    handle->aws_init            = mp3_dsp_codec_aws_init;
    handle->aws_deinit          = mp3_dsp_codec_aws_deinit;
    handle->aws_set_clock_skew_compensation_value = mp3_dsp_codec_aws_set_clock_skew_compensation_value;
    handle->aws_get_clock_skew_status = mp3_dsp_codec_aws_get_clock_skew_status;
    handle->aws_set_clock_skew = mp3_dsp_codec_aws_set_clock_skew;
#endif
#if defined(MTK_AUDIO_MIXER_SUPPORT)
    handle->set_track_role     = mp3_dsp_codec_set_track_role;
#endif
}

//======== Public API ========
#if defined(MTK_AUDIO_MIXER_SUPPORT)
mp3_codec_function_return_state_t mp3_dsp_codec_set_memory2(mp3_codec_media_handle_t *handle)
#else
mp3_codec_function_return_state_t mp3_dsp_codec_set_memory2(void)
#endif
{
    //KH: Treat it as fake function
    //If the decoder is implement on CM4, I will allocate memory in play function.
    
    return MP3_CODEC_RETURN_OK;
}

mp3_codec_media_handle_t *mp3_dsp_codec_open(mp3_codec_callback_t mp3_codec_callback)
{
    mp3_codec_media_handle_t *handle;
    mp3_codec_internal_handle_t *internal_handle;
    
    // Allocate memory
    internal_handle = (mp3_codec_internal_handle_t *)pvPortMalloc(sizeof(mp3_codec_internal_handle_t));
    if (NULL == internal_handle) {
        MP3_LOG_E("[MP3 Codec] mp3_codec_open failed: cannot allocate internal_handle\n", 0);
        return NULL;
    }
    memset(internal_handle, 0, sizeof(mp3_codec_internal_handle_t));
    handle = &internal_handle->handle;
    
    // Initialize member function
    handle->handler  = mp3_codec_callback;
    handle->play     = mp3_dsp_codec_play;
    handle->pause    = mp3_dsp_codec_pause;
    handle->resume   = mp3_dsp_codec_resume;
    handle->stop     = mp3_dsp_codec_stop;
    handle->close_codec = mp3_codec_close;
    handle->skip_id3v2_and_reach_next_frame = mp3_dsp_codec_skip_id3v2_and_reach_next_frame;
    
    mp3_dsp_codec_buffer_function_init(handle);
    
    handle->state    = MP3_CODEC_STATE_READY;
    
    return handle;
}

mp3_codec_function_return_state_t mp3_dsp_codec_close(mp3_codec_media_handle_t *handle)
{
    mp3_codec_internal_handle_t *internal_handle = (mp3_codec_internal_handle_t *)handle;
    
    if (handle->state != MP3_CODEC_STATE_STOP && handle->state != MP3_CODEC_STATE_READY) {
        MP3_LOG_I("[MP3 Codec] close--: cannot close because state(%d)\r\n",1, handle->state);
        return MP3_CODEC_RETURN_ERROR;
    }

    handle->state = MP3_CODEC_STATE_IDLE;
    
    vPortFree(internal_handle);
    
    return MP3_CODEC_RETURN_OK;
}

