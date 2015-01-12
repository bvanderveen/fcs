#include "xplane_socket.h"
#include "../debug.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

void xplane_udp_data_handler_function(udp_packet *p, void *context) {
    xplane_socket *xplane_sock = context;
    xplane_data_handler handler = xplane_sock->handler;
    void *ctx = xplane_sock->context;

    xplane_message_header *header;
    header = (xplane_message_header *)p->data;

    if (strncmp(header->code, "DATA", 4) == 0) {
        int data_count = (p->count - XPLANE_MESSAGE_HEADER_SIZE) / XPLANE_MESSAGE_DATA_SIZE;
        handler((xplane_message_data *)&p->data[5], data_count, ctx);
    }
    else {
        LLog("[xplane_socket] discarding bogus message\n");
    }
}

xplane_socket *xplane_socket_alloc(udp_socket *s) {
    xplane_socket *result = calloc(1, sizeof(xplane_socket));
    result->socket = s;
    return result;
}

void xplane_socket_dealloc(xplane_socket *s) {
    free(s);
}

void xplane_socket_read(xplane_socket *s, xplane_data_handler handler, void *context) {
    s->handler = handler;
    s->context = context;
    udp_socket_read(s->socket, xplane_udp_data_handler_function, s);
}

void xplane_socket_write(xplane_socket *s, xplane_message_data *messages, int count) {
    udp_socket *socket = s->socket;

    int bufferLength = XPLANE_MESSAGE_DATA_SIZE * count + XPLANE_MESSAGE_HEADER_SIZE;
    unsigned char *buffer = malloc(bufferLength);
    *(buffer + bufferLength) = 0;
    unsigned char *b = buffer;

    memcpy(buffer, "DATA\0", 5);
    b += 5;

    for (int i = 0; i < count; i++) {
        xplane_message_data message = messages[i];
        memcpy(b, &message.index, 4);
        b += 4;

        for (int j = 0; j < 8; j++) {
            float value = message.data[j];
            
            // xplane seems to crash if it gets NaN
            if (value != value) {
                LLog("[xplane_socket_write] got NaN, ignoring\n");
                value = 0;

            }

            memcpy(b, &value, sizeof(float));
            b += sizeof(float);
        }
    }

    udp_socket_write(socket, buffer, bufferLength);
}
