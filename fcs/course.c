#include "course.h"
#include "math.h"
#include <math.h>
#include <stdio.h>

#define EARTH_RADIUS (6373.0)

void course_settings_from_state(fcs_course_settings *settings, state *state) {

}

float course_sigma(fcs_course_settings *settings, float cross_track_error) {
    float exponent = expf(-1 * setting->intercept_gain * cross_track_error / 2);
    return TO_RADIANS(setting->intercept_angle) * ((exponent - 1) / (exponent + 1));
}

float course_sigma_derivative(fcs_course_settings *settings, float cross_track_error) {
    float exponent = expf(-1 * settings->intercept_gain * cross_track_error / 2);
    float exponent_plus_1 = exponent + 1;
    return -1 * settings->intercept_gain * TO_RADIANS(settings->intercept_angle) * exponent / (exponent_plus_1 * exponent_plus_1);
}

float course_change(fcs_course_state *state, fcs_course_settings *settings, float relative_course, float cross_track_error, float dt) {
    float sigma = course_sigma(settings, cross_track_error);
    float error = sigma - relative_course;
    state->course_integral += error * dt;
    float sigma_derivative = course_sigma_derivative(setting, cross_track_error);
    float sin_relative_course = sin(relative_course);
    float p = setting->course_p * error;
    float i = setting->course_i * setting->course_integral;
    float d = sigma_derivative * sin_relative_course;
    float result = p + i + d;

    // printf("relative_course %f\n", TO_DEGREES(relative_course));
    // printf("sigma %f\n", TO_DEGREES(sigma));
    // printf("error %f\n", TO_DEGREES(error));
    // printf("sigma_derivative %f\n", sigma_derivative);
    // printf("sin(relative_course) %f\n", sin_relative_course);
    // printf("p = %f\n", p);
    // printf("i = %f\n", i);
    // printf("d = %f\n", d);

    return result;
}

float course_control_roll(fcs_course_state *state, fcs_course_settings *settings, float current_roll, float commanded_roll, float dt) {
    float roll_error = commanded_roll - current_roll;
        pid_update(&state->aileron_controller, roll_error, settings->aileron_kp, settings->aileron_ki, settings->aileron_kd, dt);
    return state->aileron_controller.output;
}

float course_bank_angle(fcs_course_settings *settings, float course_change) {
    return atan(settings->bank_gain * course_change) / (M_PI / 2)
    * TO_RADIANS(settings->bank_angle);
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
    float position_bearing = initial_bearing_between(p0, r);
    float waypoint_bearing = initial_bearing_between(p0, p1);
    float hav_dist = haversine_distance_between(p0, r);
    return asin(sin(hav_dist / EARTH_RADIUS) * sin(position_bearing - waypoint_bearing)) * EARTH_RADIUS;
}

float normalize_bearing(float a) {
    return a + ((a > M_PI) ? -(M_PI * 2) : ((a < -M_PI) ? (M_PI * 2) : 0.0));
}

float relative_bearing(float h0, float h1) {
    return normalize_bearing(h1 - h0);
}

float follow_segment(fcs_course_state *state, fcs_course_settings *settings, float heading, float roll, geopoint *p0, geopoint *p1, geopoint *r, float dt) {
    float current_course = TO_RADIANS(heading);
    float desired_course = fmod(initial_bearing_between(p0, p1) + M_PI * 2, M_PI * 2);
    float relative_course = relative_bearing(current_course, desired_course);

    float cross_track_error = cross_track_distance(p0, p1, r);


    float course_signal = course_change(settings, -relative_course, cross_track_error, dt);
        ///* + d_desired_course / dt */;


    float commanded_bank_angle = course_bank_angle(settings, course_signal);

    float current_roll = TO_RADIANS(roll);
    float aileron_signal = course_control_roll(state, current_roll, commanded_bank_angle, dt);

    
    // printf("cross_track_error (km) = %f\n", cross_track_error);
    // printf("current_course (deg) = %f\n", TO_DEGREES(current_course));
    // printf("desired_course (deg) = %f\n", TO_DEGREES(desired_course));
    // printf("course_signal (deg) = %f\n", TO_DEGREES(course_signal));
    // printf("commanded_bank_angle (deg) = %f\n", TO_DEGREES(commanded_bank_angle));
    // printf("current bank angle (deg) = %f\n", TO_DEGREES(current_roll));

    return -aileron_signal;
}

float core_course_setting_update(fcs_course_state *state, fcs_course_settings *settings, fcs_course_inputs *inputs, float dt) {
    geopoint current_position = inputs->current_position;
    float heading = inputs->heading;
    float roll = inputs->roll;

    geopoint current_waypoint = settings->waypoints[state->current_waypoint_index];

    // if distance to next waypoint less than threshold, set next course
    float waypoint_distance_squared = distance_squared(&current_waypoint, &current_position);

    if (waypoint_distance_squared < settings->waypoint_threshold) {
        state->current_waypoint_index++;

        // if course is finished, head back toward first waypoint
        if (state->current_waypoint_index == settings->waypoint_count) {
            state->current_waypoint_index = 0;
        }

        current_waypoint = settings->waypoints[state->current_waypoint_index];
        printf("moving to next waypoint\n");
    }

    int previous_waypoint_index = state->current_waypoint_index == 0 ? 
        (settings->waypoint_count - 1) : 
        (state->current_waypoint_index - 1);

    geopoint previous_waypoint = settings->waypoints[previous_waypoint_index];

    printf("waypoint_distance_squared (%f) %f \n", settings->waypoint_threshold, waypoint_distance_squared);
    printf("current_waypoint_index %d\n", settings->current_waypoint_index);
    printf("previous_waypoint_index %d\n", previous_waypoint_index);

    // fly course
    return follow_segment(state, settings, heading, roll, &previous_waypoint, &current_waypoint, &current_position, dt);
}