#include "udp_socket.h"

#ifndef __FCS_XPLANE_SOCKET_H
#define __FCS_XPLANE_SOCKET_H

struct xplane_message_header
{
    char code[4];
    uint8_t reserved;
};
typedef struct xplane_message_header xplane_message_header;

#define XPLANE_MESSAGE_HEADER_SIZE 5

struct xplane_message_data
{
    uint32_t index;
    float data[8];
};
typedef struct xplane_message_data xplane_message_data;

#define XPLANE_MESSAGE_DATA_SIZE (9 * sizeof(uint32_t))

typedef void(*xplane_data_handler)(xplane_message_data *, int, void *);

struct xplane_socket {
    udp_socket *socket;
    xplane_data_handler handler;
    void *context;
};
typedef struct xplane_socket xplane_socket;

xplane_socket *xplane_socket_alloc(udp_socket *s);
void xplane_socket_dealloc(xplane_socket *s);

void xplane_socket_read(xplane_socket *s, xplane_data_handler handler, void *context);
void xplane_socket_write(xplane_socket *s, xplane_message_data *messages, int count);

#endif

