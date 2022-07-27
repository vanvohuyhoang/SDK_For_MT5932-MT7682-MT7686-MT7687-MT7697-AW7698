#ifndef __OPUSENC_H
#define __OPUSENC_H
#include <opus_types.h>

typedef long (*audio_read_func)(void *src, float *buffer, int samples);

typedef struct
{
    audio_read_func read_samples;
    void *readdata;
    opus_int64 total_samples_per_channel;
    int rawmode;
    int channels;
    long rate;
    int gain;
    int samplesize;
    int endianness;
    char *infilename;
    int ignorelength;
    int skip;
    int extraout;
    char *comments;
    int comments_length;
    int copy_comments;
    int copy_pictures;
} oe_enc_opt;

void comment_add(char **comments, int* length, char *tag, char *val);



#endif /* __OPUSENC_H */
