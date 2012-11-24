#include "config.h"
#include <math.h>
#include <stdlib.h>

void init_config(core_config *config) {
    // driver.core.waypoint0.lat = 47.258842;
    // driver.core.waypoint0.lon = 11.331075;
    // driver.core.waypoint0.alt = 1919;

    // driver.core.waypoint1.lat = 47.261608;
    // driver.core.waypoint1.lon = 11.356864;
    // driver.core.waypoint1.alt = 1889;

    //driver.core.desiredHeading = 81.15;
    //driver.core.desiredHeading = 270;
    config->turn_coordinator.rudderController.p = 1;
    config->turn_coordinator.rudderController.i = 0;
    config->turn_coordinator.rudderController.d = .1;

    config->pitch_setting.desiredPitch = 7;
    config->pitch_setting.elevatorController.p = .05;
    config->pitch_setting.elevatorController.i = 0;
    config->pitch_setting.elevatorController.d = 0;

    config->course_setting.current_waypoint_index = 1;
    config->course_setting.waypoint_threshold = .00005;
    config->course_setting.course_p = 2;
    config->course_setting.course_i = .0;
    config->course_setting.intercept_angle = 80;
    config->course_setting.intercept_gain = 50;
    config->course_setting.bank_angle = 45;
    config->course_setting.bank_gain = 1;

    config->course_setting.waypoint_count = 4; 

    geopoint *waypoints = (geopoint *)malloc(sizeof(geopoint) * config->course_setting.waypoint_count);

    // square
    waypoints[0].lat = 47.25;
    waypoints[0].lon = 11.32;
    waypoints[1].lat = 47.25;
    waypoints[1].lon = 11.35; 
    waypoints[2].lat = 47.265;
    waypoints[2].lon = 11.35; 
    waypoints[3].lat = 47.265;
    waypoints[3].lon = 11.32;

    // parallelogram
    // waypoints[0].lat = 47.26;
    // waypoints[0].lon = 11.32;
    // waypoints[1].lat = 47.26 - .01;
    // waypoints[1].lon = 11.38;
    // waypoints[2].lat = 47.20;
    // waypoints[2].lon = 11.38 - .01; 
    // waypoints[3].lat = 47.20 + .01;
    // waypoints[3].lon = 11.32 - .01;

    //waypoints[1] = { .lat = 47.262344, .lon = 11.363722 }; // east of LOWI
    // waypoints[2] = { .lat = 47.254662, .lon = 11.361288 }; // east base leg
    // waypoints[3] = { .lat = 47.252052, .lon = 11.330928 }; // west base leg
    // *waypoints = {
    //     { .lat = 47.258297, .lon = 11.324047 }, // west of LOWI
    //     { .lat = 47.262344, .lon = 11.363722 }, // east of LOWI
    //     { .lat = 47.254662, .lon = 11.361288 }, // east base leg
    //     { .lat = 47.252052, .lon = 11.330928 }  // west base leg
    // };

    config->course_setting.waypoints = waypoints;

    config->course_setting.aileron_controller.p = 1;
    config->course_setting.aileron_controller.i = 0;
    config->course_setting.aileron_controller.d = .1;

    // config->course_setting.course_controller.p = .1;
    // config->course_setting.course_controller.i = 0;
    // config->course_setting.course_controller.d = 0;

}

void run_config(core_config *config, core_context *context, float dt) {
    if (context->effector_state.throttle < 1)
        context->effector_state.throttle += .01;

    // some stupid basic mission that is basically to fly figure-eights
    // static int setHeading = 0;
    // static float originalHeading;
    // static float acc = 0;
    // if (!setHeading++) {
    //     originalHeading = context->sensor_state.heading;
    //     config->heading_setting.desiredHeading = originalHeading;
    // }
    // acc += .0000000001 * dt;
    // config->heading_setting.desiredHeading = originalHeading + 140 * sin(acc);

    core_pitch_setting_update(context, (void *)&config->pitch_setting, dt);
    core_course_setting_update(context, (void *)&config->course_setting, dt);
    //core_heading_setting_update(context, (void *)&config->heading_setting, dt);
    core_turn_coordinator_update(context, (void *)&config->turn_coordinator, dt);
}