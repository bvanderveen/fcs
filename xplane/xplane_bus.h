#include "xplane_socket.h"
#include "../fcs/state.h"
#include "../fcs/message.h"

#ifndef __XPLANE_BUS_H
#define __XPLANE_BUS_H

void xplane_bus_write_effectors(xplane_socket *socket, state *state);
void message_bus_read_json(json_socket *socket, state *state);
void xplane_bus_read_sensors(xplane_socket *socket, state *state);

#endif