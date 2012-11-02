#include "heading.h"
#include "pid_controller.h"
#include "math.h"

#ifndef __FCS_COURSE
#define __FCS_COURSE

struct fcs_course_setting {
    int current_waypoint_index;
    pid_controller aileron_controller;
    float course_integral;

    float waypoint_threshold;
    float course_p;
    float course_i;
    float intercept_angle;
    float intercept_gain;
    float bank_angle;
    float bank_gain;
    int waypoint_count;
    geopoint *waypoints;
};
typedef struct fcs_course_setting fcs_course_setting;

void core_course_setting_update(core_context *context, void *setting, float dt);

#endif