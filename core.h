
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

void core_sensor_state_print(core_sensor_state *state);

struct core_effector_state {
    float
    elv,
    ail,
    rud,
    throttle;
};
typedef struct core_effector_state core_effector_state;

struct pid_controller {
    float 
    setpoint,
    output,
    error,
    integral,
    p, i, d;
};
typedef struct pid_controller pid_controller;

void pid_update(pid_controller *controller, float feedback, float dt);

struct core_context {
    core_sensor_state sensor_state;
    core_effector_state effector_state;
    
    float desiredHeading;
    pid_controller rudderController;
};
typedef struct core_context core_context;

void core_update(core_context *context, float dt);