#include <arpa/inet.h>

#ifndef __FCS_UDP_H
#define __FCS_UDP_H

struct udp_endpoint {
	struct sockaddr_in ep;
};
typedef struct udp_endpoint;

void udp_endpoint_init(udp_endpoint *out, long address, int port);
int udp_endpoint_address(udp_endpoint *ep);
int udp_endpoint_port(udp_endpoint *ep);

typedef void (*)(struct udp_socket *, char *, int, udp_endpoint, void *) udp_data_hander;

struct udp_socket {
    int socket;
    udp_endpoint *broadcast;
    udp_data_handler *handler;
    void *context;
};
typedef struct udp_socket udp_socket;

udp_socket *udp_socket_alloc(udp_endpoint *listen, udp_endpoint *broadcast, udp_data_handler *handler, void *context);
void udp_socket_dealloc(udp_socket *s);

void udp_socket_read(udp_socket *s);
void udp_socket_write(udp_socket *s, char *data, int count);

#endif