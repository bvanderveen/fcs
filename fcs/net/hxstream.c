#include "hxstream.h"
#include <stdio.h>

#define FRAME_BUFFER_OCTET (0x7e)
#define CONTROL_ESPACE_OCTET (0x7c)

#define ESCAPE(O) ((O) ^ 0x20)

void *hx_stream_encode_frame(void *input, size_t input_length, uint8_t tag, size_t *output_length) {
    // worst case message size is (instances of fbo-or-ceo) * 2 + 2
    // so we'll just allocate all of it.
    // what could go wrong!?
    uint8_t *output = calloc(input_length * 2 + 2, sizeof(uint8_t));

    int i;
    int o = 0;

    output[o++] = FRAME_BUFFER_OCTET;
    output[o++] = tag;

    for (i = 0; i < input_length; i++) {
        uint8_t x = ((uint8_t *)input)[i];
        if (x == FRAME_BUFFER_OCTET || x == CONTROL_ESPACE_OCTET) {
            output[o++] = CONTROL_ESPACE_OCTET;
            output[o++] = ESCAPE(x);
        }
        else {
            output[o++] = x;
        }
    }

    *output_length = o;

    return output;
}

typedef enum {
    hx_stream_decoder_state_frame,
    hx_stream_decoder_state_tag,
    hx_stream_decoder_state_octet,
    hx_stream_decoder_state_escape,
}
hx_stream_decoder_state;

typedef struct {
    hx_stream_decoder_state state;
    hx_stream_decoder_delegate *delegate;
} hx_stream_decoder_storage;

hx_stream_decoder hx_stream_decoder_init(hx_stream_decoder_delegate *delegate) {
    hx_stream_decoder_storage *result = calloc(1, sizeof(hx_stream_decoder_storage));
    result->state = hx_stream_decoder_state_frame;
    result->delegate = delegate;
    return (hx_stream_decoder)result;
}

void hx_stream_decoder_dealloc(hx_stream_decoder *decoder) {
    free(decoder);
}


void hx_stream_decoder_execute(hx_stream_decoder *decoder, uint8_t *input, void *context) {
    hx_stream_decoder_storage *this = (hx_stream_decoder_storage *)decoder;
    hx_stream_decoder_state state = this->state;
    hx_stream_decoder_delegate *delegate = this->delegate; 

    uint8_t x = *(uint8_t *)input;

    if (state == hx_stream_decoder_state_frame) {
        if (x == FRAME_BUFFER_OCTET) {
            this->state = hx_stream_decoder_state_tag;
            delegate->on_start(context);
        }
    }
    else if (state == hx_stream_decoder_state_tag) {  
        if (x == FRAME_BUFFER_OCTET) {
            this->state = hx_stream_decoder_state_tag;
            delegate->on_start(context);
        }
        else {
            this->state = hx_stream_decoder_state_octet;
            delegate->on_tag(context, x);
        }
    }
    else if (state == hx_stream_decoder_state_octet) {
        if (x == FRAME_BUFFER_OCTET) {
            this->state = hx_stream_decoder_state_tag;
            delegate->on_start(context);
        }
        else if (x == CONTROL_ESPACE_OCTET) {
            this->state = hx_stream_decoder_state_escape;
        }
        else {
            delegate->on_data(context, x);
        }
    }
    else if (state == hx_stream_decoder_state_escape) {
        uint8_t escaped = ESCAPE(x);
        this->state = hx_stream_decoder_state_octet;
        delegate->on_data(context, escaped);
    }
    else {
        printf("[hx_stream_decoder] decoder had invalid state %d", state);
    }
}










