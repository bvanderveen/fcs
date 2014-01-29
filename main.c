#include "xplane/xplane.h"
#include "fcs/core.h"
#include "fcs/config.h"
#include <stdio.h>
#include <string.h>
#include <sys/time.h>
#include <sys/param.h>
#include <arpa/inet.h>

struct core_xplane_driver {
    udp_socket socket;
    core_config config;
    core_context core;
};
typedef struct core_xplane_driver core_xplane_driver;


int main() {
    state *state = state_alloc(128);

    udp_endpoint xplane_listen_endpoint;
    udp_endpoint_init(&xplane_listen_endpoint, INADDR_ANY, 49003);

    udp_endpoint xplane_broadcast_endpoint;
    udp_endpoint_init(&xplane_broadcast_endpoint, INADDR_ANY, 49000);

    udp_socket xplane_socket = udp_socket_alloc(xplane_listen_endpoint, xplane_broadcast_endpoint, NULL, NULL);


    udp_endpoint state_listen_endpoint;
    udp_endpoint_init(&state_listen_endpoint, INADDR_ANY, 49003);

    udp_endpoint state_broadcast_endpoint;
    udp_endpoint_init(&state_broadcast_endpoint, INADDR_ANY, 49000);

    udp_socket state_socket = udp_socket_alloc(state_listen_endpoint, state_broadcast_endpoint, NULL, NULL);



    message_channel *xplane_channel = message_channel_alloc(xplane_socket, NULL, NULL);

    message_channel *state_channel = message_channel_alloc(state_socket, NULL, NULL);


    message_channel_dealloc(state_channel);
    message_channel_dealloc(xplane_channel);

    state_dealloc(state);

    udp_socket_dealloc(xplane_socket);
    udp_socket_dealloc(state_socket);



    core_config *config;

    core_xplane_driver driver;
    memset(&driver, 0, sizeof(driver));
    driver.xplane.context = &driver;

    init_config(&driver.config);

    udp_socket_init(&driver.socket);

    //core_xplane_driver_write_initial_state(&driver);

    struct timeval t0;
    gettimeofday(&t0, NULL);

    while (1) {
        printf("\n");
        xplane_context_read(&driver.xplane);

        struct timeval t1;
        gettimeofday(&t1, NULL);

        float dt = timeval_subtract(&t1, &t0);

        run_config(&driver.config, &driver.core, dt);

        core_xplane_driver_write_effectors(&driver);

        t0 = t1;
    }

    return 0;
}