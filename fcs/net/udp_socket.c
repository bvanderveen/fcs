#include "udp_socket.h"
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <errno.h>


udp_packet *udp_packet_copy(udp_packet *p) {
    udp_packet *result = malloc(sizeof(udp_packet));
    udp_endpoint *result_ep = malloc(sizeof(udp_endpoint));

    udp_endpoint_init(result_ep, udp_endpoint_address(p->ep), udp_endpoint_port(p->ep));

    result->ep = result_ep;
    result->data = malloc(p->count);
    result->count = p->count;

    memcpy(result->data, p->data, p->count);

    return result;
}

void udp_packet_dealloc(udp_packet *p) {
    free(p->ep);
    free(p->data);
    free(p);
}

uint32_t ipv4_address_from_string(const char *ipAddress) {
  int ipbytes[4];
  sscanf(ipAddress, "%d.%d.%d.%d", &ipbytes[3], &ipbytes[2], &ipbytes[1], &ipbytes[0]);
  return ipbytes[0] | ipbytes[1] << 8 | ipbytes[2] << 16 | ipbytes[3] << 24;
}

void udp_endpoint_init(udp_endpoint *out, uint32_t address, int port) {
    memset(out, 0, sizeof(struct sockaddr_in));
    out->ep.sin_family = AF_INET;
    out->ep.sin_port = htons(port);
    out->ep.sin_addr.s_addr = htonl(address);
}

uint32_t udp_endpoint_address(udp_endpoint *ep) {
    return ntohl(ep->ep.sin_addr.s_addr);
}

int udp_endpoint_port(udp_endpoint *ep) {
    return ntohs(ep->ep.sin_port);
}

int udp_endpoint_socket(udp_endpoint *endpoint) {
    int result = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);

    if (result == -1) {
        printf("[udp_socket] failed to create socket: errno %d\n", errno);
        return -1;
    }

    if (bind(result, (const struct sockaddr *)endpoint, sizeof(struct sockaddr_in)) == -1) {

        printf("[udp_socket] failed to bind socket: errno %d\n", errno);
        return -1;
    }

    return result;
}

udp_socket *udp_socket_alloc(udp_endpoint *listen, udp_endpoint *broadcast) {
    udp_socket *result = malloc(sizeof(udp_socket));
    int socket = udp_endpoint_socket(listen);
    
    if(socket == -1) {
        free(result);
        printf("[udp_socket] failed to initialize socket\n");
        return NULL;
    }

    result->socket = socket;
    result->broadcast = broadcast;

    return result;
}

void udp_socket_dealloc(udp_socket *s) {
    close(s->socket);
    free(s);
}

void udp_socket_read(udp_socket *s, udp_data_handler handler, void *context) {
    int buffer_length = 4 * 1024;
    char buffer[buffer_length];
    struct sockaddr_in peer_endpoint;
    size_t peer_endpoint_length = sizeof(peer_endpoint);

    printf("[udp_socket_read] will recvfrom\n");

    int bytes_read = recvfrom(s->socket, buffer, buffer_length, 0, (struct sockaddr *)&peer_endpoint, (socklen_t *)&peer_endpoint_length);

    printf("[udp_socket_read] did recvfrom\n");

    if (bytes_read == -1) {
        printf("[udp_socket] recvfrom failed: errno %d\n", errno);
        return;
    }

    udp_endpoint ep;
    ep.ep = peer_endpoint;

    udp_packet packet;
    packet.ep = &ep;
    packet.data = (char *)&buffer;
    packet.count = bytes_read;

    printf("[udp_socket_read] 1\n");
    handler(&packet, context);
    printf("[udp_socket_read] 2\n");
    
}

void udp_socket_write(udp_socket *s, char *data, int count) {
    int bytes_sent = sendto(s->socket, data, count, 0, (struct sockaddr *)&s->broadcast->ep, sizeof(struct sockaddr_in));

    if (bytes_sent < 0)
        printf("[udp_socket] sendto failed: errno %d\n", errno);
}

