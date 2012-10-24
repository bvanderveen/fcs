#include "heading.h"

#ifndef __FCS_COURSE
#define __FCS_COURSE

struct geopoint {
    float
    lat,
    lon;
};
typedef struct geopoint geopoint;

struct fcs_course_setting {
    int current_waypoint_index;
    float waypoint_threshold;
    int waypoint_count;
    geopoint waypoints[];
};
typedef struct fcs_course_setting fcs_course_setting;

void core_course_setting_update(core_context *context, void *setting, float dt);

#endif