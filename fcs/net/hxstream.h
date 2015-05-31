#include <stdint.h>
#include <stdlib.h>

#ifndef __FCS_HXSTREAM_H
#define __FCS_HXSTREAM_H

typedef void *hx_stream_decoder;

typedef void(*hx_stream_decoder_frame_start_callback)(void *context);
typedef void(*hx_stream_decoder_frame_tag_callback)(void *context, uint8_t tag);
typedef void(*hx_stream_decoder_frame_data_callback)(void *context, uint8_t data);

typedef struct {
	hx_stream_decoder_frame_start_callback on_start;
	hx_stream_decoder_frame_tag_callback on_tag;
	hx_stream_decoder_frame_data_callback on_data;
} hx_stream_decoder_delegate;

hx_stream_decoder hx_stream_decoder_init(hx_stream_decoder_delegate *delegate);
void hx_stream_decoder_execute(hx_stream_decoder *decoder, uint8_t *data, void *context);
void hx_stream_decoder_dealloc(hx_stream_decoder *decoder);

void *hx_stream_encode_frame(void *input, size_t input_length, uint8_t tag, size_t *output_length);

#endif