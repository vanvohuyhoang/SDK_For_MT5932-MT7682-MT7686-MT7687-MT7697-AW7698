#include <stdio.h>
#include <string.h>
#include "ogg.h"
#include "opus_header.h"
#include "oggcelt_api.h"

// Process an Opus header and setup the opus decoder based on it.
// It takes several pointers for header values which are needed elsewhere in the code.
int process_header(ogg_packet *op, int *rate,
				   int *mapping_family, int *channels, int *preskip, int *gain,
				   long manual_gain, int *streams, int wav_format, int quiet)
{
	OpusHeader header;
	
	if (opus_header_parse(op->packet, op->bytes, &header)==0) {   
		printf("Cannot parse header\n");
		return 0;
	}

	*mapping_family = header.channel_mapping;
	*channels       = header.channels;

	if(!*rate)*rate=header.input_sample_rate;
	// If the rate is unspecified we decode to 48000
	if(*rate==0)*rate=48000;
	if(*rate<8000||*rate>192000){
		printf("Warning: Crazy input_rate %d, decoding to 48000 instead.\n",*rate);
		*rate=48000;
	}

	*preskip = header.preskip/3; // 0802
	*streams = header.nb_streams; // channel

	if(header.gain!=0 || manual_gain!=0)
	{
		/*Gain API added in a newer libopus version, if we don't have it we apply the gain ourselves. We also add in a user provided manual gain at the same time.*/
		//int gainadj = (int)(manual_gain*256.)+header.gain;
		*gain       = 1;									//*gain = pow(10., gainadj/5120.); force disable 
	}

	return 1;	// st
};

void OGG_PARSER_UNINIT(ogg_dec_mem_t *ogg_mem)
{
	ogg_sync_clear(&ogg_mem->oy);
	if (ogg_mem->stream_init)	
		ogg_stream_clear(&ogg_mem->os);
}

int OGG_PARSER_INIT(ogg_dec_mem_t *ogg_mem)
{
	// ogg parser init 
	memset(ogg_mem,0,sizeof(*ogg_mem));
	ogg_sync_init(&ogg_mem->oy);		
	ogg_mem->gain			= 1;
	ogg_mem->byte_consumed	= OPUS_BufInSize;	
	
	return OGG_OK;
};

int OGG_PARSER_PROC(char *buf_in, ogg_dec_mem_t *ogg_mem)	
{
	int		mapping_family, gain, streams;
	int		st, status = OGG_OK;
	
	if ( (st = ogg_sync_pageout(&ogg_mem->oy, &ogg_mem->og)) != 1 ) {
		if (st < 0) {	// st < 0 from ogg_sync_pageseek(sync_fail level) fcuntion
			++ogg_mem->os.pageno;  
			ogg_mem->byte_consumed = 0;
			status = OGG_page_crc_chksum_err;
			goto L_ogg_parser_end;
		}
		if ( (st = ogg_sync_buffer(&ogg_mem->oy, buf_in, ogg_mem->byte_read)) != OGG_OK)	// Get the ogg buffer for writing
			return st;	
		ogg_sync_wrote(&ogg_mem->oy, ogg_mem->byte_read);
		bit_clr(ogg_mem->oy.ogg_status,PSR_DATA_RDY);
		ogg_mem->byte_consumed = OPUS_BufInSize;
	} else {
		if ( bit_chk(ogg_mem->oy.ogg_status,PSR_OPUS_PKT) == 0)
			bit_set(ogg_mem->oy.ogg_status,PSR_DATA_RDY);
		ogg_mem->byte_consumed = 0;
	}

	if(bit_chk(ogg_mem->oy.ogg_status,PSR_OPUS_PKT) )		goto	L_ogg_parser_opus_packet; 
	else if(bit_chk(ogg_mem->oy.ogg_status,PSR_DATA_RDY) )	goto	L_ogg_parser_data_ready; 
	else													goto	L_ogg_parser_end;

	// ogg_mem->oy.data[18]        = page_sequence_number
	// ogg_mem->os.lacing_returned = opus packet number
L_ogg_parser_data_ready: 
	if (ogg_mem->stream_init == 0) {
		if( (st = ogg_stream_init(&ogg_mem->os, ogg_page_serialno(&ogg_mem->og))) != OGG_OK) 
			return st;
		ogg_mem->stream_init = 1;
	}

	if (ogg_page_serialno(&ogg_mem->og) != ogg_mem->os.serialno) {
		ogg_stream_reset_serialno(&ogg_mem->os, ogg_page_serialno(&ogg_mem->og));
	}

	if ( (st = ogg_stream_pagein(&ogg_mem->os, &ogg_mem->og)) != OGG_OK)	// Add page to the bitstream	
		return st;	

	ogg_mem->dec_granulepos = ogg_page_granulepos(&ogg_mem->og)/3;  // 0802
	if( (ogg_mem->init_done == 0) && (ogg_mem->packet_count > 1) ) {
		ogg_mem->init_done = 1;
		bit_set(ogg_mem->oy.ogg_status,PSR_OPUS_PKT);
		goto	L_ogg_parser_end;
	}

L_ogg_parser_opus_packet:
	bit_set(ogg_mem->oy.ogg_status,PSR_OPUS_PKT);
	if ( (st = ogg_stream_packetout(&ogg_mem->os, &ogg_mem->op)) != 1) {
		if (st == -1) {	// st = -1 from ogg_stream_packetout->_packetout fcuntion
			status = OGG_page_sequence_err;
		}
		bit_clr(ogg_mem->oy.ogg_status,PSR_OPUS_PKT);
		goto	L_ogg_parser_end;
	}

	//while (ogg_stream_packetout(&os, &op) == 1) {
	if (ogg_mem->op.b_o_s && ogg_mem->op.bytes>=8 && !memcmp(ogg_mem->op.packet, "OpusHead", 8)) {
		if(ogg_mem->has_opus_stream && ogg_mem->has_tags_packet) {
			ogg_mem->has_opus_stream = 0;
			printf("\nWarning: stream %x ended without EOS and a new stream began.\n",(unsigned int)ogg_mem->os.serialno);
		}
		if(!ogg_mem->has_opus_stream) {
			if(ogg_mem->packet_count>0 && ogg_mem->opus_serialno==ogg_mem->os.serialno) {
				printf("\nError: Apparent chaining without changing serial number (%x == %x).\n", (unsigned int)ogg_mem->opus_serialno,(unsigned int)ogg_mem->os.serialno);
				return -1;
			}
			ogg_mem->opus_serialno   = ogg_mem->os.serialno;
			ogg_mem->has_opus_stream = 1;
			ogg_mem->has_tags_packet = 0;
			ogg_mem->link_out        = 0;
			ogg_mem->packet_count    = 0;
			ogg_mem->exec_done       = 0;	// eos = 0
			ogg_mem->total_links++;
		} else {
			printf("\nWarning: ignoring opus stream %x\n",(unsigned int)ogg_mem->os.serialno);
		}
	}

	if (!ogg_mem->has_opus_stream || ogg_mem->os.serialno != ogg_mem->opus_serialno)
		return  -1; //break;

	//If first packet in a logical stream, process the Opus header
	if (ogg_mem->packet_count==0) {
		//int st = process_header(&ogg_mem->op, &rate, &mapping_family, &channels, &preskip, &gain, manual_gain, &streams, wav_format, 0);
		st = process_header(&ogg_mem->op, &ogg_mem->samplerate, &mapping_family, &ogg_mem->channels, &ogg_mem->preskip, &gain, 0, &streams, 0, 0);
		if (!st)
			return -1;

		if(ogg_stream_packetout(&ogg_mem->os, &ogg_mem->op)!=0 || ogg_mem->og.header[ogg_mem->og.header_len-1]==255) {
			printf("Extra packets on initial header page. Invalid stream.\n");
			return -1;
		}
		//Remember how many samples at the front we were told to skip
		//so that we can adjust the timestamp counting.

	} else if (ogg_mem->packet_count==1) {
		//if (!quiet)
		//	print_comments((char*)op.packet, op.bytes);
		ogg_mem->has_tags_packet = 1;
		if(ogg_stream_packetout(&ogg_mem->os, &ogg_mem->op)!=0 || ogg_mem->og.header[ogg_mem->og.header_len-1]==255) {
			printf("Extra packets on initial tags page. Invalid stream.\n");
			return -1;
		}
	} else {
		status = OGG_packet_ready;
		#if 0
			printf(" %03d : ", ogg_mem->op.bytes);						
			printf("frame_cntr = %d\n", ++frame_cntr);
			for(i = 0 ; i < op.bytes ; i++ ) {
				printf(" %02x", op.packet[i] & 0xff);
			} // for(i)
			printf("\n");
		#endif
	}

	if( ogg_mem->os.lacing_packet <= ogg_mem->os.lacing_returned)	
		bit_clr(ogg_mem->oy.ogg_status, PSR_OPUS_PKT);

	if (ogg_mem->op.e_o_s && ogg_mem->os.serialno == ogg_mem->opus_serialno)	ogg_mem->exec_done       = 1 ; // eos = 1, don't care for anything except opus eos
	if (ogg_mem->exec_done)														ogg_mem->has_opus_stream = 0;
	
	ogg_mem->packet_count++;

L_ogg_parser_end:

	return	status;
};

