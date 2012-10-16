#include "core.h"
#include <stdio.h>

void core_sensor_state_print(core_sensor_state *state) {
    printf("lat %04f lon %04f alt %04f pitch %04f roll %04f heading %04f\n", 
        state->lat,
        state->lon, 
        state->alt, 
        state->pitch, 
        state->roll, 
        state->heading);
}

void pid_update(pid_controller *controller, float feedback, float dt) {
    float error = controller->setpoint - feedback;
    controller->integral += error * dt;
    float derivative = (error - controller->error) / dt;
    controller->output = controller->p * error + controller->i * controller->integral + controller->d * derivative;
}

float heading_error(float heading, float desiredHeading) {
    float a = desiredHeading - heading;
    return a + ((a > 180.0) ? -360.0 : ((a < -180.0) ? 360.0 : 0.0));
}

void core_update(core_context *context, float dt) {
    if (context->effector_state.throttle < .9)
        context->effector_state.throttle += .01;

    context->effector_state.elv = .3;

    float bearingError = heading_error(context->sensor_state.heading, context->desiredHeading);
    pid_update(&context->rudderController, bearingError, dt);
    context->effector_state.rud = -context->rudderController.output;
};