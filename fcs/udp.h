#include <arpa/inet.h>

#ifndef __FCS_UDP_H
#define __FCS_UDP_H

struct udp_endpoint {
	struct sockaddr_in ep;
};
typedef struct udp_endpoint udp_endpoint;

uint32_t ipv4_address_from_string(const char *);

void udp_endpoint_init(udp_endpoint *out, uint32_t address, int port);
uint32_t udp_endpoint_address(udp_endpoint *ep);
int udp_endpoint_port(udp_endpoint *ep);


struct udp_packet {
    udp_endpoint *ep;
    char *data;
    int count;
};
typedef struct udp_packet udp_packet;

udp_packet *udp_packet_copy(udp_packet *);
void *udp_packet_dealloc(udp_packet *);

typedef void(*udp_data_handler)(udp_packet *, void *);

struct udp_socket {
    int socket;
    udp_endpoint *broadcast;
    udp_data_handler handler;
    void *context;
};
typedef struct udp_socket udp_socket;

udp_socket *udp_socket_alloc(udp_endpoint *listen, udp_endpoint *broadcast, udp_data_handler handler, void *context);
void udp_socket_dealloc(udp_socket *s);

void udp_socket_read(udp_socket *s);
void udp_socket_write(udp_socket *s, char *data, int count);

#endif