/* Copyright (C)2002-2011 Jean-Marc Valin
   Copyright (C)2007-2013 Xiph.Org Foundation
   Copyright (C)2008-2013 Gregory Maxwell
   File: opusenc.c

   Redistribution and use in source and binary forms, with or without
   modification, are permitted provided that the following conditions
   are met:

   - Redistributions of source code must retain the above copyright
   notice, this list of conditions and the following disclaimer.

   - Redistributions in binary form must reproduce the above copyright
   notice, this list of conditions and the following disclaimer in the
   documentation and/or other materials provided with the distribution.

   THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
   ``AS IS'' AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
   LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
   A PARTICULAR PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL THE FOUNDATION OR
   CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
   EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
   PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
   PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
   LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
   NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
   SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/






#include <stdio.h>

#include <math.h>
#include <stdlib.h>
#include <string.h>

#include <stdio.h>
#include <string.h>
# include "config_ogg.h"
#include "ogg.h"
#include "opus_oe_enc.h"
#include "opus_header.h"
#include "oggcelt_api.h"
#include "opuscelt_api.h"

#ifdef _MSC_VER
#define snprintf _snprintf
#endif
extern int ogg_page_packets(const ogg_page *og);
#define readint(buf, base) (((buf[base+3]<<24)&0xff000000)| \
                           ((buf[base+2]<<16)&0xff0000)| \
                           ((buf[base+1]<<8)&0xff00)| \
                           (buf[base]&0xff))
#define writeint(buf, base, val) do{ buf[base+3]=((val)>>24)&0xff; \
                                     buf[base+2]=((val)>>16)&0xff; \
                                     buf[base+1]=((val)>>8)&0xff; \
                                     buf[base]=(val)&0xff; \
                                 }while(0)
static void comment_init(char **comments, int* length, const char *vendor_string)
{
  /*The 'vendor' field should be the actual encoding library used.*/
  int vendor_length=strlen(vendor_string);
  int user_comment_list_length=0;
  int len=8+4+vendor_length+4;
  char *p=(char*)malloc(len);
  if(p==NULL){
    fprintf(stderr, "malloc failed in comment_init()\n");
    exit(1);
  }
  memcpy(p, "OpusTags", 8);
  writeint(p, 8, vendor_length);
  memcpy(p+12, vendor_string, vendor_length);
  writeint(p, 12+vendor_length, user_comment_list_length);
  *length=len;
  *comments=p;
}

void comment_add(char **comments, int* length, char *tag, char *val)
{
  char* p=*comments;
  int vendor_length=readint(p, 8);
  int user_comment_list_length=readint(p, 8+4+vendor_length);
  int tag_len=(tag?strlen(tag)+1:0);
  int val_len=strlen(val);
  int len=(*length)+4+tag_len+val_len;

  p=(char*)realloc(p, len);
  if(p==NULL){
    fprintf(stderr, "realloc failed in comment_add()\n");
    exit(1);
  }

  writeint(p, *length, tag_len+val_len);      /* length of comment */
  if(tag){
    memcpy(p+*length+4, tag, tag_len);        /* comment tag */
    (p+*length+4)[tag_len-1] = '=';           /* separator */
  }
  memcpy(p+*length+4+tag_len, val, val_len);  /* comment */
  writeint(p, 8+4+vendor_length, user_comment_list_length+1);
  *comments=p;
  *length=len;
}
static void comment_pad(char **comments, int* length, int amount)
{
  if(amount>0){
    int i;
    int newlen;
    char* p=*comments;
    /*Make sure there is at least amount worth of padding free, and
       round up to the maximum that fits in the current ogg segments.*/
    newlen=(*length+amount+255)/255*255-1;
    p=realloc(p,newlen);
    if(p==NULL){
      fprintf(stderr,"realloc failed in comment_pad()\n");
      exit(1);
    }
    for(i=*length;i<newlen;i++)p[i]=0;
    *comments=p;
    *length=newlen;
  }
}
/*Write an Ogg page to a file pointer*/
static int oe_write_page(ogg_page *page, char *out)
{
   int written = 0,i;
#if 0
   written=fwrite(page->header,1,page->header_len, fp);
   written+=fwrite(page->body,1,page->body_len, fp);
#endif
   for(i = 0; i < page->header_len;i++)
   {
	   *out++ = *page->header++;written++;
   }
   for(i = 0; i < page->body_len;i++)
   {
	   *out++ = *page->body++;written++;
   }
   return written;
}

#define MAX_FRAME_BYTES 61295
#define IMIN(a,b) ((a) < (b) ? (a) : (b))   /**< Minimum int value.   */
#define IMAX(a,b) ((a) > (b) ? (a) : (b))   /**< Maximum int value.   */

int OGGCELT_Init(char *drate,char *c,char *out,ogg_enc_mem_t *ogg_enc, OpusCustomEncoder **enc)
{

	int ret,err;
    unsigned char header_data[100];
 
    int packet_size=opus_header_to_packet(&ogg_enc->header, header_data, 100);

    char               ENCODER_string[1024];
	oe_enc_opt         inopt;
    int                cline_size;
    /*Counters*/
	opus_int64         nb_encoded=0;
	int                bytes_written=0;
	opus_int64         pages_out=0;
	opus_int64         total_bytes=0;
	opus_int64         total_samples=0;
	opus_int32         nbBytes;
	opus_int32         peak_bytes=0;
	char *optarg;
	 int                comment_padding=512;
	 int complexity;

     int                serialno;
	 const char         *opus_version;
	 char framesize[] = "20";

     ogg_int32_t        id=-1;
     int                eos=0;
	 opus_int32         frame_size=320;

	 ogg_int64_t        last_granulepos=0;
     ogg_int64_t        enc_granulepos=0;
     ogg_int64_t        original_samples=0;
     opus_int32         coding_rate=16000;
     int                last_segments=0;
	 int                max_ogg_delay=48000; /*48kHz samples*/
//	 opus_int16 *in,*out;
	 int                quiet=0;
//	 time_t             start_time;
//     time_t             stop_time;
     time_t             last_spin=0;
	 int                last_spin_len=0;
	 int                with_hard_cbr=0;
	
//     int bytes_per_packet;
	 memset(ogg_enc,0,sizeof(*ogg_enc));
	 //command 
	 with_hard_cbr = 1;
	 //header information
     ogg_enc->header.channels = 1;
	 ogg_enc->header.gain = 0;
	 ogg_enc->header.input_sample_rate = 16000;
	 ogg_enc->header.channel_mapping = 0;
	 ogg_enc->header.nb_streams = 1;
	 ogg_enc->header.preskip = 0x78;
	 ogg_enc->header.nb_coupled = 0;
	
	
	 serialno=0;//rand();
	 opus_version=opus_get_version_string();
    /*Vendor string should just be the encoder library,
    the ENCODER comment specifies the tool used.*/
     comment_init(&inopt.comments, &inopt.comments_length, opus_version);
    snprintf(ENCODER_string, sizeof(ENCODER_string), "opusenc from %s %s",PACKAGE_NAME,PACKAGE_VERSION);
     comment_add(&inopt.comments, &inopt.comments_length, "ENCODER", ENCODER_string);
	 cline_size = 0;
     ret=snprintf(&ENCODER_string[cline_size], sizeof(ENCODER_string)-cline_size, "%s--%s",cline_size==0?"":" ","bitrate");
      cline_size = 9; optarg = drate;
	 ret=snprintf(&ENCODER_string[cline_size], sizeof(ENCODER_string)-cline_size, " %s",optarg);
     cline_size = 0xc;
	 ret=snprintf(&ENCODER_string[cline_size], sizeof(ENCODER_string)-cline_size, "%s--%s",cline_size==0?"":" ","hard-cbr");
      cline_size = 0x17;
	 ret=snprintf(&ENCODER_string[cline_size], sizeof(ENCODER_string)-cline_size, "%s--%s",cline_size==0?"":" ","comp");
        cline_size = 0x1e; optarg = c;
	 ret=snprintf(&ENCODER_string[cline_size], sizeof(ENCODER_string)-cline_size, " %s",optarg);
     cline_size = 0x20;
	 ret=snprintf(&ENCODER_string[cline_size], sizeof(ENCODER_string)-cline_size, "%s--%s",cline_size==0?"":" ","framesize");
    cline_size = 0x2c; optarg = (char *) &framesize;
	 ret=snprintf(&ENCODER_string[cline_size], sizeof(ENCODER_string)-cline_size, " %s",optarg);
  
    comment_add(&inopt.comments, &inopt.comments_length, "ENCODER_OPTIONS", ENCODER_string);
  
	
	if(ogg_stream_init(&ogg_enc->os, serialno)==-1){
      fprintf(stderr,"Error: stream init failed\n");
      exit(1);
    }

	packet_size=opus_header_to_packet(&ogg_enc->header, header_data, 100);

    ogg_enc->op.packet=header_data;
    ogg_enc->op.bytes=packet_size;
    ogg_enc->op.b_o_s=1;
    ogg_enc->op.e_o_s=0;
    ogg_enc->op.granulepos=0;
	
    nbBytes = packet_size;
   
    //op.packetno=0;
    ogg_stream_packetin(&ogg_enc->os, &ogg_enc->op);
    complexity = atoi(c);
    err = OPUSCELT_16K_C1_F320_init(complexity,enc);
	if(err==-1)
	 {
		 printf("complexity should be 0:3 \n");
	
		return -1;
	}
    else if (err != 0)
	 {
		fprintf(stderr, "Failed to create the encoder: %s\n");
	
		return 1;
	}

    while((ret=ogg_stream_flush(&ogg_enc->os, &ogg_enc->og))){
      if(!ret)break;
      ret=oe_write_page(&ogg_enc->og, out);
      if(ret!=ogg_enc->og.header_len+ogg_enc->og.body_len){
        fprintf(stderr,"Error: failed writing header to output stream\n");
        exit(1);
      }
      bytes_written+=ret;
      pages_out++;
    }

    comment_pad(&inopt.comments, &inopt.comments_length, comment_padding);
    ogg_enc->op.packet=(unsigned char *)inopt.comments;
    ogg_enc->op.bytes=inopt.comments_length;
    ogg_enc->op.b_o_s=0;
    ogg_enc->op.e_o_s=0;
    ogg_enc->op.granulepos=0;
    ogg_enc->op.packetno=1;
    ogg_stream_packetin(&ogg_enc->os, &ogg_enc->op);
  

 /* writing the rest of the opus header packets */
  while((ret=ogg_stream_flush(&ogg_enc->os, &ogg_enc->og))){
    if(!ret)break;
    ret=oe_write_page(&ogg_enc->og, out+bytes_written);
    if(ret!=ogg_enc->og.header_len + ogg_enc->og.body_len){
      fprintf(stderr,"Error: failed writing header to output stream\n");
      exit(1);
    }
    bytes_written+=ret;
    pages_out++;
  }
 free(inopt.comments); 

 ogg_enc->nbCompressedBytes = (int)(2.5 *(opus_int32)atol(drate));
  printf(" ogg_enc->nbCompressedBytes = %d %d %d\n", ogg_enc->nbCompressedBytes,*drate,*(drate+1));
 ogg_enc->nb_samples = -1;
 ogg_enc->id = -1;
 ogg_enc->dextra = 0x68;
 ogg_enc->min_bytes= MAX_FRAME_BYTES;
return bytes_written;
}



int OGGCELT_Proc(short *pcm, char *compressed, int nbsamples, ogg_enc_mem_t *ogg_enc, OpusCustomEncoder *enc)
{

	int ret = 0;
   
    /*Counters*/

	//local
	 opus_int32         frame_size=320;
	 opus_int32         coding_rate=16000;
	 int                max_ogg_delay=960*5; /*48kHz samples*/
	 opus_int32         nbBytes;
	 int nbCompressedBytes;
	 nbCompressedBytes = ogg_enc->nbCompressedBytes;
    printf("  nbCompressedBytes = %d\n", nbCompressedBytes);
  if(!ogg_enc->op.e_o_s){
    int size_segments,i,cur_frame_size;
	
    ogg_enc->id++;
  printf("  ogg_enc->id = %d\n", ogg_enc->id);
    if(ogg_enc->nb_samples<0){
	  ogg_enc->nb_samples = nbsamples;
	   ogg_enc->original_samples+=ogg_enc->nb_samples;
      printf("  1 ogg_enc->nb_samples = %d ogg_enc->original_samples = %d\n", ogg_enc->nb_samples,ogg_enc->original_samples);
	   if(ogg_enc->nb_samples<320)
			 ogg_enc->nb_samples = ogg_enc->dextra+ogg_enc->nb_samples;
	   if(ogg_enc->nb_samples>320)
	   {
		   ogg_enc->dextra =  ogg_enc->nb_samples - 320;
           ogg_enc->nb_samples = 320;
	   }
      ogg_enc->total_samples+=ogg_enc->nb_samples;
      if(ogg_enc->nb_samples<frame_size)
		  ogg_enc->op.e_o_s=1;
      else ogg_enc->op.e_o_s=0;
    }
    ogg_enc->op.e_o_s|=ogg_enc->eos;

    cur_frame_size=frame_size;

    /*No fancy end padding, just fill with zeros for now.*/
    if(nbsamples<cur_frame_size)for(i=nbsamples;i<cur_frame_size;i++)pcm[i]=0;
     
     nbBytes = OPUSCELT_16K_C1_F320_proc(pcm, (unsigned char*)compressed, nbCompressedBytes,enc);
    
    /*Encode current frame*/
    printf("  nbBytes = %d\n",  nbBytes);
    ogg_enc->nb_encoded+=cur_frame_size;
    ogg_enc->enc_granulepos+=cur_frame_size*48000/coding_rate;
    ogg_enc->total_bytes+=nbBytes;
    size_segments=(nbBytes+255)/255;
    ogg_enc->peak_bytes=IMAX(nbBytes,ogg_enc->peak_bytes);
    ogg_enc->min_bytes=IMIN(nbBytes,ogg_enc->min_bytes);
 
    /*Flush early if adding this packet would make us end up with a
      continued page which we wouldn't have otherwise.*/
    while((((size_segments<=255)&&(ogg_enc->last_segments+size_segments>255))||
           (ogg_enc->enc_granulepos-ogg_enc->last_granulepos>max_ogg_delay))&& ogg_stream_flush_fill(&ogg_enc->os, &ogg_enc->og,255*255)){
      if(ogg_page_packets(&ogg_enc->og)!=0)ogg_enc->last_granulepos=ogg_page_granulepos(&ogg_enc->og);
      ogg_enc->last_segments-=ogg_enc->og.header[26];
      ret=oe_write_page(&ogg_enc->og, compressed);
      if(ret!=ogg_enc->og.header_len+ogg_enc->og.body_len){
         fprintf(stderr,"Error: failed writing data to output stream\n");
         exit(1);
      }
      ogg_enc->bytes_written+=ret;
      ogg_enc->pages_out++;
    }

    /*The downside of early reading is if the input is an exact
      multiple of the frame_size you'll get an extra frame that needs
      to get cropped off. The downside of late reading is added delay.
      If your ogg_delay is 120ms or less we'll assume you want the
      low delay behavior.*/
	
    if((!ogg_enc->op.e_o_s)&&max_ogg_delay>5760){
	 
	  ogg_enc->nb_samples = nbsamples;
	   ogg_enc->original_samples+=ogg_enc->nb_samples;
	   if(ogg_enc->nb_samples<320)
			 ogg_enc->nb_samples = ogg_enc->dextra+ogg_enc->nb_samples;
	   if(ogg_enc->nb_samples>320)
	   {
		   ogg_enc->dextra =  ogg_enc->nb_samples - 320;
           ogg_enc->nb_samples = 320;
	   }
      ogg_enc->total_samples+=ogg_enc->nb_samples;

      if(ogg_enc->nb_samples<frame_size)
		  ogg_enc->eos=1;
      if(ogg_enc->nb_samples==0)
		  ogg_enc->op.e_o_s=1;
    } else ogg_enc->nb_samples=-1;

    ogg_enc->op.packet=(unsigned char *)compressed;
    ogg_enc->op.bytes=nbBytes;
    ogg_enc->op.b_o_s=0;
    ogg_enc->op.granulepos=ogg_enc->enc_granulepos;
    if(ogg_enc->op.e_o_s){
      /*We compute the final GP as ceil(len*48k/input_rate). When a resampling
        decoder does the matching floor(len*input/48k) conversion the length will
        be exactly the same as the input.*/
      ogg_enc->op.granulepos=((ogg_enc->original_samples*48000+coding_rate-1)/coding_rate)+ogg_enc->header.preskip;
    }
    ogg_enc->op.packetno=2+ogg_enc->id;
    ogg_stream_packetin(&ogg_enc->os, &ogg_enc->op);
    ogg_enc->last_segments+= size_segments;

    /*If the stream is over or we're sure that the delayed flush will fire,
      go ahead and flush now to avoid adding delay.*/
   printf("ogg_enc->op.e_o_s = %d ogg_enc->enc_granulepos = %d frame_size = %d coding_rate = %d ogg_enc->last_granulepos = %d max_ogg_delay = %d\n",ogg_enc->op.e_o_s,ogg_enc->enc_granulepos,frame_size,coding_rate,ogg_enc->last_granulepos,max_ogg_delay);
    while((ogg_enc->op.e_o_s||(ogg_enc->enc_granulepos+(frame_size*48000/coding_rate)-ogg_enc->last_granulepos>max_ogg_delay)||
           (ogg_enc->last_segments>=255))?
#ifdef OLD_LIBOGG
    /*Libogg > 1.2.2 allows us to achieve lower overhead by
      producing larger pages. For 20ms frames this is only relevant
      above ~32kbit/sec.*/
           ogg_stream_flush(&os, &og):
           ogg_stream_pageout(&os, &og)){
#else
           ogg_stream_flush_fill(&ogg_enc->os, &ogg_enc->og,255*255):
           ogg_stream_pageout_fill(&ogg_enc->os, &ogg_enc->og,255*255)){
#endif
      if(ogg_page_packets(&ogg_enc->og)!=0)ogg_enc->last_granulepos=ogg_page_granulepos(&ogg_enc->og);
      ogg_enc->last_segments-=ogg_enc->og.header[26];
      ret=oe_write_page(&ogg_enc->og, compressed);
	  printf("ret = %d\n",ret);
      if(ret!=ogg_enc->og.header_len+ogg_enc->og.body_len){
         fprintf(stderr,"Error: failed writing data to output stream\n");
         exit(1);
      }
      ogg_enc->bytes_written+=ret;
      ogg_enc->pages_out++;
    }
  
  }
 if(ogg_enc->op.e_o_s==1&&ret==0)
	 ret = -1;
return ret;
}
void OGGCELT_Uninit(ogg_enc_mem_t *ogg_mem, OpusCustomEncoder *enc)
{
	ogg_stream_clear(&ogg_mem->os);
	OPUSCELT_uninit(enc);
	
}