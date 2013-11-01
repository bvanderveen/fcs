#include "../fcs/udp.h"

#ifndef __XPLANE_H
#define __XPLANE_H

enum xplane_data_index {
    xplane_data_speeds = 3,
    xplane_data_mach_vvi_g = 4,
    xplane_data_ail_elv_rud = 8,
    xplane_data_angular_moments = 15,
    xplane_data_angular_accellerations = 16,
    xplane_data_angular_velocities = 17,
    xplane_data_pitch_roll_heading = 18,
    xplane_data_lat_lon_alt = 20,
    xplane_data_loc_vel_dist = 21,
    xplane_data_throttle = 25
};
typedef int xplane_data_index;

struct xplane_message_header
{
    char code[4];
    uint8_t reserved;
};
typedef struct xplane_message_header xplane_message_header;

struct xplane_message_data
{
    uint32_t index;
    float data[8];
};
typedef struct xplane_message_data xplane_message_data;

typedef void(*xplane_data_handler)(xplane_message_data *, int, void *);

struct xplane_socket {
    udp_socket *socket;
    xplane_data_handler handler;
    void *context;
};
typedef struct xplane_socket xplane_socket;

xplane_socket *xplane_socket_alloc(udp_socket *s, xplane_data_handler handler, void *context);
void xplane_socket_dealloc(xplane_socket *s);

void xplane_socket_read(xplane_socket *s);
void xplane_socket_write(xplane_socket *s, xplane_message_data *messages, int count);

#endif

