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
#include <math.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <string.h>
#include "oggcelt_api.h"
#include "datapcm.h"
#define MAXSIZE 2000



int ogg_opus_encode_from_buffer_and_check(char *drate, char *c)
{
    int ret,j;
    opus_int32       nb_samples;
    short            *input;
    //opus_int32       frame_size=320;
    char             *buf;
    ogg_enc_mem_t    ogg_enc;
    OpusCustomEncoder       *ogg_celt_enc;
    int length = 14233;

    short *pcmptr = INPUTPCM ;
    unsigned char *encptr = ENCDATA ;

    buf=malloc(sizeof(short)*MAXSIZE);
    if(buf==NULL){
        fprintf(stderr,"Error: couldn't allocate sample buffer.\n");
        exit(1);
    }
    ret = OGGCELT_Init(drate,c,buf,&ogg_enc, &ogg_celt_enc);
    if(ret==-1||ret==1)
        exit(1);
    //if(fwrite(buf,sizeof(char),ret,fo)!=ret)
        //printf("write file error\n");
     for(j = 0; j < ret;j++)
    {
        if(buf[j] != *encptr)
            printf("error %X %X \n",buf[j],*encptr);
           encptr++;
    }

    input=malloc(sizeof(short)*320);
    if(input==NULL){
        fprintf(stderr,"Error: couldn't allocate sample buffer.\n");
        exit(1);
    }


    while(1)
    {

        //nb_samples = (int)fread(input, sizeof(short), frame_size, fi);
        nb_samples = 320;
        length = length - 320;
        if(length < 0)
            nb_samples = nb_samples + length;

         for(j = 0; j < nb_samples;j++)
         {
             input[j] = *pcmptr++;
         }

        ret = OGGCELT_Proc(input, buf,nb_samples,&ogg_enc, ogg_celt_enc);
        if(ret > 0)
        {

            for(j = 0; j < ret;j++)
            {
                if(buf[j] != *encptr)
                    printf("error %X %X \n",buf[j],*encptr);
                encptr++;
            }
        }
            //fwrite(buf,sizeof(unsigned char),ret,fo);
        else if(ret==-1)
        {
            printf(" Buffer check OK\n");
            break;
        }

  }
  OGGCELT_Uninit(&ogg_enc,ogg_celt_enc);
  free(buf);
  free(input);
  return 0;
}



int ogg_opus_encode_from_file(char *drate, char *c, FILE *fi, FILE *fo)
{
    int ret;
    opus_int32       nb_samples;
    short            *input;
    opus_int32       frame_size=320;
    char             *buf;
    ogg_enc_mem_t    ogg_enc;
    OpusCustomEncoder       *ogg_celt_enc;
    buf=malloc(sizeof(short)*MAXSIZE);
    if(buf==NULL){
        fprintf(stderr,"Error: couldn't allocate sample buffer.\n");
        exit(1);
    }
    ret = OGGCELT_Init(drate, c, buf, &ogg_enc, &ogg_celt_enc);
    if(ret==-1||ret==1)
        exit(1);
    if(fwrite(buf,sizeof(char),ret,fo)!=ret)
        printf("write file error\n");


    input=malloc(sizeof(short)*320);
    if(input==NULL){
        fprintf(stderr,"Error: couldn't allocate sample buffer.\n");
        exit(1);
    }


    while(1)
    {
        nb_samples = (int)fread(input, sizeof(short), frame_size, fi);
        if(nb_samples < 0)
            break;
        ret = OGGCELT_Proc(input, buf, nb_samples, &ogg_enc, ogg_celt_enc);
        if(ret > 0)
            fwrite(buf,sizeof(char),ret,fo);
        else if(ret==-1)
        {
            printf(" File end\n");
            break;
        }

  }
  OGGCELT_Uninit(&ogg_enc, ogg_celt_enc);
  free(buf);
  free(input);
  return 0;
}