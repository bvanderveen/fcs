#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <errno.h>

void udp_endpoint_init(udp_endpoint *out, long address, int port) {
    memset(out, 0, sizeof(struct sockaddr_in));
    out->ep.sin_family = AF_INET;
    out->ep.sin_port = htons(port);
    out->ep.sin_addr.s_addr = htonl(address);
}

int udp_endpoint_address(udp_endpoint *ep) {
    return 0;
}

int udp_endpoint_port(udp_endpoint *ep) {
    return ntohs(peerEndpoint->sin_port);
}

int udp_endpoint_socket(udp_endpoint *endpoint) {
    int result = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);

    if (result == -1) {
        return -1;
    }

    if (bind(result, (const struct sockaddr *)endpoint, sizeof(struct sockaddr_in)) == -1) {
        return -1;
    }

    return result;
}

udp_socket *udp_socket_alloc(udp_endpoint *listen, udp_endpoint *broadcast, udp_data_handler *handler, void *context) {
    udp_socket *result = (udp_socket *)malloc(sizeof(udp_socket));
    result->socket = udp_endpoint_socket(listen);
    result->broadcast = broadcast;
    result->handler = handler;
    result->context = context;
    return result;
}

void udp_socket_dealloc(udp_socket *s) {
    free(s);
}


void udp_socket_read(udp_socket *s) {
    int bufferLength = 4 * 1024;
    char buffer[bufferLength];
    struct sockaddr_in peerEndpoint;
    size_t peerEndpointLength = sizeof(peerEndpoint);

    int bytesRead = recvfrom(context->socket, buffer, bufferLength, 0, (struct sockaddr *)&peerEndpoint, (socklen_t *)&peerEndpointLength);

    if (bytesRead == -1) {
        printf("recvfrom() failed: %d", errno);
        return;
    }


    s->handler(s, buffer, bytesRead, &peerEndpoint);
}

void udp_socket_write(udp_socket *s, char *data, int count) {
    int bytesSent = sendto(s->socket, data, count, 0, (struct sockaddr *)&s->broadcast, sizeof(struct sockaddr_in));

    if (bytesSent < 0)
        printf("sendto() failed: %d", errno);
}



void init_udp_endpoint(struct sockaddr_in *outEndpoint, long address, int port) {
    memset(outEndpoint, 0, sizeof(struct sockaddr_in));
    outEndpoint->sin_family = AF_INET;
    outEndpoint->sin_port = htons(port);
    outEndpoint->sin_addr.s_addr = htonl(address);
}

