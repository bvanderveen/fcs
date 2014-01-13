#include "../net/xplane_socket.h"
#include "../state.h"

#ifndef __FCS_BUS_XPLANE_H
#define __FCS_BUS_XPLANE_H

void xplane_bus_write_effectors(xplane_socket *socket, state *state);
void xplane_bus_read_sensors(xplane_socket *socket, state *state);

#endif