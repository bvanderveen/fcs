#include "xplane.h"
#include "core.h"
#include <stdio.h>
#include <string.h>
#include <sys/time.h>
#include <sys/param.h>

struct core_xplane_driver {
    xplane_context xplane;
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

    printf("elv = %f, ail = %f, rud = %f\n", messages[0].data[0], messages[0].data[1], messages[0].data[2]);

    messages[1].index = xplane_data_throttle;
    messages[1].data[0] = driver->core.effector_state.throttle;
    messages[1].data[1] = driver->core.effector_state.throttle;
    messages[1].data[2] = driver->core.effector_state.throttle;
    messages[1].data[3] = driver->core.effector_state.throttle;
    messages[1].data[4] = 0;
    messages[1].data[5] = 0;
    messages[1].data[6] = 0;
    messages[1].data[7] = 0;

    xplane_write_data(&driver->xplane, messages, 2);
}

void on_data_message(xplane_context *xplane, xplane_message_data *messages, int count) {
    core_xplane_driver *driver = (core_xplane_driver *)xplane->context;

    for (int i = 0; i < count; i++) {
        xplane_message_data *message = messages + i * sizeof(xplane_message_data);

        switch (message->index) {
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
                driver->core.sensor_state.aN = message->data[4];
                driver->core.sensor_state.aT = message->data[5];
                driver->core.sensor_state.aB = message->data[6];
                break;
        }
    }
    core_sensor_state_print(&driver->core.sensor_state);
}

int timeval_subtract(struct timeval *t2, struct timeval *t1)
{
    return (t2->tv_usec + 1000000 * t2->tv_sec) - (t1->tv_usec + 1000000 * t1->tv_sec);
}

int main() {
    core_xplane_driver driver;
    memset(&driver, 0, sizeof(driver));
    driver.xplane.context = &driver;

    driver.core.maxBearingError = 10;

    driver.core.waypoint0.lat = 47.258842;
    driver.core.waypoint0.lon = 11.331075;
    driver.core.waypoint0.alt = 1919;

    driver.core.waypoint1.lat = 47.261608;
    driver.core.waypoint1.lon = 11.356864;
    driver.core.waypoint1.alt = 1889;

    driver.core.desiredHeading = 81.15;
    //driver.core.desiredHeading = 270;
    driver.core.rudderController.p = 1;
    driver.core.rudderController.i = .0;
    driver.core.rudderController.d = 4;

    driver.core.aileronController.p = 4;
    driver.core.aileronController.i = .000000002;
    driver.core.aileronController.d = 1;

    driver.core.aileronController2.p = .1;
    driver.core.aileronController2.i = .0;
    driver.core.aileronController2.d = .1;

    driver.core.desiredPitch = -1;
    driver.core.elevatorController.p = .05;
    driver.core.elevatorController.i = 0;
    driver.core.elevatorController.d = 0;

    init_xplane_context(&driver.xplane, 49003, 49000);
    driver.xplane.data_handler = &on_data_message;

    struct timeval t0;
    gettimeofday(&t0, NULL);

    while (1) {
        xplane_context_read(&driver.xplane);

        struct timeval t1;
        gettimeofday(&t1, NULL);

        float dt = 1000 * timeval_subtract(&t1, &t0);

        core_update(&driver.core, dt);

        core_xplane_driver_write_effectors(&driver);

        t0 = t1;
    }

    return 0;
}