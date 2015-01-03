#include "heading.h"
#include "pid_controller.h"
#include "math.h"
#include "state.h"

#ifndef __FCS_COURSE
#define __FCS_COURSE

struct fcs_course_state {
    int current_waypoint_index;
    pid_state aileron_controller;
    float course_integral;
};
typedef struct fcs_course_state fcs_course_state;

struct fcs_course_settings {
    float waypoint_threshold;
    float course_p;
    float course_i;
    float aileron_kp;
    float aileron_ki;
    float aileron_kd;
    float intercept_angle;
    float intercept_gain;
    float bank_angle;
    float bank_gain;
    int waypoint_count;
    geopoint *waypoints;
};
typedef struct fcs_course_settings fcs_course_settings;

struct fcs_course_inputs {
    geopoint current_position;
    float heading;
    float roll;
};
typedef struct fcs_course_inputs fcs_course_inputs;

void course_settings_from_state(fcs_course_settings *settings, state *state);

float core_course_setting_update(fcs_course_state *state, fcs_course_settings *settings, fcs_course_inputs *inputs float dt);

#endif