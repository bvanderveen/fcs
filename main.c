#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

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

void init_udp_endpoint(struct sockaddr_in *outEndpoint, int listenPort) {
    memset(outEndpoint, 0, sizeof(struct sockaddr_in));
    outEndpoint->sin_family = AF_INET;
    outEndpoint->sin_port = htons(listenPort);
    outEndpoint->sin_addr.s_addr = htonl(INADDR_ANY);
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

        printf("on_data_message (index = %d)\n", message->index);
        for (int j = 0; j < 8; j++) {
            printf("dataf[%d] = %f\n", j, message->data[j]);
        }
    }
}

void on_packet(char *buffer, int length, struct sockaddr_in *peerEndpoint) {
    printf("on_packet %s:%d - %d bytes\n", inet_ntoa(peerEndpoint->sin_addr), ntohs(peerEndpoint->sin_port), length);
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

int main() {
    int listenPort = 49003;
    struct sockaddr_in listenEndpoint;

    init_udp_endpoint(&listenEndpoint, listenPort);
    int listenSocket = create_udp_socket(&listenEndpoint);

    int bufferLength = 4 * 1024;
    char buffer[bufferLength];

    while (1) {
        struct sockaddr_in peerEndpoint;
        size_t peerEndpointLength = sizeof(peerEndpoint);
        int bytesRead = recvfrom(listenSocket, buffer, bufferLength, 0, (struct sockaddr *)&peerEndpoint, (socklen_t *)&peerEndpointLength);

        if (bytesRead == -1) {
            printf("recvfrom failed");
            break;
        }

        on_packet(buffer, bytesRead, &peerEndpoint);
    }

    return 0;
}