#include "core.h"
#include "state.h"
#include "bus/xplane.h"
#include "time.h"

void core_control(state *state, core_control_state *control_state, float dt) {
    // copy state to control data structures

    fcs_course_settings course_settings;
    course_settings_from_state(&settings, state);

    geopoint current_position = { 
        .lat = state_get_float(STATE_SENSOR_LATITUDE), 
        .lon = state_get_float(STATE_SENSOR_LONGITUDE) 
    };

    float heading = state_get_float(STATE_SENSOR_HEADING);
    float roll = state_get_float(STATE_SENSOR_ROLL);

    float aileron_signal = core_course_setting_update(control_state.course_state, &course_settings, current_position, heading, roll, dt);

    // fly course (lat, lon, heading, roll, waypoints, current waypount) -> (ail, course_integral, aileron_controller_integral)
    // coordinate turn
    // adjust energy rates

    // copy control data structures to state
}

void core_loop(core_callbacks *callbacks, state *state, core_control_state *control_state, struct timeval *t0, struct timeval *t1, void *context) {
    callbacks->read_sensors(state, context);
    callbacks->read_commands(state, context);

    struct timeval t;
    gettimeofday(t, NULL);

    float dt = timeval_subtract(t1, t);

    core_control(state, control_state, dt);

    callbacks->write_effectors(state, context);
    callbacks->write_indicators(state, context);

    *t1 = t;
}
