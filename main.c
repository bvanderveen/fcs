#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <errno.h>

struct xplane_message_header
{
    char code[4];
    uint8_t reserved;
};
typedef struct xplane_message_header xplane_message_header;

struct xplane_message_data
{
    uint32_t index;
    float data[8];
};
typedef struct xplane_message_data xplane_message_data;

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

void on_data_message(xplane_message_data *messages, int count) {
    for (int i = 0; i < count; i++) {
        xplane_message_data *message = messages + i * sizeof(xplane_message_data);

        // printf("on_data_message (index = %d) ", message->index);
        // for (int j = 0; j < 8; j++) {
        //     printf("%f ", message->data[j]);
        // }
        // printf("\n");
    }
}

void on_packet(char *buffer, int length, struct sockaddr_in *peerEndpoint) {
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

        on_data_message(data, data_count);
    }
}

void write_xplane_data(int socket, struct sockaddr_in *endpoint, xplane_message_data *messages, int count) {

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

    int bytesSent = sendto(socket, buffer, bufferLength, 0, (struct sockaddr *)endpoint, sizeof(struct sockaddr_in));

    if (bytesSent < 0)
        printf("sendto() failed: %d", errno);

    free(buffer);
}

enum xplane_data_index {
    xplane_data_throttle = 25
};
typedef int xplane_data_index;

int main() {
    int listenPort = 49003;
    struct sockaddr_in listenEndpoint;

    int destinationPort = 49000;
    struct sockaddr_in destinationEndpoint;

    init_udp_endpoint(&listenEndpoint, INADDR_ANY, listenPort);
    int listenSocket = create_udp_socket(&listenEndpoint);

    int bufferLength = 4 * 1024;
    char buffer[bufferLength];

    float throttle = 0;

    while (1) {
        struct sockaddr_in peerEndpoint;
        size_t peerEndpointLength = sizeof(peerEndpoint);
        int bytesRead = recvfrom(listenSocket, buffer, bufferLength, 0, (struct sockaddr *)&peerEndpoint, (socklen_t *)&peerEndpointLength);

        if (bytesRead == -1) {
            printf("recvfrom failed");
            break;
        }

        on_packet(buffer, bytesRead, &peerEndpoint);

        xplane_message_data data;
        data.index = xplane_data_throttle;
        data.data[0] = throttle;
        data.data[1] = throttle;
        data.data[2] = 0;
        data.data[3] = 0;
        data.data[4] = 0;
        data.data[5] = 0;
        data.data[6] = 0;
        data.data[7] = throttle;

        write_xplane_data(listenSocket, &destinationEndpoint, &data, 1);

        throttle += .01;

        if (throttle > 1.0)
            throttle = 1.0;
    }

    return 0;
}