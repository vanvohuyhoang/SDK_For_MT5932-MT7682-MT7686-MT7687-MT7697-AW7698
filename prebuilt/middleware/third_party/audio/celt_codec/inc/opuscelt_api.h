#ifndef __OPUSCELT_API_H__
#define __OPUSCELT_API_H__
#include "opus_types.h"
typedef short opus_val16;
typedef int opus_val32;


typedef int celt_sig;
typedef short celt_norm;
typedef int celt_ener;
#define MAX_PERIOD 1024
#define MAXFACTORS 8
//#define OPUS_RESTRICT restrict

typedef struct OpusCustomMode OpusCustomMode;
typedef struct OpusCustomDecoder OpusCustomDecoder;
typedef struct OpusCustomEncoder OpusCustomEncoder;


//#define MAXFACTORS 8
/* e.g. an fft of length 128 has 4 factors
 as far as kissfft is concerned
 4*4*4*2
 */





/** Mode definition (opaque)
 @brief Mode definition
 */
struct OpusCustomMode {
   opus_int32 Fs;
   int          overlap;

   int          nbEBands;
   int          effEBands;
   opus_val16    preemph[4];
   const opus_int16   *eBands;   /**< Definition for each "pseudo-critical band" */

   int         maxLM;
   int         nbShortMdcts;
   int         shortMdctSize;

   int          nbAllocVectors; /**< Number of lines in the matrix below */
   const unsigned char   *allocVectors;   /**< Number of bits in each band for several rates */
   const opus_int16 *logN;

   const opus_val16 *window;
  // mdct_lookup mdct;
  // PulseCache cache;
};

/** Encoder state
 @brief Encoder state
 */
typedef struct {
   int valid;
   float tonality;
   float tonality_slope;
   float noisiness;
   float activity;
   float music_prob;
   int        bandwidth;
}AnalysisInfo;

struct OpusCustomEncoder {
   const OpusCustomMode *mode;     /**< Mode used by the encoder */
   int channels;
   int stream_channels;

   int force_intra;
   int clip;
   int disable_pf;
   int complexity;
   int upsample;
   int start, end;

   opus_int32 bitrate;
   int vbr;
   int signalling;
   int constrained_vbr;      /* If zero, VBR can do whatever it likes with the rate */
   int loss_rate;
   int lsb_depth;
   int variable_duration;
   int lfe;
   int arch;

   /* Everything beyond this point gets cleared on a reset */
#define ENCODER_RESET_START rng

   opus_uint32 rng;
   int spread_decision;
   opus_val32 delayedIntra;
   int tonal_average;
   int lastCodedBands;
   int hf_average;
   int tapset_decision;

   int prefilter_period;
   opus_val16 prefilter_gain;
   int prefilter_tapset;
#ifdef RESYNTH
   int prefilter_period_old;
   opus_val16 prefilter_gain_old;
   int prefilter_tapset_old;
#endif
   int consec_transient;
   AnalysisInfo analysis;

   opus_val32 preemph_memE[2];
   opus_val32 preemph_memD[2];

   /* VBR-related parameters */
   opus_int32 vbr_reservoir;
   opus_int32 vbr_drift;
   opus_int32 vbr_offset;
   opus_int32 vbr_count;
   opus_val32 overlap_max;
   opus_val16 stereo_saving;
   int intensity;
   opus_val16 *energy_mask;
   opus_val16 spec_avg;

#ifdef RESYNTH
   /* +MAX_PERIOD/2 to make space for overlap */
   celt_sig syn_mem[2][2*MAX_PERIOD+MAX_PERIOD/2];
#endif

   celt_sig in_mem[1]; /* Size = channels*mode->overlap */
   /* celt_sig prefilter_mem[],  Size = channels*COMBFILTER_MAXPERIOD */
   /* opus_val16 oldBandE[],     Size = channels*mode->nbEBands */
   /* opus_val16 oldLogE[],      Size = channels*mode->nbEBands */
   /* opus_val16 oldLogE2[],     Size = channels*mode->nbEBands */
};

/** Decoder state
 @brief Decoder state
 */
struct OpusCustomDecoder {
   const OpusCustomMode *mode;
   int overlap;
   int channels;
   int stream_channels;

   int downsample;
   int start, end;
   int signalling;
   int arch;

   /* Everything beyond this point gets cleared on a reset */
#define DECODER_RESET_START rng

   opus_uint32 rng;
   int error;
   int last_pitch_index;
   int loss_count;
   int skip_plc;
   int postfilter_period;
   int postfilter_period_old;
   opus_val16 postfilter_gain;
   opus_val16 postfilter_gain_old;
   int postfilter_tapset;
   int postfilter_tapset_old;

   celt_sig preemph_memD[2];

   celt_sig _decode_mem[1]; /* Size = channels*(DECODE_BUFFER_SIZE+mode->overlap) */
   /* opus_val16 lpc[],  Size = channels*LPC_ORDER */
   /* opus_val16 oldEBands[], Size = 2*mode->nbEBands */
   /* opus_val16 oldLogE[], Size = 2*mode->nbEBands */
   /* opus_val16 oldLogE2[], Size = 2*mode->nbEBands */
   /* opus_val16 backgroundLogE[], Size = 2*mode->nbEBands */
};

/**
 * @brief
 * do initialize of the encoder
 * complexity mean quality from 0(low) ~3(high) 
 * return 0 : SUCCESS  ;the other value: FAIL 
 * input  no
 * output no
 */
int OPUSCELT_16K_C1_F320_init(int complexity, OpusCustomEncoder **enc);
/**
 * @brief
 * encoder function per frame 
 * return :the compressed size + 8(data header per frame) (bytes)  
 * input  pcm
 * output compressed + 8(data header per frame)
 * nbCompressedBytes   : the size of compressed 
 */
 
int OPUSCELT_16K_C1_F320_proc(short *pcm, unsigned char *compressed, int nbCompressedBytes, OpusCustomEncoder *enc);
/**
 * @brief
 * the end of encoder function 
 * return :no  
 * input  no
 * output no
 */

void OPUSCELT_uninit(OpusCustomEncoder *enc);
/**
 * @brief
 * do initialize of the decoder
 * input avc_dec the address of decoder memory
 * return 0 : SUCCESS  ; the other value: FAIL 
 * input  no
 * output no
 */

int OPUSCELT_DEC_16K_C1_F320_init(OpusCustomDecoder **avc_dec);  
/**
 * @brief
 * decoder function per frame 
 * return :the pcm size(16bits)  
 * input avc_dec the address of decoder memory
 * input  compressed  
 * output pcm
 * nbCompressedBytes   : the size of compressed data which is the same with the encoder
 */
int OPUSCELT_DEC_16K_C1_F320_proc(unsigned char *compressed,short *pcm,int nbCompressedBytes,OpusCustomDecoder *avc_dec);

void OPUSCELT_DEC_uninit(OpusCustomDecoder *avc_dec);

#endif	/* __OPUSCELT_API_H__ */ 