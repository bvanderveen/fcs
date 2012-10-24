#include "course.h"


// TODO meters or what?
float distance_squared(geopoint p0, geopoint p1) {
    return 0;
}

void follow_segment(core_context *context, geopoint *p0, geopoint *p1, float dt) {

}

void core_course_setting_update(core_context *context, void *course_setting, float dt) {
    fcs_course_setting *setting = (fcs_course_setting *)course_setting;

    // if course is finished, head back toward first waypoint
    if (setting->current_waypoint_index == setting->waypoint_count) {
        setting->current_waypoint_index = 0;
    }

    geopoint current_waypoint = setting->waypoints[setting->current_waypoint_index];
    geopoint current_position = { .lat = context->sensor_state.lat, .lon = context->sensor_state.lon };

    // if distance to next waypoint less than threshold, set next course
    if (distance_squared(current_waypoint, current_position) < setting->waypoint_threshold) {
        setting->current_waypoint_index++;
    }

    int previous_waypoint_index = setting->current_waypoint_index == 0 ? 
        (setting->waypoint_count - 1) : 
        (setting->current_waypoint_index  - 1);

    geopoint previous_waypoint = setting->waypoints[previous_waypoint_index];

    // fly course
    follow_segment(context, &previous_waypoint, &current_waypoint, dt);
}