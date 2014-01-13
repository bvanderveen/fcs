#include "../net/json_socket.h"
#include "../state.h"

#ifndef __FCS_BUS_MESSAGE_H
#define __FCS_BUS_MESSAGE_H

void message_bus_read_json(json_socket *socket, state *state);
void message_bus_write_values(json_socket *socket, state *state);

#endif
