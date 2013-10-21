#include "message.h"

struct message_header
{
    char code[4];
    uint8_t reserved;
};
typedef struct message_header message_header;

void message_channel_data_handler(udp_socket *socket, char *data, int count, udp_endpoint *peer, void *context) {
    message_channel channel = (message_channel *)context;

    message_header *header;
    header = (message_header *)data;

    if (strncmp(header->code, "DATA", 4) == 0) {
        int data_count = (length - 5) / 36;

        message_data *data = malloc(sizeof(message_data) * data_count);

        for (int i = 0; i < data_count; i++) {
            char *b = data + 5 + (36 * i);

            message_data *m = data + (i * sizeof(message_data));

            m->index = (uint32_t)(*b);
            memcpy(m->data, b + 4, 32);
        }

        channel->handler(channel, data, data_count, channel->context);
        free(data);
    }
}

message_channel *message_channel_alloc(udp_socket *s, void *context) {
    message_channel *result = (message_channel *)malloc(sizeof(message_channel));
    result->socket = s;
    result->handler = handler;
    result->context = context;
    return result;
}

void message_channel_dealloc(message_channel *c) {
    free(c);
}

void message_channel_receive(message_channel *c) {
    udp_socket_read(c->socket);
}

void message_channel_send(message_channel *c, message_data *messages, int count) {
    int bufferLength = sizeof(message_data) * count + 5;
    char *buffer = malloc(bufferLength);
    *(buffer + bufferLength) = 0;
    char *b = buffer;

    memcpy(buffer, "DATA\0", 5);
    b += 5;

    for (int i = 0; i < count; i++) {
        message_data message = messages[i];
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

    // write to socket

    udp_socket_write(c->socket, buffer, b);

    free(buffer);
}