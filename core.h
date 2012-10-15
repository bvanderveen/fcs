
struct core_sensor_state {
    float 
    lat, 
    lon,
    alt,
    pitch,
    roll,
    heading;
};
typedef struct core_sensor_state core_sensor_state;

struct core_effector_state {
    float
    elv,
    ail,
    rud,
    throttle;
};
typedef struct core_effector_state core_effector_state;

struct core_context {
    core_sensor_state sensor_state;
    core_effector_state effector_state;
};
typedef struct core_context core_context;

void core_update(core_context *context, float dt);