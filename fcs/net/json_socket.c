#include "json_socket.h"
#include <stdlib.h>
#include <stdio.h>
#include "yajl/yajl_gen.h"

void json_socket_udp_data_handler_function(udp_packet *p, void *context) {
    json_socket *s = context;
    json_handler handler = s->handler;
    void *ctx = s->context;


    LLog("[json_socket_udp_data_handler_function] got data count %d\n", p->count);
    LLog("[json_socket_udp_data_handler_function] got data\n%s\n", p->data);

    LLog("[json_socket_udp_data_handler_function] will parse json\n");

    size_t error_buffer_len = sizeof(char) * 2048;
    char *error_buffer = malloc(error_buffer_len);

    // XXX buffer overflow waiting to happen
    yajl_val v = yajl_tree_parse(p->data, error_buffer, error_buffer_len);

    if (!v) {
        LLog("[json_socket_udp_data_handler_function] error parsing json\n");
        LLog("%s", error_buffer);
        LLog("[json_socket_udp_data_handler_function] end error\n");
    }

    free(error_buffer);

    LLog("[json_socket_udp_data_handler_function] did parse json, value = %d\n", (unsigned int)v);

    LLog("[json_socket_udp_data_handler_function] will call handler = %x\n", (unsigned int)handler);
    handler(v, ctx);
    LLog("[json_socket_udp_data_handler_function] did call handler = %x\n", (unsigned int)handler);

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

    LLog("[json_socket_read] will call udp_socket_read handler = %x, context = %x\n", (unsigned int)handler, (unsigned int)context);
    udp_socket_read(s->socket, json_socket_udp_data_handler_function, s);
    LLog("[json_socket_read] did call udp_socket_read\n");
}

void json_socket_write(json_socket *s, json_writer writer, void *context) {

    yajl_gen g = yajl_gen_alloc(NULL);

    writer(g, context);

    char *data;
    size_t len;
    yajl_gen_get_buf(g, &data, &len);

    udp_socket_write(s->socket, data, len);

    yajl_gen_free(g);
}
