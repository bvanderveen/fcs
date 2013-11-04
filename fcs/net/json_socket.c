#include "json_socket.h"
#include <stdlib.h>
#include <stdio.h>

void json_socket_udp_data_handler_function(udp_packet *p, void *context) {
    printf("json_socket_udp_data_handler_function\n");
    json_socket *s = context;

    // XXX buffer overflow waiting to happen
    yajl_val v = yajl_tree_parse(p->data, NULL, 0);

    printf("[json_socket_udp_data_handler_function] handler = %x\n", (unsigned int)s->handler);
    s->handler(v, s->context);

    // XXX memory leak
    //free(v);
}

json_socket *json_socket_alloc(udp_socket *s) {
    json_socket *result = malloc(sizeof(json_socket));

    result->socket = s;

    return result;
}

void json_socket_dealloc(json_socket *s) {
    free(s);
}

void json_socket_read(json_socket *s, json_handler handler) {
    printf("[json_socket_read] 1\n");
    s->socket->context = s;
    s->handler = handler;
    printf("[json_socket_read] handler = %x\n", (unsigned int)handler);
    udp_socket_read(s->socket, json_socket_udp_data_handler_function);
    printf("[json_socket_read] 2\n");
}
