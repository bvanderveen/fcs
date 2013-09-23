#include "xplane/xplane.h"
#include "fcs/core.h"
#include "fcs/config.h"
#include <stdio.h>
#include <string.h>
#include <sys/time.h>
#include <sys/param.h>

struct core_xplane_driver {
    xplane_context xplane;
    core_config config;
    core_context core;
};
typedef struct core_xplane_driver core_xplane_driver;

void core_xplane_driver_write_effectors(core_xplane_driver *driver) {
    xplane_message_data messages[2];

    messages[0].index = xplane_data_ail_elv_rud;
    messages[0].data[0] = driver->core.effector_state.elv;
    messages[0].data[1] = driver->core.effector_state.ail;
    messages[0].data[2] = driver->core.effector_state.rud;
    messages[0].data[3] = 0;
    messages[0].data[4] = 0;
    messages[0].data[5] = 0;
    messages[0].data[6] = 0;
    messages[0].data[7] = 0;

    messages[1].index = xplane_data_throttle;
    messages[1].data[0] = driver->core.effector_state.throttle;
    messages[1].data[1] = driver->core.effector_state.throttle;
    messages[1].data[2] = driver->core.effector_state.throttle;
    messages[1].data[3] = driver->core.effector_state.throttle;
    messages[1].data[4] = 0;
    messages[1].data[5] = 0;
    messages[1].data[6] = 0;
    messages[1].data[7] = 0;

    printf("elv = %f, ail = %f, rud = %f, thr = %f\n", messages[0].data[0], messages[0].data[1], messages[0].data[2], messages[1].data[0]);
    xplane_write_data(&driver->xplane, messages, 2);
}

void core_xplane_driver_write_initial_state(core_xplane_driver *driver) {
    xplane_message_data messages[2] = {
        { .index = xplane_data_loc_vel_dist, .data = { 
            -1280, -900, 900, 
            50, 120, 0, 
            //0, 0, 0,
            -999, -999 } },
        { .index = xplane_data_pitch_roll_heading, .data = {
            30.0, 30.0, 18.0,
            -999, -999, -999, -999, -999 } }
    };
    xplane_write_data(&driver->xplane, messages, 3);
}

void on_data_message(xplane_context *xplane, xplane_message_data *messages, int count) {
    core_xplane_driver *driver = (core_xplane_driver *)xplane->context;

    for (int i = 0; i < count; i++) {
        xplane_message_data *message = messages + i * sizeof(xplane_message_data);

        switch (message->index) {
            case xplane_data_speeds:
                driver->core.sensor_state.cas = message->data[0]; // IAS
            case xplane_data_pitch_roll_heading:
                driver->core.sensor_state.pitch = message->data[0];
                driver->core.sensor_state.roll = message->data[1];
                driver->core.sensor_state.heading = message->data[2];
                break;
            case xplane_data_lat_lon_alt:
                driver->core.sensor_state.lat = message->data[0];
                driver->core.sensor_state.lon = message->data[1];
                driver->core.sensor_state.alt = message->data[2];
                break;
            case xplane_data_mach_vvi_g:
                driver->core.sensor_state.vvi = message->data[2];
                driver->core.sensor_state.aN = message->data[4];
                driver->core.sensor_state.aT = message->data[5];
                driver->core.sensor_state.aB = message->data[6];
                break;
        }
    }
    //core_sensor_state_print(&driver->core.sensor_state);
}

int timeval_msec(struct timeval *t)
{
    return 1000 * (t->tv_usec + 1000000 * t->tv_sec)
}

int timeval_subtract(struct timeval *t2, struct timeval *t1)
{
    return timeval_msec(t2) - timeval_msec(t1);
}

int main() {
    core_xplane_driver driver;
    memset(&driver, 0, sizeof(driver));
    driver.xplane.context = &driver;

    init_config(&driver.config);

    int listen = 49003;
    int destination = 49000;
    init_xplane_context(&driver.xplane, listen, destination);
    driver.xplane.data_handler = &on_data_message;

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