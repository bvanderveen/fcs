#ifndef __PID_CONTROLLER_H
#define __PID_CONTROLLER_H

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