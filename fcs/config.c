#include "config.h"
#include <math.h>

void init_config(core_config *config) {
    // driver.core.waypoint0.lat = 47.258842;
    // driver.core.waypoint0.lon = 11.331075;
    // driver.core.waypoint0.alt = 1919;

    // driver.core.waypoint1.lat = 47.261608;
    // driver.core.waypoint1.lon = 11.356864;
    // driver.core.waypoint1.alt = 1889;

    //driver.core.desiredHeading = 81.15;
    //driver.core.desiredHeading = 270;
    config->turn_coordinator.rudderController.p = .000000001;
    config->turn_coordinator.rudderController.i = .0000000015;
    config->turn_coordinator.rudderController.d = 110;

    config->heading_setting.aileronController.p = 2;
    config->heading_setting.aileronController.i = .00000000001;
    config->heading_setting.aileronController.d = 1;

    config->heading_setting.aileronController2.p = .1;
    config->heading_setting.aileronController2.i = .0;
    config->heading_setting.aileronController2.d = .1;

    config->pitch_setting.desiredPitch = 0;
    config->pitch_setting.elevatorController.p = .05;
    config->pitch_setting.elevatorController.i = 0;
    config->pitch_setting.elevatorController.d = 0;
}

void run_config(core_config *config, core_context *context, float dt) {
    if (context->effector_state.throttle < 1)
        context->effector_state.throttle += .01;

    // some stupid basic mission that is basically to fly figure-eights
    static int setHeading = 0;
    static float originalHeading;
    static float acc = 0;
    if (!setHeading++) {
        originalHeading = context->sensor_state.heading;
        config->heading_setting.desiredHeading = originalHeading;
    }
    acc += .0000000001 * dt;
    config->heading_setting.desiredHeading = originalHeading + 140 * sin(acc);

    core_pitch_setting_update(context, (void *)&config->pitch_setting, dt);
    core_heading_setting_update(context, (void *)&config->heading_setting, dt);
    core_turn_coordinator_update(context, (void *)&config->turn_coordinator, dt);
}