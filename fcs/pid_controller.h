#ifndef __FCS_PID_CONTROLLER
#define __FCS_PID_CONTROLLER

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

#endif