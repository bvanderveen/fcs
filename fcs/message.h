#import "udp.h"

#ifndef __FCS_MESSAGE_H
#define __FCS_MESSAGE_H

struct message_data
{
    uint32_t index;
    float data[8];
};
typedef struct message_data message_data;

typedef void (*)(message_channel *, message_data *, int, void *) message_handler;

struct message_channel {
	udp_socket *socket;
	message_handler *handler;
	void *context;
};
typedef struct message_channel message_channel;

message_channel *message_channel_alloc(udp_socket *s, message_handler *, void *context);
void message_channel_dealloc(message_channel *c);

void message_channel_receive(message_channel *c);
void message_channel_send(message_channel *c, message_data *messages, int count);

#endif