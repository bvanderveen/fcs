#ifndef __FCS_CORE
#define __FCS_CORE

struct core_sensor_state {
    float 
    lat, lon, alt,
    cas, vvi;
    aT, aN, aB,
    pitch, roll, heading,
};
typedef struct core_sensor_state core_sensor_state;

void core_sensor_state_print(core_sensor_state *state);

struct core_effector_state {
    float
    elv,
    ail,
    rud,
    throttle;
};
typedef struct core_effector_state core_effector_state;

// struct core_path {
//     core_point *points;
//     int count;
// };
// typedef struct core_path core_path;

// struct core_point {
//     float
//     lat,
//     lon;
// };
// typedef struct core_point core_point;

struct core_context {
    core_sensor_state sensor_state;
    core_effector_state effector_state;
};
typedef struct core_context core_context;


#endif
