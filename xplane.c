#include "xplane.h"
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <errno.h>

void init_udp_endpoint(struct sockaddr_in *outEndpoint, long address, int port) {
    memset(outEndpoint, 0, sizeof(struct sockaddr_in));
    outEndpoint->sin_family = AF_INET;
    outEndpoint->sin_port = htons(port);
    outEndpoint->sin_addr.s_addr = htonl(address);
}

int create_udp_socket(struct sockaddr_in *endpoint) {
    int result = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);

    if (result == -1) {
        return -1;
    }

    if (bind(result, (const struct sockaddr *)endpoint, sizeof(struct sockaddr_in)) == -1) {
        return -1;
    }

    return result;
}

void init_xplane_context(xplane_context *context, int listenPort, int destinationPort) {
    init_udp_endpoint(&context->listenEndpoint, INADDR_ANY, listenPort);
    init_udp_endpoint(&context->destinationEndpoint, INADDR_ANY, destinationPort);
    context->socket = create_udp_socket(&context->listenEndpoint);
}

void on_packet(xplane_context *context, char *buffer, int length, struct sockaddr_in *peerEndpoint) {
    //printf("on_packet %s:%d - %d bytes\n", inet_ntoa(peerEndpoint->sin_addr), ntohs(peerEndpoint->sin_port), length);
    xplane_message_header *header;
    header = (xplane_message_header *)buffer;

    if (strncmp(header->code, "DATA", 4) == 0) {
        int data_count = (length - 5) / 36;

        xplane_message_data *data = malloc(sizeof(xplane_message_data) * data_count);

        for (int i = 0; i < data_count; i++) {
            char *b = buffer + 5 + (36 * i);

            xplane_message_data *data_message = data + (i * sizeof(xplane_message_data));

            data_message->index = (uint32_t)(*b);
            memcpy(data_message->data, b + 4, 32);
        }

        context->data_handler(context, data, data_count);
    }
}

void xplane_context_read(xplane_context *context) {
    int bufferLength = 4 * 1024;
    char buffer[bufferLength];
    struct sockaddr_in peerEndpoint;
    size_t peerEndpointLength = sizeof(peerEndpoint);

    int bytesRead = recvfrom(context->socket, buffer, bufferLength, 0, (struct sockaddr *)&peerEndpoint, (socklen_t *)&peerEndpointLength);

    if (bytesRead == -1) {
        printf("recvfrom() failed: %d", errno);
        return;
    }

    on_packet(context, buffer, bytesRead, &peerEndpoint);
}

void xplane_write_data(xplane_context *context, xplane_message_data *messages, int count) {

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
            memcpy(b, &message.data[j], sizeof(float));
            b += sizeof(float);
        }
    }

    for (int i = 0; i < bufferLength; i++)
    {
      printf("%d ", (int)(buffer[i]));
    }
    printf("\n");

    int bytesSent = sendto(context->socket, buffer, bufferLength, 0, (struct sockaddr *)&context->destinationEndpoint, sizeof(struct sockaddr_in));

    if (bytesSent < 0)
        printf("sendto() failed: %d", errno);

    free(buffer);
}
