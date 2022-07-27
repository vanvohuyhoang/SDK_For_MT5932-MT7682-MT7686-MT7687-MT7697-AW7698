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


#include <stdio.h>
#include <string.h>
#include "file_fmt.h"
#include "oggcelt_api.h"
#include "opuscelt_api.h"
#include <os.h>

void Wave_Write_Header(WAVE_Header *wave_header, ogg_dec_mem_t *ogg_mem, unsigned int fo_len, FILE *fo)
{
    unsigned char *pt;

    fseek(fo,0,SEEK_SET);

    fo_len = (wave_header->NumChannels ==  2) ? fo_len>>1 : fo_len;

    pt = wave_header->riff;              *pt++ = 'R'; *pt++ = 'I'; *pt++ = 'F'; *pt++ = 'F';
    pt = wave_header->wave;              *pt++ = 'W'; *pt++ = 'A'; *pt++ = 'V'; *pt++ = 'E';
    pt = wave_header->fmt_chunk_marker ; *pt++ = 'f'; *pt++ = 'm'; *pt++ = 't'; *pt++ = 0x20;
    pt = wave_header->data_chunk_header; *pt++ = 'd'; *pt++ = 'a'; *pt++ = 't'; *pt++ = 'a';
    wave_header->NumChannels    = ogg_mem->channels;
    wave_header->SampleRate     = ogg_mem->samplerate;
    wave_header->length_of_fmt  = 16;
    wave_header->BitsPerSample  = 16;
    wave_header->format_type    = 1;
    wave_header->ByteRate       = (wave_header->SampleRate) * (wave_header->NumChannels) * (wave_header->BitsPerSample)/8;
    wave_header->BlockAlign     = (wave_header->NumChannels) * (wave_header->BitsPerSample)/8;
    wave_header->data_size      = fo_len * (wave_header->NumChannels) * (wave_header->BitsPerSample)/8;
    wave_header->overall_size   = (wave_header->data_size) + 36;

    fwrite(wave_header,sizeof(WAVE_Header),1,fo);
};

void toc_dec(unsigned char data, int *mode, int *framesize, int Fs)
{
   if (data&0x80) {
       *mode = MODE_CELT_ONLY;
       *framesize = ((data>>3)&0x3);
       *framesize = (Fs<<*framesize)/400;
   } else if ((data&0x60) == 0x60) {
       *mode = MODE_HYBRID;
       *framesize = (data&0x08) ? Fs/50 : Fs/100;
   } else {
       *mode = MODE_SILK_ONLY;
       *framesize = ((data>>3)&0x3);
       if (*framesize == 3) *framesize = Fs*60/1000;
       else                 *framesize = (Fs<<*framesize)/100;
   }
};

int ogg_format_report(char *argv, ogg_dec_mem_t *ogg_mem, int *framesize)
{
    int             mode, report = OGG_OK;
    unsigned char   data = ogg_mem->op.packet[0];

    toc_dec(data, &mode, framesize, ogg_mem->samplerate);

    if (*framesize != OPUS_FrameeSize) {
        printf("The frame size of %s file is not %d samples.\n", argv, OPUS_FrameeSize);
        return -1;
    }

    if (ogg_mem->channels != OPUS_Channel) {
        printf("The %s file is not channel = %d.\n",argv, OPUS_Channel);
        return -2;
    }

    if (ogg_mem->samplerate != OPUS_SampleRate) {
        printf("The %s file is not %d sample rate.\n",argv, OPUS_SampleRate);
        return -3;
    }

    if (mode != MODE_CELT_ONLY) {
        if (++ogg_mem->mode_cnt > 10) {
            printf("The %s file is not CELT decode.\n",argv);
            return -4;
        }
    } else
        report = OGG_packet_celt_mode;

    return  report;
};

int ogg_opus_decode(char *finame, FILE *fi, FILE *fo)
{
    WAVE_Header             wave_header;
    OpusCustomDecoder       *ogg_celt_dec;
    ogg_dec_mem_t           ogg_mem;
    char                    buf_in[OPUS_BufInSize];
    short                   buf_ou[OPUS_FrameeSize];
    int                     status, skip, framesize, frame_cntr, fo_total_len;

    skip         = 0;
    fo_total_len = 0;
    frame_cntr   = 0;

    //===================== init celt decoder ========================
    status = OPUSCELT_DEC_16K_C1_F320_init(&ogg_celt_dec);
    if (status != OGG_OK) {
        printf ("Failed to create the celt decoder. (status = %d)\n", status);
        return -2;
    }
    //================================================================

    //===================== init process 1 ===========================
    status = OGG_PARSER_INIT(&ogg_mem);
    if (status != OGG_OK) {
        printf ("OGG INIT1 error. (status = %d)\n", status);
        OPUSCELT_DEC_uninit(ogg_celt_dec);
        return -3;
    }
    //================================================================

    //===================== init process 2 ===========================
    do {
        if ( ogg_mem.byte_consumed != 0 ) {
            ogg_mem.byte_read = (int)fread(buf_in + OPUS_BufInSize - ogg_mem.byte_consumed, sizeof(char), ogg_mem.byte_consumed, fi);
        }
        status = OGG_PARSER_PROC(buf_in, &ogg_mem);
        if (status != OGG_OK) {
            printf ("OGG INIT2 error. (status = %d)\n", status);
            OPUSCELT_DEC_uninit(ogg_celt_dec);
            #ifdef OGG_DEC_ALLOC_EN
            OGG_PARSER_UNINIT(&ogg_mem);
            #endif
            return -4;
        }
    } while (!ogg_mem.init_done);
    //================================================================

    //===================== decode loop ===========================
    do {
        if ( ogg_mem.byte_consumed != 0 ) {
            ogg_mem.byte_read = (int)fread(buf_in + OPUS_BufInSize - ogg_mem.byte_consumed, sizeof(char), ogg_mem.byte_consumed, fi);
        }

        status = OGG_PARSER_PROC(buf_in, &ogg_mem);
        if (status == OGG_page_crc_chksum_err) {
            printf ("\nOGG Page CRC CheckSum Error.\n");
            continue;
        } else if (status == OGG_page_sequence_err) {
            printf ("\nOGG Page Sequence Error.\n");
            continue;
        } else if (status < 0) {
            printf ("\nOGG Parser error.  (status = %d)\n", status);
            OPUSCELT_DEC_uninit(ogg_celt_dec);
            #ifdef OGG_DEC_ALLOC_EN
            OGG_PARSER_UNINIT(&ogg_mem);
            #endif
            return -5;
        } else if (status != OGG_packet_ready)
            continue;

        status = ogg_format_report(finame, &ogg_mem, &framesize);
        if ( status < 0) {
            printf ("\nOGG Format error.\n");
            OPUSCELT_DEC_uninit(ogg_celt_dec);
            #ifdef OGG_DEC_ALLOC_EN
            OGG_PARSER_UNINIT(&ogg_mem);
            #endif
            return  -6;
        } else if (status != OGG_packet_celt_mode)
            continue;

#if 0   // Test packet loss, eeee
        if ((frame_cntr >= 34) && (frame_cntr <= 39))
            status = OPUSCELT_DEC_16K_C1_F320_proc(NULL, buf_ou, ogg_mem.op.bytes, ogg_celt_dec);
        else
            status = OPUSCELT_DEC_16K_C1_F320_proc(ogg_mem.op.packet, buf_ou, ogg_mem.op.bytes, ogg_celt_dec);
        if (frame_cntr >= 200) break;
#else
        status = OPUSCELT_DEC_16K_C1_F320_proc(ogg_mem.op.packet, buf_ou, ogg_mem.op.bytes, ogg_celt_dec);
#endif

        if (status < 0) {
            printf ("\nOGG CELT Decoder error.\n");
            OPUSCELT_DEC_uninit(ogg_celt_dec);
            #ifdef OGG_DEC_ALLOC_EN
            OGG_PARSER_UNINIT(&ogg_mem);
            #endif
            return -7;
        }
        fo_total_len += (framesize - skip);

        fwrite(buf_ou+skip, 2, framesize-skip, fo);
        skip = 0;

        printf("CELT Decoder <frame_cntr = %03d>\r", ++frame_cntr);

    } while (!ogg_mem.exec_done);

    printf("\n");

    OPUSCELT_DEC_uninit(ogg_celt_dec);

#ifdef OGG_DEC_ALLOC_EN
    OGG_PARSER_UNINIT(&ogg_mem);
#endif

    Wave_Write_Header(&wave_header, &ogg_mem, fo_total_len, fo);

    return OGG_OK;
};

char* g_ogg_inFileName = "input.opus";
char* g_ogg_outFileName = "output.wav";
void parser_ogg_opus_decode_from_file()
{
    int f_error = 0;
    FILE *fi, *fo;
    if ( (fi = fopen(g_ogg_inFileName,"rb")) == NULL) {
        f_error = 1;
        printf("The %s file is not existence.\n", g_ogg_inFileName);
    }

    if ( (fo = fopen(g_ogg_outFileName,"wb")) == NULL) {
        f_error = 1;;
        printf("The %s file is not be opened.\n", g_ogg_outFileName);
    }

    if ( f_error == 0 )
    {
        fseek(fo, sizeof(WAVE_Header), SEEK_SET);
        ogg_opus_decode(g_ogg_inFileName, fi, fo);
        printf(" Finish ... !\n\n");
    }
    else
        printf("Error : please put a file and name is as input.opus for parsering !!!\n");

    if ( fi != NULL )
        fclose(fi);

    if ( fo != NULL )
        fclose(fo);

}

#include "ogg_arrary.h"
short g_ogg_decode_out[240960] = {0};
int parser_ogg_opus_decode_from_buf(void)
{
    OpusCustomDecoder   *ogg_celt_dec;
    int                 status = 0;
    ogg_dec_mem_t       ogg_mem;
    char                buf_in[OPUS_BufInSize] = {0};
    short               buf_ou[OPUS_FrameeSize] = {0};
    int                 out_size = 0;
    int                 framesize = 0;
    int                 fo_total_len = 0;

    //===================== init celt decoder ========================
    status = OPUSCELT_DEC_16K_C1_F320_init(&ogg_celt_dec);
    if (status != OGG_OK)
    {
        printf ("Failed to create the celt decoder. (status = %d)\n", status);
        return -2;
    }
    //================================================================


    //===================== init process 1 ===========================
    status = OGG_PARSER_INIT(&ogg_mem);
    if (status != OGG_OK)
    {
        printf ("OGG INIT1 error. (status = %d)\n", status);
        OPUSCELT_DEC_uninit(ogg_celt_dec);
        return -3;
    }
    //================================================================

    //===================== init process 2 ===========================
    int total_cp_size = 0;
    int loop_counter = 0;
    do {
        if ( ogg_mem.byte_consumed != 0 )
        {
            int copy_size = OGG_TESF_SIZE - total_cp_size;
            if ( copy_size > OPUS_BufInSize )
                copy_size = OPUS_BufInSize;

            if ( copy_size > 0 )
            {
                os_memset(buf_in, 0, OPUS_BufInSize);
                os_memcpy(buf_in, g_ogg_test_buf + total_cp_size, copy_size);
                ogg_mem.byte_read = copy_size;
                total_cp_size  = total_cp_size + copy_size;
                printf("Init process 2 -> copy size : %d, Already copy size : %d \r\n", copy_size, total_cp_size);
            }
            else
            {
                printf("ERROR : copy size < 0 !!!!\r\n");
                ogg_mem.byte_read = 0;//if copy is completed, should let byte_read = 0
            }
        }
        else
            printf("Init process 2 : wait ogg consumption !!! \r\n");

        loop_counter++;
        status = OGG_PARSER_PROC(buf_in, &ogg_mem);
        printf("Init process 2, Loop counter : %d \r\n", loop_counter);
        if (status != OGG_OK) {
            printf ("OGG INIT2 error. (status = %d)\n", status);
            OPUSCELT_DEC_uninit(ogg_celt_dec);
            #ifdef OGG_DEC_ALLOC_EN
            OGG_PARSER_UNINIT(&ogg_mem);
            #endif
            return -4;
        }
    }while (!ogg_mem.init_done);
    printf("Init process 2 -> total copy size : %d\r\n", total_cp_size);

    //===================== decode loop ===========================
    //total_cp_size = 0;//does not copy in the beginning, continue copy from init process 2
    loop_counter = 0;
    do {
        if ( ogg_mem.byte_consumed != 0 )
        {
            int copy_size = OGG_TESF_SIZE - total_cp_size;
            if ( copy_size > OPUS_BufInSize )
                copy_size = OPUS_BufInSize;

            if ( copy_size > 0 )
            {
                printf("Decode -> copy size : %d \r\n", copy_size);
                os_memset(buf_in, 0, OPUS_BufInSize);
                os_memcpy(buf_in, g_ogg_test_buf + total_cp_size, copy_size);
                ogg_mem.byte_read = copy_size;
                total_cp_size  = total_cp_size + copy_size;
                printf("Decode -> copy size : %d, Already copy size : %d \r\n", copy_size, total_cp_size);
            }
            else
            {
                printf("copy size < 0 !!!!\r\n");
                ogg_mem.byte_read = 0;//if copy is completed, should let byte_read = 0
            }
        }
        else
            printf("Decode : wait ogg consumption !!! \r\n");

        status = OGG_PARSER_PROC(buf_in, &ogg_mem);
        if (status == OGG_page_crc_chksum_err) {
            printf ("\nOGG Page CRC CheckSum Error.\n");
            continue;
        } else if (status == OGG_page_sequence_err) {
            printf ("\nOGG Page Sequence Error.\n");
            continue;
        } else if (status < 0) {
            printf ("\nOGG Parser error.  (status = %d)\n", status);
            OPUSCELT_DEC_uninit(ogg_celt_dec);
            #ifdef OGG_DEC_ALLOC_EN
            OGG_PARSER_UNINIT(&ogg_mem);
            #endif
            return -5;
        } else if (status != OGG_packet_ready)
            continue;

        status = ogg_format_report("ogg_test", &ogg_mem, &framesize);
        if ( status < 0) {
            printf ("\nOGG Format error.\n");
            OPUSCELT_DEC_uninit(ogg_celt_dec);
            #ifdef OGG_DEC_ALLOC_EN
            OGG_PARSER_UNINIT(&ogg_mem);
            #endif
            return  -6;
        } else if (status != OGG_packet_celt_mode)
            continue;
        loop_counter++;
        out_size = OPUSCELT_DEC_16K_C1_F320_proc(ogg_mem.op.packet, buf_ou, ogg_mem.op.bytes, ogg_celt_dec);
        if (status < 0 || out_size < 320 ) {
            printf ("\nOGG CELT Decoder error.(out size : %d)\n", out_size);
            OPUSCELT_DEC_uninit(ogg_celt_dec);
            #ifdef OGG_DEC_ALLOC_EN
            OGG_PARSER_UNINIT(&ogg_mem);
            #endif
            return -7;
        }
        os_memcpy(g_ogg_decode_out + fo_total_len, buf_ou, out_size*sizeof(short));
        fo_total_len += out_size;
        printf("Decode counter : %d, total decode length : %d \r\n", loop_counter, fo_total_len);
    } while (!ogg_mem.exec_done);
    printf("total decode size : %d \r\n", fo_total_len);
    OPUSCELT_DEC_uninit(ogg_celt_dec);
    #ifdef OGG_DEC_ALLOC_EN
    OGG_PARSER_UNINIT(&ogg_mem);
    #endif

    return 0;
}
