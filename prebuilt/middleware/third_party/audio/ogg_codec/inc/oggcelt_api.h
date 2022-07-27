#ifndef __OGGCELT_API_H__
#define __OGGCELT_API_H__
#include "opus_header.h"
#include "opus_types.h"
#include "opus_custom.h"

typedef struct {
    long			gain;
	int				has_opus_stream;
	int				has_tags_packet;
	int				total_links;
	int				stream_init;
	opus_int64		packet_count, page_granule, link_out;
	int				opus_serialno;
	int				eos;
	ogg_sync_state	 oy;
    ogg_page		 og;
    ogg_packet		 op;
    ogg_stream_state os;
	// parser status	
	int				byte_read;
	int				byte_consumed;	
	
	int				preskip;
	int				mode_cnt;	
	int				samplerate;
	int				channels;	
	ogg_int64_t     dec_granulepos;
	ogg_int64_t     cur_samples;

	int				init_done;
	int				exec_done;	
} ogg_dec_mem_t; 

typedef struct {

	OpusHeader         header;
 
	ogg_stream_state   os;
    ogg_page           og;
	ogg_packet         op;
	int dextra;
	int nb_samples;
	int nbCompressedBytes;
	int				eos;
	ogg_int32_t        id;
	opus_int64         nb_encoded;
	opus_int64         bytes_written;
	opus_int64         pages_out;
	opus_int64         total_bytes;
	opus_int64         total_samples;

	opus_int32         peak_bytes;
	opus_int32         min_bytes;
 

     ogg_int64_t        enc_granulepos;
     ogg_int64_t        original_samples;
	 ogg_int64_t        last_granulepos;
     int                last_segments;
} ogg_enc_mem_t; 
int OGGCELT_Init(char *drate,char *c,char *out,ogg_enc_mem_t *ogg_enc, OpusCustomEncoder **enc);
int OGGCELT_Proc(short *pcm,  char *compressed, int nbsamples, ogg_enc_mem_t *ogg_enc, OpusCustomEncoder *enc);
void OGGCELT_Uninit(ogg_enc_mem_t *ogg_mem, OpusCustomEncoder *enc);
int  OGG_PARSER_INIT(ogg_dec_mem_t *ogg_mem);
int  OGG_PARSER_PROC(char *buf_in, ogg_dec_mem_t *ogg_mem);
void OGG_PARSER_UNINIT(ogg_dec_mem_t *ogg_mem);

#endif	/* __OGGCELT_API_H__ */