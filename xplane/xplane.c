#include "xplane.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

void xplane_udp_data_handler_function(udp_packet *p, void *context) {
    xplane_socket *xplane_sock = context;

    xplane_message_header *header;
    header = (xplane_message_header *)p->data;

    if (strncmp(header->code, "DATA", 4) == 0) {
        int data_count = (p->count - 5) / 36;

        xplane_message_data *data = malloc(sizeof(xplane_message_data) * data_count);

        for (int i = 0; i < data_count; i++) {
            char *b = p->data + 5 + (36 * i);

            xplane_message_data *data_message = &data[i];

            data_message->index = (uint32_t)(*b);
            memcpy(data_message->data, b + 4, 32);
        }

        xplane_sock->handler(data, data_count, xplane_sock->context);

        free(data);
    }
    else {
        printf("[xplane_socket] discarding bogus message\n");
    }
}

xplane_socket *xplane_socket_alloc(udp_socket *s, xplane_data_handler handler, void *context) {
    xplane_socket *result = malloc(sizeof(xplane_socket));

    s->handler = &xplane_udp_data_handler_function;
    s->context = result;


    result->socket = s;
    result->context = context;
    result->handler = handler;

    
    return result;
}

void xplane_socket_dealloc(xplane_socket *s) {
    free(s);
}

void xplane_socket_read(xplane_socket *s) {
    udp_socket *socket = s->socket;
    udp_socket_read(socket);
}

void xplane_socket_write(xplane_socket *s, xplane_message_data *messages, int count) {
    udp_socket *socket = s->socket;

    int bufferLength = sizeof(xplane_message_data) * count + 5;
    char *buffer = malloc(bufferLength);
    *(buffer + bufferLength) = 0;
    char *b = buffer;

    memcpy(buffer, "DATA\0", 5);
    b += 5;

    for (int i = 0; i < count; i++) {
        xplane_message_data message = messages[i];
        memcpy(b, &message.index, 4);
        b += 4;

        for (int j = 0; j < 8; j++) {
            float value = message.data[j];
            
            // xplane seems to crash if it gets NaN
            if (value != value) {
                printf("got NaN, ignoring\n");
                value = 0;

            }

            memcpy(b, &value, sizeof(float));
            b += sizeof(float);
        }
    }

    udp_socket_write(socket, buffer, bufferLength);
}

// void xplane_udp_socket(udp_socket *socket, core_context *context) {
//     int listen = 49003;
//     int destination = 49000;
//     udp_socket_init(socket, listen, destination, &on_data_message, context);

//     socket.data_handler = &on_data_message;
// }

// void core_xplane_driver_write_effectors(core_xplane_driver *driver) {
//     xplane_message_data messages[2];

//     messages[0].index = xplane_data_ail_elv_rud;
//     messages[0].data[0] = driver->core.effector_state.elv;
//     messages[0].data[1] = driver->core.effector_state.ail;
//     messages[0].data[2] = driver->core.effector_state.rud;
//     messages[0].data[3] = 0;
//     messages[0].data[4] = 0;
//     messages[0].data[5] = 0;
//     messages[0].data[6] = 0;
//     messages[0].data[7] = 0;

//     messages[1].index = xplane_data_throttle;
//     messages[1].data[0] = driver->core.effector_state.throttle;
//     messages[1].data[1] = driver->core.effector_state.throttle;
//     messages[1].data[2] = driver->core.effector_state.throttle;
//     messages[1].data[3] = driver->core.effector_state.throttle;
//     messages[1].data[4] = 0;
//     messages[1].data[5] = 0;
//     messages[1].data[6] = 0;
//     messages[1].data[7] = 0;

//     printf("elv = %f, ail = %f, rud = %f, thr = %f\n", messages[0].data[0], messages[0].data[1], messages[0].data[2], messages[1].data[0]);
//     xplane_write_data(&driver->xplane, messages, 2);
// }

// void core_xplane_driver_write_initial_state(core_xplane_driver *driver) {
//     xplane_message_data messages[2] = {
//         { .index = xplane_data_loc_vel_dist, .data = { 
//             -1280, -900, 900, 
//             50, 120, 0, 
//             //0, 0, 0,
//             -999, -999 } },
//         { .index = xplane_data_pitch_roll_heading, .data = {
//             30.0, 30.0, 18.0,
//             -999, -999, -999, -999, -999 } }
//     };
//     udp_socket_write(&driver->socket, messages, 3);
// }
