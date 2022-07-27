#ifndef __FILE_FMT_H__
#define __FILE_FMT_H__

#define AUDIO_FILE_ENCODING_MULAW_8         1 /* 8-bit ISDN u-law */
#define AUDIO_FILE_ENCODING_LINEAR_8        2 /* 8-bit linear PCM */
#define AUDIO_FILE_ENCODING_LINEAR_16       3 /* 16-bit linear PCM */
#define AUDIO_FILE_ENCODING_LINEAR_32       5 /* 32-bit linear PCM */ 
#define AUDIO_FILE_ENCODING_FLOAT           6 /* 32-bit IEEE floating point */ 
#define AUDIO_FILE_ENCODING_DOUBLE          7 /* 64-bit IEEE floating point */ 
#define AUDIO_FILE_ENCODING_ADPCM_G721     23 /* 4-bit CCITT g.721 ADPCM */ 
#define AUDIO_FILE_ENCODING_ADPCM_G723_3   25 /* CCITT g.723 3-bit ADPCM */ 
#define AUDIO_FILE_ENCODING_ALAW_8         27 /* 8-bit ISDN A-law */

typedef struct {
unsigned int    magic;          /* magic number */
unsigned int    hdr_size;       /* byte offset to start of audio data */ 
unsigned int    data_size;      /* data length, in bytes (optional) */ 
unsigned int    encoding;       /* data encoding enumeration */
unsigned int    sample_rate;    /* samples per second */
unsigned int    channels;       /* number of interleaved channels */
} au_filehdr_t;  

typedef struct WAVE_Header {
    unsigned char riff[4];				// "RIFF" string
    unsigned int overall_size;			// overall size of file in bytes
    unsigned char wave[4];              // "WAVE" string
    unsigned char fmt_chunk_marker[4];  // "fmt" string
    unsigned int length_of_fmt;         // 16 for PCM. This is the size of the rest of the Subchunk which follows this number.
    unsigned short format_type;         // format type. 1-PCM, 3- IEEE float, 6 - 8bit A law, 7 - 8bit mu law
    unsigned short NumChannels;         // 1(Mono), 2(Stero)
    unsigned int SampleRate;			// 8000, 44100, etc.	
    unsigned int ByteRate;				// SampleRate * NumChannels * BitsPerSample/8
    unsigned short BlockAlign;			// NumChannels * BitsPerSample/8
    unsigned short BitsPerSample;		// bits per sample, 16(16-bit) , 32(32-bit) etc
    unsigned char data_chunk_header[4]; // "data" string
    unsigned int data_size;             // NumSamples * NumChannels * BitsPerSample/8 - size of the next chunk that will be read
} WAVE_Header;

//struct  AU_Header   au_header = {0x646e732e, 0x18000000, 0x0, 0x0, 0x0, 0x0 };
//struct  WAVE_Header	wave_header;

void WavFile_End(WAVE_Header *wave_header, unsigned int fo_len, FILE *fo);

#endif /* __FILE_FMT_H__ */