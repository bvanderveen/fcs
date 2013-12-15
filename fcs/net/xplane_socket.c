#include "xplane_socket.h"
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
        int data_count = (p->count - 5) / sizeof(xplane_message_data);

        printf("[xplane_udp_data_handler_function] 1\n");
        xplane_message_data *data = malloc(sizeof(xplane_message_data) * data_count);
        printf("[xplane_udp_data_handler_function] 2\n");

        for (int i = 0; i < data_count; i++) {
            char *b = p->data + 5 + (sizeof(xplane_message_data) * i);

            xplane_message_data *data_message = &data[i];

            data_message->index = (uint32_t)(*b);
            memcpy(data_message->data, b + sizeof(uint32_t), sizeof(float) * 8);
        }

        printf("[xplane_udp_data_handler_function] handler = %x\n", (unsigned int)handler);
        handler(data, data_count, ctx);
        printf("[xplane_udp_data_handler_function] 4\n");

        free(data);
        printf("[xplane_udp_data_handler_function] 5\n");
    }
    else {
        printf("[xplane_socket] discarding bogus message\n");
    }
}

xplane_socket *xplane_socket_alloc(udp_socket *s) {
    xplane_socket *result = malloc(sizeof(xplane_socket));
    result->socket = s;
    return result;
}

void xplane_socket_dealloc(xplane_socket *s) {
    free(s);
}

void xplane_socket_read(xplane_socket *s, xplane_data_handler handler, void *context) {
    s->handler = handler;
    s->context = context;
    printf("[xplane_socket_read] will call udp_socket_read\n");
    udp_socket_read(s->socket, xplane_udp_data_handler_function, s);
    printf("[xplane_socket_read] did call udp_socket_read\n");
}

void xplane_socket_write(xplane_socket *s, xplane_message_data *messages, int count) {
    udp_socket *socket = s->socket;

    int bufferLength = sizeof(xplane_message_data) * count + 5;
    char *buffer = malloc(bufferLength);
    *(buffer + bufferLength) = 0;
    char *b = buffer;

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
                printf("got NaN, ignoring\n");
                value = 0;

            }

            memcpy(b, &value, sizeof(float));
            b += sizeof(float);
        }
    }

    udp_socket_write(socket, buffer, bufferLength);
}
