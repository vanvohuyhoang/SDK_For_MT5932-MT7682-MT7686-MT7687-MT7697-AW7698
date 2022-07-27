/* Copyright Statement:
 *
 * (C) 2018  Airoha Technology Corp. All rights reserved.
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


/****************************************************************************
 *
 * Header files.
 *
 ****************************************************************************/

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

/* Kernel includes. */
#include <FreeRTOS.h>
#include <task.h>
#include <queue.h>

#include "cli.h"
#include "cli_def.h"
#include "task_def.h"
#include <toi.h>
#include <os.h>
#include <os_util.h>

#include "opuscelt_api.h"
#include "pcm_long.h"

#define OPUS_SIZE_320 320

#define OPUS_SIZE_80  80
#define OPUS_SIZE_120  120
#define OPUS_SIZE_160  160
#define OPUS_EN_SIZE OPUS_SIZE_80

#define OPUS_DECODE_MAX_SIZE EN_OUT_LEN_LONG_TEST //460160

#define OPUS_ENCODE_80_MAX_SIZE 115040 //(EN_OUT_LEN_LONG_TEST/320)*80
#define OPUS_ENCODE_120_MAX_SIZE 172560 //(EN_OUT_LEN_LONG_TEST/320)*120
#define OPUS_ENCODE_160_MAX_SIZE 230080 //(EN_OUT_LEN_LONG_TEST/320)*160
#define OPUS_ENCODE_MAX_SIZE OPUS_ENCODE_80_MAX_SIZE

#define OPUS_HEADER_LEN 0
//unsigned char opus_header[OPUS_HEADER_LEN] = {0};

//encoder
short en_in[OPUS_SIZE_320] = {0};
unsigned char en_out[OPUS_EN_SIZE+OPUS_HEADER_LEN] = {0};
unsigned char en_total_out[OPUS_ENCODE_MAX_SIZE] = {0}; //(EN_OUT_LEN_LONG_TEST/320+1)*80

//decoder
unsigned char de_in[OPUS_EN_SIZE+OPUS_HEADER_LEN] = {0};
short de_out[OPUS_SIZE_320] = {0};
short de_total_out[OPUS_DECODE_MAX_SIZE] = {0};//115040/80*320
//extern short g_ogg_decode_out[240960];

TaskHandle_t xHandle = NULL;
uint8_t g_opus_i2s_init = 0;
uint8_t g_op_code = 0;//0 : static pcm data, 1:static opus data, 2 : mic pcm, 3 : mic opus data, 4 : ogg arrary data
uint32_t g_total_encode_size = 0;
uint32_t g_total_decode_size = 0;

extern int parser_ogg_opus_decode_from_buf(void);
extern int ogg_opus_encode_from_buffer_and_check(char *drate, char *c);

void codec_process(int decode)//0: encode, 1:decode
{
    uint32_t complexity = 3;
    int32_t err = 0;
    uint32_t bytes_per_packet = 80;
    uint32_t out_len = 0;
    OpusCustomEncoder *enc;
    OpusCustomDecoder *dec;

    //for encode
    uint32_t en_total_len = 0;
    uint32_t en_checkerr = 0;
    int32_t en_left_len = 0;
    uint32_t en_counter = 0;

    //for decode
    uint32_t de_total_len = 0;
    uint32_t de_checkerr = 0;
    int32_t de_left_len = 0;
    uint32_t de_counter = 0;

    //encoder
    if ( decode == 0 )
    {

        err = OPUSCELT_16K_C1_F320_init(complexity, &enc);
        if (0 == err)
            printf("OPUSCELT_16K_C1_F320_init OK\n");
        else
            printf("!!! ERROR !!! OPUSCELT_16K_C1_F320_init fail !!!\n");


        en_left_len = sizeof(g_AudioPcm_Long)/sizeof(short);//EN_OUT_LEN;

        printf("encoder start -> length : %d\n", (int)en_left_len);

        g_total_encode_size = 0;
        //if we have less than 320, padding 0 to fit 320 length
        do{
            //printf("encode left size = %d\r\n", en_left_len);
            os_memset(en_in, 0, sizeof(en_in));


            if (en_left_len < OPUS_SIZE_320 )
                os_memcpy(en_in, g_AudioPcm_Long + (en_counter*OPUS_SIZE_320), sizeof(short)*en_left_len/*OPUS_SIZE_320*/);
            else
                os_memcpy(en_in, g_AudioPcm_Long + (en_counter*OPUS_SIZE_320), sizeof(short)*OPUS_SIZE_320);


            #if 0//print en_in data
            printf("en_in First 4 byte : %d  %d  %d  %d\n", en_in[0], en_in[1], en_in[2], en_in[3]);
            printf("en_in Last 4 byte  : %d  %d  %d  %d\n", en_in[316], en_in[317], en_in[318], en_in[319]);
            #endif

            //encoder proc
            os_memset(en_out, 0, sizeof(en_out));
                out_len = OPUSCELT_16K_C1_F320_proc(en_in, en_out, bytes_per_packet, enc);

            if( out_len < bytes_per_packet + OPUS_HEADER_LEN)
                printf("!!! OK !!! encoding out_len(%u) < bytes_per_packet(%u) + OPUS_HEADER_LEN !!!\n", (unsigned int)out_len, (unsigned int)bytes_per_packet);
            else if (out_len < 0)
                printf("!!! OK !!! encoding out_len(%u) is final frame !!!\n", (unsigned int)out_len);

            #if 0
            printf("en_out First 4 byte : %d  %d  %d  %d\n", en_out[0], en_out[1], en_out[2], en_out[3]);
            printf("en_out Last 4 byte  : %d  %d  %d  %d\n", en_out[76], en_out[77], en_out[78], en_out[79]);
            #endif


            //copy data to en_total_out
            os_memcpy(en_total_out+en_total_len, en_out, out_len*sizeof(unsigned char));

            en_counter++;
            en_left_len = en_left_len - OPUS_SIZE_320;
            en_total_len = en_total_len + out_len;

        } while (en_left_len > 0);//(en_left_len >= OPUS_SIZE_320);

        g_total_encode_size = en_total_len;
        printf("Encode counter : %u, Total encode len : %u \n", (unsigned int)en_counter, (unsigned int)g_total_encode_size);


        if(en_checkerr == 1)
            printf("!!! ERROR !!! Encode data is not correct, please check again !!!\n");
        else
            printf("Encode data is ok.\n");

        OPUSCELT_uninit(enc);
        printf("Encoder finish\n");
    }
    else
    {
        printf("************************************************************************************************\n");
        //decoder

        de_left_len = sizeof(en_total_out)/sizeof(unsigned char);//COMPRESS_LEN;

        printf("decoder start -> length : %d\n", (int)de_left_len);

        err = OPUSCELT_DEC_16K_C1_F320_init(&dec);
        if (0 == err)
            printf("OPUSCELT_DEC_16K_C1_F320_init(dec) OK\n");
        else
            printf("!!! ERROR !!! OPUSCELT_DEC_16K_C1_F320_init fail !!!\n");


        g_total_decode_size = 0;
        //if we do not more than 80, remove it.
        do{
            //read g_CompressData[] data to in buffer
            os_memset(de_in, 0, sizeof(de_in));
            //os_memcpy(de_in, en_total_out+(de_counter*OPUS_EN_SIZE), sizeof(unsigned char)*OPUS_EN_SIZE);
            os_memcpy(de_in, en_total_out + (de_counter*(bytes_per_packet+OPUS_HEADER_LEN)), sizeof(unsigned char)*(bytes_per_packet+OPUS_HEADER_LEN));

            #if 0//print de_in data
            printf("de_in First 4 byte : %d  %d  %d  %d\n", de_in[0], de_in[1], de_in[2], de_in[3]);
            printf("de_in Last 4 byte  : %d  %d  %d  %d\n", de_in[76], de_in[77], de_in[78], de_in[79]);
            #endif

            os_memset(de_out, 0, sizeof(de_out));
            //out_len = OPUSCELT_DEC_16K_C1_F320_proc(de_in + OPUS_HEADER_LEN - 4, de_out, bytes_per_packet);
            out_len = OPUSCELT_DEC_16K_C1_F320_proc(de_in + OPUS_HEADER_LEN, de_out, bytes_per_packet, dec);

            if ( out_len != OPUS_SIZE_320)
                printf("!!! ERROR !!! decode out_len(%u) is not equal (320) !!!\n", (unsigned int)out_len);

            //copy data to de_total_out
            os_memcpy(de_total_out+de_total_len, de_out, out_len*sizeof(short));

            de_counter++;
            de_left_len = de_left_len - (OPUS_EN_SIZE + OPUS_HEADER_LEN);
            de_total_len = de_total_len + out_len;

        }while (de_left_len >= (OPUS_EN_SIZE+OPUS_HEADER_LEN));

        g_total_decode_size = de_total_len;
        printf("Decode counter : %u, Total decode len : %u \n", (unsigned int)de_counter, (unsigned int)g_total_decode_size);

        if(de_checkerr == 1)
            printf("!!! ERROR !!! Decode data is not correct, please check again !!!\n");
        else
            printf("Decode data is ok.\n");

        OPUSCELT_DEC_uninit(dec);
        printf("Decoder finish\n");
    }
}


void play_array_opus_data(void)
{
    g_op_code = 1;

    codec_process(0);//encode
    codec_process(1);//decode

}

static void
codec_thread(void *cmd)
{

    switch ( (uint32_t)cmd )
    {
        case 1:
            play_array_opus_data();
            break;

        case 2:
            parser_ogg_opus_decode_from_buf();
            break;

        case 3:
        {
            char *drate = "64";
            char *complexity = "3";
            ogg_opus_encode_from_buffer_and_check(drate, complexity);
            break;
        }
        default :
            printf("codec_thread cmd missed\n\n");
    }


    if (xHandle != NULL) {
        vTaskSuspend( xHandle );
        vTaskDelete( xHandle );
        xHandle = NULL;
    }

}


void opus_enable(uint32_t cmd)
{
    BaseType_t xReturned;
    printf("encoder_decoder_thread\n");

    xReturned = xTaskCreate( codec_thread, OPUS_TASK_NAME, OPUS_TASK_STACKSIZE / sizeof(StackType_t), (void *)cmd, OPUS_TASK_PRIO, &xHandle );

    if( xReturned != pdPASS )
    {
        printf("create encoder_decoder_thread fail\n");
    }

}
