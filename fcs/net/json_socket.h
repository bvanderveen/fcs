#import "udp_socket.h"
#import "yajl/yajl_tree.h"
#import "yajl/yajl_gen.h"

#ifndef __FCS_JSON_SOCKET_H
#define __FCS_JSON_SOCKET_H

typedef void (*json_handler)(yajl_val, void *);
typedef void (*json_writer)(yajl_gen, void *);

struct json_socket {
    udp_socket *socket;
    json_handler handler;
    void *context;
};
typedef struct json_socket json_socket;

json_socket *json_socket_alloc(udp_socket *s);
void json_socket_dealloc(json_socket *s);

void json_socket_read(json_socket *s, json_handler handler, void *context);
void json_socket_write(json_socket *s, json_writer writer, void *context);

#endif