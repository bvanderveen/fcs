#include "course.h"
#include "math.h"
#include <math.h>
#include <stdio.h>

#define EARTH_RADIUS (6373.0)

float course_sigma(fcs_course_setting *setting, float cross_track_error) {
    //return atan(setting->intercept_gain * cross_track_error /) / (M_PI/2) * TO_RADIANS(setting->intercept_angle);
    float exponent = expf(-1 * setting->intercept_gain * cross_track_error / 2);
    return TO_RADIANS(setting->intercept_angle) * ((exponent - 1) / (exponent + 1));
}

float course_sigma_derivative(fcs_course_setting *setting, float cross_track_error) {
    float exponent = expf(-1 * setting->intercept_gain * cross_track_error / 2);
    printf("exponent %f\n", exponent);
    float exponent_plus_1 = exponent + 1;
    return -1 * setting->intercept_gain * TO_RADIANS(setting->intercept_angle) * exponent / (exponent_plus_1 * exponent_plus_1);
}

float heading_between(geopoint *p0, geopoint *p1) {
    // tan(angle) = ∂lat / ∂lon
    geopoint r;
    difference(p0, p1, &r);
    float result = atan(TO_RADIANS(r.lon) / TO_RADIANS(r.lat)) + M_PI;
    // if (result > M_PI_2)
    //     result -= M_2_PI;
    // if (result < 0)
    //     result += M_2_PI;
    return result;
}

float course_change(fcs_course_setting *setting, float course_error, float cross_track_error, float dt) {
    float sigma = course_sigma(setting, cross_track_error);
    float error = sigma - course_error;
    printf("course_error %f\n", TO_DEGREES(course_error));
    printf("sigma %f\n", TO_DEGREES(sigma));
    printf("error %f\n", TO_DEGREES(error));
    setting->course_integral += error * dt;
    float sigma_derivative = course_sigma_derivative(setting, cross_track_error);
    printf("sigma_derivative %f\n", sigma_derivative);
    float sin_course_error = sin(course_error);
    printf("sin(course_error) %f\n", sin_course_error);

    float p = setting->course_p * error;
    float i = setting->course_i * setting->course_integral;
    float d = sigma_derivative * sin_course_error;

    printf("p = %f\n", p);
    printf("i = %f\n", i);
    printf("d = %f\n", d);

    float result = p + i + d;

    return result;
}

float course_control_roll(fcs_course_setting *setting, float current_roll, float commanded_roll, float dt) {
    float roll_error = commanded_roll - current_roll;
    pid_update(&setting->aileron_controller, roll_error, dt);
    return setting->aileron_controller.output;
}

float course_bank_angle(fcs_course_setting *setting, float course_change) {
    return atan(setting->bank_gain * course_change) / (M_PI / 2)
    * TO_RADIANS(setting->bank_angle);
}

float initial_bearing_between(geopoint *p0, geopoint *p1) {
    float dlon = TO_RADIANS(p1->lon - p0->lon);
    float lat0 = TO_RADIANS(p0->lat);
    float lat1 = TO_RADIANS(p1->lat);
    return atan2(sin(dlon) * cos(lat1), cos(lat0) * sin(lat1) - sin(lat0) * cos(lat1) * cos(dlon));
}

float haversine_distance_between(geopoint *p0, geopoint *p1) {
    float dlat = TO_RADIANS(p0->lat - p1->lat);
    float dlon = TO_RADIANS(p1->lon - p0->lon);
    float lat0 = TO_RADIANS(p0->lat);
    float lat1 = TO_RADIANS(p1->lat);

    float sin_dlat_2 = sin(dlat / 2);
    float sin_dlon_2 = sin(dlon / 2);

    float a = sin_dlat_2 * sin_dlat_2 + sin_dlon_2 * sin_dlon_2 * cos(lat0) * cos(lat1);
    return 2 * atan2(sqrt(a), sqrt(1 - a)) * EARTH_RADIUS;
}

float cross_track_distance(geopoint *p0, geopoint *p1, geopoint *r) {
    // printf("p0 "); geopoint_print(p0);
    // printf("p1 "); geopoint_print(p1);
    // printf("r  "); geopoint_print(r);
    float position_bearing = initial_bearing_between(p0, r);
    float waypoint_bearing = initial_bearing_between(p0, p1);
    float hav_dist = haversine_distance_between(p0, r);
    // printf("position bearing %f, %f\n", position_bearing, TO_DEGREES(position_bearing));
    // printf("waypoint bearing %f, %f\n", waypoint_bearing, TO_DEGREES(waypoint_bearing));
    // printf("haversine dist   %f\n", hav_dist);
    return asin(sin(hav_dist / EARTH_RADIUS) * sin(position_bearing - waypoint_bearing)) * EARTH_RADIUS;
}

float heading_error(float h0, float h1) {
    float a = h1 - h0;
    printf("a %f\n", TO_DEGREES(a));
    float result = a + ((a > M_PI) ? -(M_PI * 2) : ((a < -M_PI) ? (M_PI * 2) : 0.0));
    printf("result %f\n", TO_DEGREES(result));
    return result;
}

void follow_segment_2(core_context *context, fcs_course_setting *setting, geopoint *p0, geopoint *p1, geopoint *r, float dt) {
    float current_course = TO_RADIANS(context->sensor_state.heading);

    float desired_course = initial_bearing_between(p0, p1);

    float course_error = heading_error(current_course, desired_course);

    float cross_track_error = cross_track_distance(p0, p1, r);

    printf("cross_track_error (km) = %f\n", cross_track_error);
    printf("current_course (deg) = %f\n", TO_DEGREES(current_course));
    printf("desired_course (deg) = %f\n", TO_DEGREES(desired_course));

    float course_signal = course_change(setting, -course_error, cross_track_error, dt);
        ///* + d_desired_course / dt */;

    printf("course_signal (deg) = %f\n", TO_DEGREES(course_signal));

    float commanded_bank_angle = course_bank_angle(setting, course_signal);
    printf("commanded_bank_angle (deg) = %f\n", TO_DEGREES(commanded_bank_angle));

    float current_roll = TO_RADIANS(context->sensor_state.roll);
    printf("current bank angle (deg) = %f\n", TO_DEGREES(current_roll));
    float aileron_signal = course_control_roll(setting, current_roll, commanded_bank_angle, dt);

    context->effector_state.ail = -aileron_signal;
}

void core_course_setting_update(core_context *context, void *course_setting, float dt) {
    fcs_course_setting *setting = (fcs_course_setting *)course_setting;

    geopoint current_waypoint = setting->waypoints[setting->current_waypoint_index];
    geopoint current_position = { .lat = context->sensor_state.lat, .lon = context->sensor_state.lon };

    // if distance to next waypoint less than threshold, set next course
    float waypoint_distance_squared = distance_squared(&current_waypoint, &current_position);

    if (waypoint_distance_squared < setting->waypoint_threshold) {
        setting->current_waypoint_index++;

        // if course is finished, head back toward first waypoint
        if (setting->current_waypoint_index == setting->waypoint_count) {
            setting->current_waypoint_index = 0;
        }

        current_waypoint = setting->waypoints[setting->current_waypoint_index];
        printf("moving to next waypoint\n");
    }

    int previous_waypoint_index = setting->current_waypoint_index == 0 ? 
        (setting->waypoint_count - 1) : 
        (setting->current_waypoint_index - 1);

    geopoint previous_waypoint = setting->waypoints[previous_waypoint_index];

    printf("waypoint_distance_squared (%f) %f \n", setting->waypoint_threshold, waypoint_distance_squared);
    printf("current_waypoint_index %d\n", setting->current_waypoint_index);
    printf("previous_waypoint_index %d\n", previous_waypoint_index);

    // printf("previous_waypoint "); geopoint_print(&previous_waypoint);
    // printf("current_waypoint "); geopoint_print(&current_waypoint);
    // printf("current_position "); geopoint_print(&current_position);

    // fly course
    follow_segment_2(context, course_setting, &previous_waypoint, &current_waypoint, &current_position, dt);
}