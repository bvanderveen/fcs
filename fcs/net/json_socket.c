#include "json_socket.h"
#include <stdlib.h>
#include <stdio.h>

void json_socket_udp_data_handler_function(udp_packet *p, void *context) {
    
    json_socket *s = context;
    json_handler handler = s->handler;
    void *ctx = s->context;


    printf("[json_socket_udp_data_handler_function] got data count %d\n", p->count);
    printf("[json_socket_udp_data_handler_function] got data\n%s\n", p->data);

    printf("[json_socket_udp_data_handler_function] will parse json\n");

    size_t error_buffer_len = sizeof(char) * 2048;
    char *error_buffer = malloc(error_buffer_len);

    // XXX buffer overflow waiting to happen
    yajl_val v = yajl_tree_parse(p->data, error_buffer, error_buffer_len);

    if (!v) {
        printf("[json_socket_udp_data_handler_function] error parsing json\n");
        printf("%s", error_buffer);
        printf("[json_socket_udp_data_handler_function] end error\n");
    }

    printf("[json_socket_udp_data_handler_function] did parse json, value = %d\n", (unsigned int)v);

    printf("[json_socket_udp_data_handler_function] will call handler = %x\n", (unsigned int)handler);
    handler(v, ctx);
    printf("[json_socket_udp_data_handler_function] did call handler = %x\n", (unsigned int)handler);

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

void json_socket_read(json_socket *s, json_handler handler, void *context) {
    s->handler = handler;
    s->context = context;

    printf("[json_socket_read] will call udp_socket_read handler = %x, context = %x\n", (unsigned int)handler, (unsigned int)context);
    udp_socket_read(s->socket, json_socket_udp_data_handler_function, s);
    printf("[json_socket_read] did call udp_socket_read\n");
}
