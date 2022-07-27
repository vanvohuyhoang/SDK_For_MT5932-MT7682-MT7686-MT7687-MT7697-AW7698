/* Copyright Statement:
 *
 * (C) 2005-2016 MediaTek Inc. All rights reserved.
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


#include "amr_decoder.h"
#ifdef MTK_AUDIO_AMR_ENABLED
#include "amr_decoder_internal.h"
#include "hal_audio_internal_service.h"
#include "memory_attribute.h"

#ifdef HAL_I2S_MODULE_ENABLED
#include "hal_i2s.h"
#include "nau8810.h"
#include "hal_i2c_master.h"
#else
#error "HAL_I2S_MODULE_ENABLED no defined !!"
#endif

//#define AMR_FILE                                        /*For Storage Format*/

#ifdef AMR_FILE
#define AMR_MAGIC_NUMBER "#!AMR\n"
#endif
#define PCM_FRAME_SIZE            (160*2)              /*PCM frame size in bytes  , keep 20ms at 8k fs.  (Fix do not modify) */
#define AMR_PCM_STREAM_OUT_SIZE   (PCM_FRAME_SIZE*4)
#define MAX_AMR_FRAME_SIZE        (31+1)               /*Maximum dpacked_size+1 byte toc*/
#define I2S_TX_VFIFO_LENGTH       320
//#define MAX_SERIAL_SIZE           244                   /* max. num. of serial bits.  (Fix do not modify) */

const int16_t dpacked_size[16] = {12, 13, 15, 17, 19, 20, 26, 31, 5, 0, 0, 0, 0, 0, 0, 0};

ATTR_ZIDATA_IN_NONCACHED_RAM_4BYTE_ALIGN static uint32_t I2S_TX_VFIFO[I2S_TX_VFIFO_LENGTH];

//IPCOMMON_PLUS


static amr_decoder_internal_handle_t *amr_decoder_internal_handle = NULL;
static QueueHandle_t amr_decoder_queue_handle = NULL;
static uint8_t amr_decoder_queue_reg_num = 0;
static amr_decoder_queue_event_id_t amr_decoder_queue_event_id_array[MAX_AMR_DECODER_FUNCTIONS];
static amr_decoder_internal_callback_t amr_decoder_queue_handler[MAX_AMR_DECODER_FUNCTIONS];
static TaskHandle_t amr_decoder_task_handle = NULL;
static hal_i2s_port_t i2s_port = HAL_I2S_1;
static uint32_t amr_decode_check_threshold = 0;

static void amr_decoder_deocde_hisr_handler(void *data);
static void amr_decoder_event_send_from_isr(amr_decoder_queue_event_id_t id, void *parameter);
static void amr_decoder_event_register_callback(amr_decoder_queue_event_id_t reg_id, amr_decoder_internal_callback_t callback);

static void amr_decoder_delete_memory_buffer(void)
{
    amr_decoder_internal_handle_t *internal_handle = amr_decoder_internal_handle;

    if (internal_handle->dec_int_buffer != NULL) {
        vPortFree(internal_handle->dec_int_buffer);
        internal_handle->dec_int_buffer = NULL;
    }

    if (internal_handle->dec_tmp_buffer != NULL) {
        vPortFree(internal_handle->dec_tmp_buffer);
        internal_handle->dec_tmp_buffer = NULL;
    }

    if (internal_handle->dec_pcm_buffer != NULL) {
        vPortFree(internal_handle->dec_pcm_buffer);
        internal_handle->dec_pcm_buffer = NULL;
    }

    if (internal_handle->dec_bs_buffer != NULL) {
        vPortFree(internal_handle->dec_bs_buffer);
        internal_handle->dec_bs_buffer = NULL;
    }

    if (internal_handle->stream_out_pcm_buff.buffer_base_pointer != NULL) {
        vPortFree(internal_handle->stream_out_pcm_buff.buffer_base_pointer);
        internal_handle->stream_out_pcm_buff.buffer_base_pointer = NULL;
    }

    if (internal_handle != NULL) {
        vPortFree(internal_handle);
        internal_handle = NULL;
    }

}

/*play amr by nau8810 codec + i2s*/
static void amr_decoder_i2s_tx_callback(hal_i2s_event_t event, void *user_data)
{
    hal_i2s_disable_tx_dma_interrupt_ex(i2s_port);
    amr_decoder_internal_handle_t *internal_handle = amr_decoder_internal_handle;
    amr_decoder_handle_t *handle = &internal_handle->handle;

    uint32_t tx_vfifo_space = 0; //in sample
    uint32_t consume_samples = 0; //in sample
    uint32_t i2s_data = 0;
    uint32_t i2s_last_data = 0;
    uint32_t stream_out_pcm_buff_data_count = 0;

    switch (event) {
        case HAL_I2S_EVENT_END:
            handle->handler(handle, AMR_DECODER_MEDIA_EVENT_END);
            break;
        case HAL_I2S_EVENT_DATA_REQUEST: {
            hal_i2s_get_tx_sample_count_ex(i2s_port, &tx_vfifo_space);
            for (uint8_t loop_idx = 0; loop_idx < 2; loop_idx++) {
                uint8_t *stream_out_pcm_buff_ptr = NULL;
                stream_out_pcm_buff_data_count = 0;
                ring_buffer_get_read_information(&internal_handle->stream_out_pcm_buff, &stream_out_pcm_buff_ptr, &stream_out_pcm_buff_data_count);
                stream_out_pcm_buff_data_count >>= 1; // 2bytes for mono
                //LOGI("idx=%d tx_vfifo=%d , stream_out=%d\r\n", loop_idx, tx_vfifo_space, stream_out_pcm_buff_data_count);
                if ((tx_vfifo_space > 0) && (stream_out_pcm_buff_data_count > 0)) {
                    consume_samples = MINIMUM(tx_vfifo_space, stream_out_pcm_buff_data_count);
                    //LOGI("consume=%d \r\n",consume_samples);
                    for (uint32_t i = 0; i < consume_samples; i++) {
                        memcpy(&i2s_data, stream_out_pcm_buff_ptr, 2);
                        ring_buffer_read_done(&internal_handle->stream_out_pcm_buff, 2);
                        hal_i2s_tx_write_ex(i2s_port, i2s_data);
                        //LOGI("i=%d i2s_data=%x \r\n",i, i2s_data);
                        i2s_last_data = i2s_data;
                        stream_out_pcm_buff_ptr += 2;
                    }
                    tx_vfifo_space -= consume_samples;
                }
            }

            if (tx_vfifo_space >= (PCM_FRAME_SIZE >> 1)) {
                //LOGW("underflow, vfifo(%d) pcm(%d)\r\n",tx_vfifo_space,stream_out_pcm_buff_data_count);
                for (uint32_t i = 0; i < ((PCM_FRAME_SIZE >> 1) - consume_samples); i++) {
                    hal_i2s_tx_write_ex(i2s_port, i2s_last_data);
                }
            }
        }
        break;
    };

    if ((ring_buffer_get_space_byte_count(&internal_handle->stream_out_pcm_buff)) >= PCM_FRAME_SIZE) {
        amr_decoder_event_send_from_isr(AMR_DECODER_QUEUE_EVENT_DECODE, NULL);
    }

    hal_i2s_enable_tx_dma_interrupt_ex(i2s_port);

}


static void amr_decoder_i2s_tx_enable(void)
{
    LOGMSGIDI("[CTRL]amr_decoder_i2s_tx_enable\r\n", 0);
    hal_i2s_enable_tx_ex(i2s_port);
    hal_i2s_enable_audio_top_ex(i2s_port);
}


static void amr_decoder_i2s_tx_disable(void)
{
    LOGMSGIDI("[CTRL]amr_decoder_i2s_tx_disable\r\n", 0);
    hal_i2s_disable_tx_ex(i2s_port);
    hal_i2s_disable_audio_top_ex(i2s_port);
    hal_i2s_deinit_ex(i2s_port);
    hal_i2s_stop_tx_vfifo_ex(i2s_port);
    hal_i2s_stop_rx_vfifo_ex(i2s_port);
}


static int8_t amr_decoder_i2s_configure(void)
{
    LOGMSGIDI("[CTRL]amr_decoder_i2s_configure \r\n", 0);
    hal_i2s_config_t i2s_config;
    hal_i2s_status_t result = HAL_I2S_STATUS_OK;

    result = hal_i2s_init_ex(i2s_port, HAL_I2S_TYPE_EXTERNAL_MODE);
    if (HAL_I2S_STATUS_OK != result) {
        LOGMSGIDE("hal_i2s_init failed\r\n", 0);
        return -1;
    }

    /* Configure I2S  */
    i2s_config.clock_mode = HAL_I2S_MASTER;
    i2s_config.sample_width = HAL_I2S_SAMPLE_WIDTH_16BIT;
    i2s_config.frame_sync_width = HAL_I2S_FRAME_SYNC_WIDTH_32;
    i2s_config.tx_mode = HAL_I2S_TX_DUPLICATE_DISABLE;

    i2s_config.i2s_out.channel_number = HAL_I2S_STEREO;
    i2s_config.i2s_in.channel_number = HAL_I2S_STEREO;
    i2s_config.i2s_out.sample_rate = HAL_I2S_SAMPLE_RATE_8K;
    i2s_config.i2s_in.sample_rate  = HAL_I2S_SAMPLE_RATE_8K;
    i2s_config.i2s_in.msb_offset   = 0;
    i2s_config.i2s_out.msb_offset  = 0;
    i2s_config.i2s_in.word_select_inverse  = HAL_I2S_WORD_SELECT_INVERSE_DISABLE;
    i2s_config.i2s_out.word_select_inverse = HAL_I2S_WORD_SELECT_INVERSE_DISABLE;
    i2s_config.i2s_in.lr_swap  = HAL_I2S_LR_SWAP_DISABLE;
    i2s_config.i2s_out.lr_swap = HAL_I2S_LR_SWAP_DISABLE;

    result = hal_i2s_set_config_ex(i2s_port, &i2s_config);
    if (HAL_I2S_STATUS_OK != result) {
        LOGMSGIDE("hal_i2s_set_config failed\r\n", 0);
        return -1;
    }

    result = hal_i2s_setup_tx_vfifo_ex(i2s_port, I2S_TX_VFIFO, I2S_TX_VFIFO_LENGTH / 2, I2S_TX_VFIFO_LENGTH);
    if (HAL_I2S_STATUS_OK != result) {
        LOGMSGIDE("hal_i2s_setup_tx_vfifo failed\r\n", 0);
        return -1;
    }

    return 1;
}


static int8_t amr_decoder_nau8810_configure(void)
{
    LOGMSGIDI("[CTRL]amr_nau8810_configure \r\n", 0);

    /*configure NAU8810*/
    AUCODEC_STATUS_e codec_status;
    hal_i2c_port_t i2c_port;
    hal_i2c_frequency_t frequency;

    codec_status = AUCODEC_STATUS_OK;
    i2c_port = HAL_I2C_MASTER_1;
    frequency = HAL_I2C_FREQUENCY_50K;

    codec_status = aucodec_i2c_init(i2c_port, frequency);
    if (codec_status != AUCODEC_STATUS_OK) {
        LOGMSGIDE("aucodec_i2c_init failed\r\n", 0);
    }

    aucodec_softreset();//soft reset

    codec_status = aucodec_init();
    if (codec_status != AUCODEC_STATUS_OK) {
        LOGMSGIDE("aucodec_init failed\r\n", 0);
    }

    aucodec_set_dai_fmt(eI2S, e16Bit, eBCLK_NO_INV);
    codec_status = aucodec_set_dai_sysclk(eSR8KHz, eSLAVE, e32xFS, 24576000, ePLLEnable);//24576000
    if (codec_status != AUCODEC_STATUS_OK) {
        LOGMSGIDE("aucodec_set_dai_sysclk failed\r\n", 0);
    }

    //aucodec_set_output(eSpkOut);//Input: DACIN, Output:  speaker out
    aucodec_set_output(eLineOut);//Input: DACIN, Output:  aux out

    //must deinit i2c after configuring codec
    aucodec_i2c_deinit();

    if (codec_status == AUCODEC_STATUS_OK) {
        return 1;
    } else {
        return -1;
    }

}

static int8_t amr_decoder_nau8810_close(void)
{
    LOGMSGIDI("[CTRL]amr_decoder_nau8810_close \r\n", 0);

    /*configure NAU8810*/
    AUCODEC_STATUS_e codec_status;
    hal_i2c_port_t i2c_port;
    hal_i2c_frequency_t frequency;

    codec_status = AUCODEC_STATUS_OK;
    i2c_port = HAL_I2C_MASTER_1;
    frequency = HAL_I2C_FREQUENCY_50K;

    codec_status = aucodec_i2c_init(i2c_port, frequency);
    if (codec_status != AUCODEC_STATUS_OK) {
        LOGMSGIDE("aucodec_i2c_init failed\r\n", 0);
    }

    aucodec_softreset();//soft reset

    //must deinit i2c after configuring codec
    aucodec_i2c_deinit();

    if (codec_status == AUCODEC_STATUS_OK) {
        return 1;
    } else {
        return -1;
    }

}



/*	share buffer operation function */
static void amr_decoder_set_share_buffer(amr_decoder_handle_t *handle, uint8_t *buffer, uint32_t length)
{
    handle->share_buffer.buffer_base = buffer;
    length &= ~0x1; // make buffer size even
    handle->share_buffer.buffer_size = length;
    handle->share_buffer.write = 0;
    handle->share_buffer.read = 0;
    handle->waiting = false;
    handle->underflow = false;
}


static void amr_decoder_get_write_buffer(amr_decoder_handle_t *handle, uint8_t **buffer, uint32_t *length)
{
    int32_t count = 0;

    if (handle->share_buffer.read > handle->share_buffer.write) {
        count = handle->share_buffer.read - handle->share_buffer.write - 1;
    } else if (handle->share_buffer.read == 0) {
        count = handle->share_buffer.buffer_size - handle->share_buffer.write - 1;
    } else {
        count = handle->share_buffer.buffer_size - handle->share_buffer.write;
    }
    *buffer = handle->share_buffer.buffer_base + handle->share_buffer.write;
    *length = count;
}


static void amr_decoder_get_read_buffer(amr_decoder_handle_t *handle, uint8_t **buffer, uint32_t *length)
{
    int32_t count = 0;

    if (handle->share_buffer.write >= handle->share_buffer.read) {
        count = handle->share_buffer.write - handle->share_buffer.read;
    } else {
        count = handle->share_buffer.buffer_size - handle->share_buffer.read;
    }
    *buffer = handle->share_buffer.buffer_base + handle->share_buffer.read;
    *length = count;
}


static void amr_decoder_write_data_done(amr_decoder_handle_t *handle, uint32_t length)
{
    amr_decoder_internal_handle_t *internal_handle = amr_decoder_internal_handle;
    uint32_t pcm_out_space_in_bytes = ring_buffer_get_space_byte_count(&internal_handle->stream_out_pcm_buff);

    handle->share_buffer.write += length;
    if (handle->share_buffer.write == handle->share_buffer.buffer_size) {
        handle->share_buffer.write = 0;
    }

    if (pcm_out_space_in_bytes >= PCM_FRAME_SIZE) {
        amr_decoder_event_send_from_isr(AMR_DECODER_QUEUE_EVENT_DECODE, NULL);
    }
}


static void amr_decoder_finish_write_data(amr_decoder_handle_t *handle)
{
    handle->waiting = false;
    handle->underflow = false;
}


static void amr_decoder_reset_share_buffer(amr_decoder_handle_t *handle)
{
    //memset(handle->share_buffer.buffer_base, 0, handle->share_buffer.buffer_size);  // do this or it will have previous data.
    handle->share_buffer.write = 0;
    handle->share_buffer.read = 0;
    handle->waiting = false;
    handle->underflow = false;
}


static void amr_decoder_read_data_done(amr_decoder_handle_t *handle, uint32_t length)
{
    handle->share_buffer.read += length;
    if (handle->share_buffer.read == handle->share_buffer.buffer_size) {
        handle->share_buffer.read = 0;
    }
}


static int32_t amr_decoder_get_free_space(amr_decoder_handle_t *handle)
{
    int32_t count = 0;

    count = handle->share_buffer.read - handle->share_buffer.write - 2;
    if (count < 0) {
        count += handle->share_buffer.buffer_size;
    }
    return count;
}


static int32_t amr_decoder_get_data_count(amr_decoder_handle_t *handle)
{
    int32_t count = 0;

    count = handle->share_buffer.write - handle->share_buffer.read;
    if (count < 0) {
        count += handle->share_buffer.buffer_size;
    }
    return count;
}


static void amr_decoder_reset_pcm_out_buffer(void)
{
    amr_decoder_internal_handle_t *internal_handle = amr_decoder_internal_handle;
    internal_handle->stream_out_pcm_buff.read_pointer = 0;
    internal_handle->stream_out_pcm_buff.write_pointer = 0;
}


static void amr_decoder_buffer_function_init(amr_decoder_handle_t *handle)
{
    handle->set_share_buffer       = amr_decoder_set_share_buffer;
    handle->get_write_buffer       = amr_decoder_get_write_buffer;
    handle->get_read_buffer        = amr_decoder_get_read_buffer;
    handle->write_data_done        = amr_decoder_write_data_done;
    handle->finish_write_data      = amr_decoder_finish_write_data;
    handle->reset_share_buffer     = amr_decoder_reset_share_buffer;
    handle->read_data_done         = amr_decoder_read_data_done;
    handle->get_free_space         = amr_decoder_get_free_space;
    handle->get_data_count         = amr_decoder_get_data_count;
}


static void amr_decoder_event_send_from_isr(amr_decoder_queue_event_id_t id, void *parameter)
{
    if (amr_decoder_queue_handle == NULL) {
        return;
    }
    amr_decoder_queue_event_t event;
    event.id        = id;
    event.parameter = parameter;
    if (xQueueSendFromISR(amr_decoder_queue_handle, &event, 0) != pdPASS) {
        //LOGI("queue not pass , id=%d\r\n", id);
        return;
    }

    return;
}


static void amr_decoder_event_register_callback(amr_decoder_queue_event_id_t reg_id, amr_decoder_internal_callback_t callback)
{
    uint32_t id_idx;
    for (id_idx = 0; id_idx < MAX_AMR_DECODER_FUNCTIONS; id_idx++) {
        if (amr_decoder_queue_event_id_array[id_idx] == AMR_DECODER_QUEUE_EVENT_NONE) {
            amr_decoder_queue_event_id_array[id_idx] = reg_id;
            amr_decoder_queue_handler[id_idx] = callback;
            amr_decoder_queue_reg_num++;
            break;
        }
    }
    return;
}


static void amr_decoder_event_deregister_callback(amr_decoder_queue_event_id_t dereg_id)
{
    LOGMSGIDI("[CTRL]deregister HISR callback \r\n", 0);
    uint32_t id_idx;
    for (id_idx = 0; id_idx < MAX_AMR_DECODER_FUNCTIONS; id_idx++) {
        if (amr_decoder_queue_event_id_array[id_idx] == dereg_id) {
            amr_decoder_queue_event_id_array[id_idx] = AMR_DECODER_QUEUE_EVENT_NONE;
            amr_decoder_queue_reg_num--;
            break;
        }
    }
    return;
}


static void amr_decoder_task_main(void *arg)
{
    LOGMSGIDI("amr_decoder_task_main\r\n", 0);

    amr_decoder_queue_event_t event;
    amr_decoder_queue_handle = xQueueCreate(AMR_DECODER_QUEUE_SIZE, sizeof(amr_decoder_queue_event_t));

    /* Initialize queue registration */
    uint8_t id_idx;
    for (id_idx = 0; id_idx < MAX_AMR_DECODER_FUNCTIONS; id_idx++) {
        amr_decoder_queue_event_id_array[id_idx] = AMR_DECODER_QUEUE_EVENT_NONE;
    }

    while (1) {
        if (xQueueReceive(amr_decoder_queue_handle, &event, portMAX_DELAY)) {
            amr_decoder_queue_event_id_t rece_id = event.id;
            //LOGI("rece_id=%d\n",rece_id);
            uint8_t id_idx;
            for (id_idx = 0; id_idx < MAX_AMR_DECODER_FUNCTIONS; id_idx++) {
                if (amr_decoder_queue_event_id_array[id_idx] == rece_id) {
                    //LOGI("find queue event id\n");
                    amr_decoder_queue_handler[id_idx](event.parameter);
                    break;
                }
            }
        }
    }
}


void amr_decoder_task_create(void)
{
    if (amr_decoder_task_handle ==  NULL) {
        LOGMSGIDI("create amr decoder task\r\n", 0);
        xTaskCreate(amr_decoder_task_main, AMR_DECODER_TASK_NAME, AMR_DECODER_TASK_STACKSIZE / sizeof(StackType_t), NULL, AMR_DECODER_TASK_PRIO, &amr_decoder_task_handle);
    }

}


amr_decoder_status_t amr_decoder_get_memory_size(uint32_t *memory_size)
{
    uint32_t dec_int_buf_size = 0;
    uint32_t dec_tmp_buf_size = 0;
    uint32_t dec_bs_buf_size = 0;
    uint32_t dec_pcm_buf_size = 0;

    if (amr_decoder_internal_handle == NULL) {
        LOGMSGIDE("amr_decoder_get_memory_size error,  amr_decoder_internal_handle should not be NULL\r\n", 0);
        return AMR_DECODER_STATUS_ERROR;
    }

    AMRDec_GetBufferSize((unsigned int *)&dec_int_buf_size,
                         (unsigned int *)&dec_tmp_buf_size,
                         (unsigned int *)&dec_bs_buf_size,
                         (unsigned int *)&dec_pcm_buf_size);

    *memory_size = dec_int_buf_size +
                   dec_tmp_buf_size +
                   dec_bs_buf_size +
                   dec_pcm_buf_size +
                   AMR_PCM_STREAM_OUT_SIZE +
                   I2S_TX_VFIFO_LENGTH * 4;

    return AMR_DECODER_STATUS_OK;
}

static void amr_decoder_deocde_hisr_handler(void *data)
{
    uint32_t  remain = 0;
    uint32_t  consume = 0;
    uint8_t   *share_buff_read_ptr = NULL;
    uint8_t   *P_dst = NULL;
    uint8_t   *P_src = NULL;
    uint32_t  share_buff_data_len = 0;
    uint32_t  total_share_buff_data_len = 0;

    uint8_t   toc = 0;
    uint8_t   quality_bit = 0;
    uint8_t   frame_type = 0;

    amr_decoder_internal_handle_t *internal_handle = amr_decoder_internal_handle;
    amr_decoder_handle_t *handle = &internal_handle->handle;
    if (((ring_buffer_get_space_byte_count(&internal_handle->stream_out_pcm_buff)) < PCM_FRAME_SIZE) || (handle->state != AMR_DECODER_STATE_DECODING)) {
        return;
    }

    if (amr_decode_check_threshold == 0) {
        amr_decode_check_threshold = MAX_AMR_FRAME_SIZE * 10;
    }

    total_share_buff_data_len = handle->get_data_count(handle);
    if ((total_share_buff_data_len <= amr_decode_check_threshold) && (internal_handle->flush_data_flag == 0)) {
        if (!handle->waiting) {
            handle->waiting = true;
            handle->handler(handle, AMR_DECODER_MEDIA_EVENT_REQUEST);
        }
    }

    /*Fnd TOC*/
    remain = 1; //1bytes
    P_dst = (uint8_t *)&toc;
    for (uint32_t loop_idx = 0; loop_idx < 2; loop_idx++) {
        if (remain == 0) {
            break;
        }
        handle->get_read_buffer(handle, &share_buff_read_ptr, &share_buff_data_len);
        if (share_buff_data_len > 0) {
            consume = MINIMUM(share_buff_data_len, remain);
            memcpy(P_dst, share_buff_read_ptr, consume);
            {
                /* check data in share buffer is enough for dpacked_size */
                quality_bit  = (uint8_t)(toc >> 2) & 0x01;
                frame_type = (uint8_t)(toc >> 3) & 0x0F;
                //LOGI("quality_bit=%x , frame_type=%x , dpacked_size=%d \r\n", quality_bit, frame_type, dpacked_size[frame_type]);
                //amr_decode_check_threshold = dpacked_size[frame_type];
                total_share_buff_data_len = handle->get_data_count(handle);
                if (total_share_buff_data_len < dpacked_size[frame_type]) {
                    LOGMSGIDW("share_buff(%d) < dpacked_size(%d)\r\n",2, total_share_buff_data_len, dpacked_size[frame_type]);
                    handle->handler(handle, AMR_DECODER_MEDIA_EVENT_ERROR);
                    return ;
                }
            }
            handle->read_data_done(handle, consume);
            remain -= consume;
            P_dst += consume;
        }
    }

    if ((remain != 0) || (dpacked_size[frame_type] == 0)) {
        LOGMSGIDE("TOC not found!\r\n", 0);
        handle->handler(handle, AMR_DECODER_MEDIA_EVENT_ERROR);
        return;
    }

    /*Read data from share buffer to decode*/
    remain = dpacked_size[frame_type];
    P_dst = (uint8_t *)internal_handle->dec_bs_buffer;
    for (uint32_t loop_idx = 0; loop_idx < 2; loop_idx++) {
        if (remain == 0) {
            break;
        }
        handle->get_read_buffer(handle, &share_buff_read_ptr, &share_buff_data_len);
        if (share_buff_data_len > 0) {
            consume = MINIMUM(share_buff_data_len, remain);
            memcpy(P_dst, share_buff_read_ptr, consume);
            handle->read_data_done(handle, consume);
            remain -= consume;
            P_dst += consume;
        }
    }

    /*decode*/
    AMR_Decode(internal_handle->amr_dec_handle,
               internal_handle->dec_tmp_buffer,
               internal_handle->dec_pcm_buffer,
               internal_handle->dec_bs_buffer,
               quality_bit, frame_type);

    /* Write pcm to stream_out_buffer */
    remain = PCM_FRAME_SIZE;
    uint32_t free_space = 0;
    P_src = (uint8_t *)internal_handle->dec_pcm_buffer;
    for (uint32_t loop_idx = 0; loop_idx < 2; loop_idx++) {
        if (remain == 0) {
            break;
        }
        ring_buffer_get_write_information(&internal_handle->stream_out_pcm_buff, &P_dst, &free_space);
        if (free_space > 0) {
            consume = MINIMUM(free_space, remain);
            //LOGI(" consume=%d \n",consume);
            memcpy(P_dst, P_src, consume);
            ring_buffer_write_done(&internal_handle->stream_out_pcm_buff, consume);
            P_src += consume;
            remain -= consume;
        }
    }

    if (remain != 0) {
        LOGMSGIDE("stream_out_pcm_buff space not enough for 1 frame, remain=%d!\n\r",1, remain);
    }

    //uint32_t pcm_out_space_in_bytes = ring_buffer_get_space_byte_count(&internal_handle->stream_out_pcm_buff);
    //if (pcm_out_space_in_bytes >= PCM_FRAME_SIZE) {
    //    amr_decoder_event_send_from_isr(AMR_DECODER_QUEUE_EVENT_DECODE, NULL);
    //}

}

static amr_decoder_status_t amr_decoder_flush(amr_decoder_handle_t *handle, int32_t flush_data_flag)
{
    LOGMSGIDI("[CTRL]amr_decoder_flush \r\n", 0);
    amr_decoder_internal_handle_t *internal_handle = amr_decoder_internal_handle;

    internal_handle->flush_data_flag = flush_data_flag;
    hal_i2s_set_eof_ex(i2s_port);

    return AMR_DECODER_STATUS_OK;
}

static amr_decoder_status_t amr_decoder_pause(amr_decoder_handle_t *handle)
{
    LOGMSGIDI("[CTRL]amr_decoder_pause \r\n", 0);

    if (handle->state != AMR_DECODER_STATE_DECODING) {
        LOGMSGIDE("codec state != AMR_DECODER_STATE_DECODING\r\n", 0);
        return AMR_DECODER_STATUS_ERROR;
    }

    hal_i2s_disable_tx_dma_interrupt_ex(i2s_port);
    hal_i2s_disable_tx_ex(i2s_port);
    hal_i2s_disable_audio_top_ex(i2s_port);

    handle->state = AMR_DECODER_STATE_PAUSE;
    return AMR_DECODER_STATUS_OK;
}


static amr_decoder_status_t amr_decoder_resume(amr_decoder_handle_t *handle)
{
    LOGMSGIDI("[CTRL]amr_decoder_resume \r\n", 0);

    if (handle->state != AMR_DECODER_STATE_PAUSE) {
        LOGMSGIDE("codec state != AMR_DECODER_STATE_PAUSE\r\n", 0);
        return AMR_DECODER_STATUS_ERROR;
    }

    hal_i2s_enable_tx_ex(i2s_port);
    hal_i2s_enable_audio_top_ex(i2s_port);
    hal_i2s_enable_tx_dma_interrupt_ex(i2s_port);

    handle->state = AMR_DECODER_STATE_DECODING;
    return AMR_DECODER_STATUS_OK;
}


static amr_decoder_status_t amr_decoder_start(amr_decoder_handle_t *handle)
{
    LOGMSGIDI("[CTRL]amr_decoder_start \r\n", 0);

    if (handle->state != AMR_DECODER_STATE_READY && handle->state != AMR_DECODER_STATE_STOP) {
        LOGMSGIDE("amr_decoder_start error, state=%d\r\n",1, handle->state);
        return AMR_DECODER_STATUS_ERROR;
    }

    uint32_t remain = 0;
    uint32_t consume = 0;
    uint32_t share_buff_data_len = 0;
    uint32_t total_share_buff_data_len = 0;
    uint8_t *share_buff_read_ptr = NULL;
    uint8_t *P_dst = NULL;
    uint8_t *P_src = NULL;

    int8_t ret = 0;

    /*decoder use*/
    uint8_t toc = 0; // table of content
    uint8_t quality_bit = 0;
    uint8_t frame_type = 0;

    amr_decoder_internal_handle_t *internal_handle = (amr_decoder_internal_handle_t *) handle;
    amr_decoder_event_register_callback(AMR_DECODER_QUEUE_EVENT_DECODE, amr_decoder_deocde_hisr_handler);

    internal_handle->error_cnt = 0;
    internal_handle->flush_data_flag = 0;
    internal_handle->media_bitstream_end = 0;

#ifdef AMR_FILE
    int8_t magic[8];
    /*Find magic number*/
    memset(magic, 0, 8);
    remain = 6; //magic number is 6bytes
    P_dst = (uint8_t *)magic;
    for (uint32_t loop_idx = 0; loop_idx < 2; loop_idx++) {
        if (remain == 0) {
            break;
        }
        handle->get_read_buffer(handle, &share_buff_read_ptr, &share_buff_data_len);
        if (share_buff_data_len > 0) {
            consume = MINIMUM(share_buff_data_len, remain);
            memcpy(P_dst, share_buff_read_ptr, consume);
            handle->read_data_done(handle, consume);
            remain -= consume;
            P_dst += consume;
        }
    }

    if (memcmp(magic, AMR_MAGIC_NUMBER, 6)) {
        printf("Invalid magic number: magic=%s!\n\r", magic);
        return AMR_DECODER_STATUS_ERROR;
    } else {
        printf("Find magic number: magic=%s!\n\r", magic);
    }
#endif

    /*Assign buffer and initialize*/
    internal_handle->amr_dec_handle = AMRDec_Init(internal_handle->dec_int_buffer);
    if (internal_handle->amr_dec_handle == NULL) {
        LOGMSGIDE("amr_dec_handle is NULL\r\n", 0);
        return AMR_DECODER_STATUS_ERROR;
    }

    handle->state = AMR_DECODER_STATE_DECODING;
    while ((ring_buffer_get_space_byte_count(&internal_handle->stream_out_pcm_buff)) >= PCM_FRAME_SIZE) {

        /*Fnd TOC*/
        remain = 1; //1bytes
        P_dst = (uint8_t *)&toc;
        for (uint32_t loop_idx = 0; loop_idx < 2; loop_idx++) {
            if (remain == 0) {
                break;
            }
            handle->get_read_buffer(handle, &share_buff_read_ptr, &share_buff_data_len);
            if (share_buff_data_len > 0) {
                consume = MINIMUM(share_buff_data_len, remain);
                memcpy(P_dst, share_buff_read_ptr, consume);
                {
                    /* check data in share buffer is enough for dpacked_size */
                    quality_bit  = (uint8_t)(toc >> 2) & 0x01;
                    frame_type = (uint8_t)(toc >> 3) & 0x0F;
                    LOGMSGIDI("quality_bit=%x , frame_type=%x , dpacked_size=%d \r\n",3, quality_bit, frame_type, dpacked_size[frame_type]);
                    //amr_decode_check_threshold = dpacked_size[frame_type];
                    total_share_buff_data_len = handle->get_data_count(handle);
                    if (total_share_buff_data_len < dpacked_size[frame_type]) {
                        LOGMSGIDW("share_buff(%d) < dpacked_size(%d)\r\n",2, total_share_buff_data_len, dpacked_size[frame_type]);
                        break;
                    }
                }
                handle->read_data_done(handle, consume);
                remain -= consume;
                P_dst += consume;
            }
        }

        if ((remain != 0) || (dpacked_size[frame_type] == 0)) {
            LOGMSGIDE("TOC not found!\r\n", 0);
            break;
        }

        {
            /*Read data from share buffer to decode*/
            remain = dpacked_size[frame_type];
            P_dst = (uint8_t *)internal_handle->dec_bs_buffer;
            for (uint32_t loop_idx = 0; loop_idx < 2; loop_idx++) {
                if (remain == 0) {
                    break;
                }
                handle->get_read_buffer(handle, &share_buff_read_ptr, &share_buff_data_len);
                if (share_buff_data_len > 0) {
                    consume = MINIMUM(share_buff_data_len, remain);
                    memcpy(P_dst, share_buff_read_ptr, consume);
                    handle->read_data_done(handle, consume);
                    remain -= consume;
                    P_dst += consume;
                }
            }
        }

        /*decode*/
        AMR_Decode(internal_handle->amr_dec_handle,
                   internal_handle->dec_tmp_buffer,
                   internal_handle->dec_pcm_buffer,
                   internal_handle->dec_bs_buffer,
                   quality_bit, frame_type);

        /* Write pcm to stream_out_buffer */
        remain = PCM_FRAME_SIZE;
        uint32_t free_space = 0;
        P_src = (uint8_t *)internal_handle->dec_pcm_buffer;
        for (uint32_t loop_idx = 0; loop_idx < 2; loop_idx++) {
            if (remain == 0) {
                break;
            }
            ring_buffer_get_write_information(&internal_handle->stream_out_pcm_buff, &P_dst, &free_space);
            if (free_space > 0) {
                consume = MINIMUM(free_space, remain);
                memcpy(P_dst, P_src, consume);
                ring_buffer_write_done(&internal_handle->stream_out_pcm_buff, consume);
                P_src += consume;
                remain -= consume;
            }
        }

        if (remain != 0) {
            LOGMSGIDE("[Write pcm]remain=%d!\n\r",1, remain);
            return AMR_DECODER_STATUS_ERROR;
        }
    }

    ret = amr_decoder_nau8810_configure();
    if (ret == -1) {
        LOGMSGIDE("amr_decoder_nau8810_configure failed---\n", 0);
    }

    ret = amr_decoder_i2s_configure();
    if (ret == -1) {
        LOGMSGIDE("amr_decoder_i2s_configure failed---\n", 0);
    }

    hal_i2s_register_tx_vfifo_callback_ex(i2s_port, amr_decoder_i2s_tx_callback, NULL);
    amr_decoder_i2s_tx_enable();
    hal_i2s_enable_tx_dma_interrupt_ex(i2s_port);

    return AMR_DECODER_STATUS_OK;
}

static amr_decoder_status_t amr_decoder_stop(amr_decoder_handle_t *handle)
{
    LOGMSGIDI("[CTRL]amr_decoder_stop \r\n", 0);

    if (handle->state != AMR_DECODER_STATE_READY  &&
            handle->state != AMR_DECODER_STATE_DECODING &&
            handle->state != AMR_DECODER_STATE_PAUSE) {
        LOGMSGIDE("[CTRL]amr_decoder_stop failed, state=%d\r\n",1, handle->state);
        return AMR_DECODER_STATUS_ERROR;
    }

    hal_i2s_disable_tx_dma_interrupt_ex(i2s_port);
    amr_decoder_i2s_tx_disable();
    amr_decoder_nau8810_close();
    handle->reset_share_buffer(handle);
    amr_decoder_reset_pcm_out_buffer();
    amr_decoder_event_deregister_callback(AMR_DECODER_QUEUE_EVENT_DECODE);
    handle->state = AMR_DECODER_STATE_STOP;
    return AMR_DECODER_STATUS_OK;
}

amr_decoder_handle_t *amr_decoder_open(amr_decoder_callback_t amr_decoder_callback)
{
    LOGMSGIDI("[CTRL]amr_decoder_open \r\n", 0);

    amr_decoder_handle_t *handle;
    amr_decoder_internal_handle_t *internal_handle = NULL;

    /* alloc internal handler space and initialize it*/
    internal_handle = (amr_decoder_internal_handle_t *)pvPortMalloc(sizeof(amr_decoder_internal_handle_t));
    if (internal_handle == NULL) {
        LOGMSGIDE("Malloc internal_handle failed\r\n", 0);
        return NULL;
    }
    memset(internal_handle, 0, sizeof(amr_decoder_internal_handle_t));

    /* assign internal handler to be global and static handler*/
    amr_decoder_internal_handle = internal_handle;

    /* initialize handle*/
    handle = &internal_handle->handle;
    handle->handler = amr_decoder_callback;
    handle->audio_id = 0xbeef;

    /*decoder*/
    handle->decoder_start = amr_decoder_start;
    handle->decoder_stop  = amr_decoder_stop;
    handle->decoder_pause = amr_decoder_pause;
    handle->decoder_resume = amr_decoder_resume;
    handle->decoder_flush = amr_decoder_flush;

    /*initialize buffer control function*/
    amr_decoder_buffer_function_init(handle);

    /*create codec task*/
    amr_decoder_task_create();

    AMRDec_GetBufferSize((unsigned int *)&internal_handle->dec_int_buf_size,
                         (unsigned int *)&internal_handle->dec_tmp_buf_size,
                         (unsigned int *)&internal_handle->dec_bs_buf_size,
                         (unsigned int *)&internal_handle->dec_pcm_buf_size);

    /*allocate memory for stream out pcm*/
    internal_handle->stream_out_pcm_buff_size = AMR_PCM_STREAM_OUT_SIZE;
    internal_handle->stream_out_pcm_buff.buffer_byte_count = internal_handle->stream_out_pcm_buff_size;
    internal_handle->stream_out_pcm_buff.read_pointer = 0;
    internal_handle->stream_out_pcm_buff.write_pointer = 0;
    internal_handle->stream_out_pcm_buff.buffer_base_pointer  = (uint8_t *)pvPortMalloc(internal_handle->stream_out_pcm_buff_size);
    if (internal_handle->stream_out_pcm_buff.buffer_base_pointer == NULL) {
        LOGMSGIDE("Malloc stream_out_pcm_buff failed\r\n", 0);
        amr_decoder_delete_memory_buffer();
        return NULL;
    }

    /*allocate memory for decoder*/
    internal_handle->dec_int_buffer = (uint8_t *)pvPortMalloc(internal_handle->dec_int_buf_size);
    if (internal_handle->dec_int_buffer == NULL) {
        LOGMSGIDE("Malloc dec_int_buffer failed\r\n", 0);
        amr_decoder_delete_memory_buffer();
        return NULL;
    }

    internal_handle->dec_tmp_buffer = (uint8_t *)pvPortMalloc(internal_handle->dec_tmp_buf_size);
    if (internal_handle->dec_tmp_buffer == NULL) {
        LOGMSGIDE("Malloc dec_tmp_buffer failed\r\n", 0);
        amr_decoder_delete_memory_buffer();
        return NULL;
    }

    internal_handle->dec_pcm_buffer = (int16_t *)pvPortMalloc(internal_handle->dec_pcm_buf_size);
    if (internal_handle->dec_pcm_buffer == NULL) {
        LOGMSGIDE("Malloc dec_pcm_buffer failed\r\n", 0);
        amr_decoder_delete_memory_buffer();
        return NULL;
    }

    internal_handle->dec_bs_buffer  = (uint8_t *)pvPortMalloc(internal_handle->dec_bs_buf_size);
    if (internal_handle->dec_bs_buffer == NULL) {
        LOGMSGIDE("Malloc dec_bs_buffer failed\r\n", 0);
        amr_decoder_delete_memory_buffer();
        return NULL;
    }

    LOGMSGIDI("internal_handle->dec_int_buffer=%x size=%d\r\n",2, internal_handle->dec_int_buffer , internal_handle->dec_int_buf_size);
    LOGMSGIDI("internal_handle->dec_tmp_buffer=%x size=%d\r\n",2, internal_handle->dec_tmp_buffer , internal_handle->dec_tmp_buf_size);
    LOGMSGIDI("internal_handle->dec_pcm_buffer=%x size=%d\r\n",2, internal_handle->dec_pcm_buffer , internal_handle->dec_pcm_buf_size);
    LOGMSGIDI("internal_handle->dec_bs_buffer=%x size=%d\r\n",2, internal_handle->dec_bs_buffer , internal_handle->dec_bs_buf_size);
    LOGMSGIDI("internal_handle->stream_out_pcm_buff.buffer_base_pointer=%x size=%d\r\n",2, internal_handle->stream_out_pcm_buff.buffer_base_pointer , internal_handle->stream_out_pcm_buff_size);

    /*codec state*/
    handle->state = AMR_DECODER_STATE_READY;

    return handle;
}


amr_decoder_status_t amr_decoder_close(amr_decoder_handle_t *handle)
{
    LOGMSGIDI("[CTRL]amr_decoder_close \r\n", 0);

    if (handle->state != AMR_DECODER_STATE_STOP && handle->state != AMR_DECODER_STATE_READY) {
        return AMR_DECODER_STATUS_ERROR;
    }

    handle->state = AMR_DECODER_STATE_IDLE;

    if (amr_decoder_task_handle != NULL) {
        vTaskDelete(amr_decoder_task_handle);
        amr_decoder_task_handle = NULL;
    }

    if (amr_decoder_queue_handle != NULL) {
        vQueueDelete(amr_decoder_queue_handle);
        amr_decoder_queue_handle = NULL;
    }

    amr_decoder_delete_memory_buffer();
    amr_decoder_internal_handle = NULL;
    return AMR_DECODER_STATUS_OK;

}


#endif/*MTK_AUDIO_AMR_DECODER_ENABLED*/
