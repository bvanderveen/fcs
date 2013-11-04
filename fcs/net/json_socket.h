#import "udp_socket.h"
#import "yajl/yajl_tree.h"

#ifndef __FCS_MESSAGE_H
#define __FCS_MESSAGE_H

typedef void (*json_handler)(yajl_val, void *);

struct json_socket {
	udp_socket *socket;
	json_handler handler;
	void *context;
};
typedef struct json_socket json_socket;

json_socket *json_socket_alloc(udp_socket *s);
void json_socket_dealloc(json_socket *s);

void json_socket_read(json_socket *s, json_handler handler);

#endif