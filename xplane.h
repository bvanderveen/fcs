
#include <arpa/inet.h>

#ifndef __XPLANE_H
#define __XPLANE_H

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

enum xplane_data_index {
    xplane_data_mach_vvi_g = 4,
    xplane_data_ail_elv_rud = 8,
    xplane_data_angular_moments = 15,
    xplane_data_angular_accellerations = 16,
    xplane_data_angular_velocities = 17,
    xplane_data_pitch_roll_heading = 18,
    xplane_data_lat_lon_alt = 20,
    xplane_data_throttle = 25
};
typedef int xplane_data_index;

struct xplane_context {
    int socket;
    struct sockaddr_in listenEndpoint;
    struct sockaddr_in destinationEndpoint;
    void(*data_handler)(struct xplane_context *, xplane_message_data *, int);
    void *context;
};
typedef struct xplane_context xplane_context;

void init_xplane_context(xplane_context *context, int listenPort, int destinationPort);
void xplane_context_read(xplane_context *context);
void xplane_write_data(xplane_context *context, xplane_message_data *messages, int count);

#endif

