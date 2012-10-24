#include "pid_controller.h"

void pid_update(pid_controller *controller, float feedback, float dt) {
    float error = controller->setpoint - feedback;
    controller->integral += error * dt;
    float derivative = (error - controller->error) / dt;
    controller->output = controller->p * error + controller->i * controller->integral + controller->d * derivative;
    controller->error = error;
}