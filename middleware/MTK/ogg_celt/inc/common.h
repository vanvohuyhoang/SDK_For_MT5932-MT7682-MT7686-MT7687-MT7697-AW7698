#ifndef __COMMON_H__
#define __COMMON_H__

#define OGG_DEC_ALLOC_EN
#define OPUS_Channel		1
#define OPUS_SampleRate		16000
#define OPUS_BufInSize		256
#define OPUS_FrameeSize		320

// ==========================================================================================================
#define Ogg_Body_Size	16384
#define Ogg_Lacing_Size	256

#define MODE_SILK_ONLY  1000
#define MODE_HYBRID     1001
#define MODE_CELT_ONLY  1002

#define PSR_DATA_RDY	0	
#define PSR_OPUS_PKT	1

typedef enum {
	OGG_packet_ready			=  70,
	OGG_packet_celt_mode		=  71,
	OGG_OK                      =   0,
	OGG_mem_alloc_err			= -11,
	OGG_stream_check_err		= -13, 
	OGG_serialno_err			= -14,
	OGG_version_err				= -15,
	OGG_os_lacing_expand_err	= -16,
	OGG_os_body_expand_err   	= -17,
	OGG_mem_overflow            = -18,
	OGG_page_crc_chksum_err		= -19, 
	OGG_page_sequence_err		= -20,
} ogg_status_t;


#define bit_set(a,b)  ((a) |=  (1<<b))
#define bit_clr(a,b)  ((a) &= ~(1<<b))
#define bit_chk(a,b)  ((a)  &  (1<<b))

#endif	/* __COMMON_H__ */ 