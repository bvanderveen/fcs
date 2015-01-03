#include "pid_controller.h"

float pid_update(pid_state *state, float error, float kp, float ki, float kd, float dt) {
    float derivative = (error - state->error) / dt;
    
    state->integral += error * dt;
    state->error = error;

    return kp * error + ki * state->integral + kd * derivative;
}