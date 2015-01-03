#ifndef __FCS_PID_CONTROLLER
#define __FCS_PID_CONTROLLER

struct pid_state {
    float error, integral;
};
typedef struct pid_state pid_state;

float pid_update(pid_state *state, float error, float kp, float ki, float kd, float dt);

#endif