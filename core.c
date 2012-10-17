#include "core.h"
#include <stdio.h>
#include <math.h>

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
    controller->error = error;
}

float heading_error(float heading, float desiredHeading) {
    float a = desiredHeading - heading;
    return a + ((a > 180.0) ? -360.0 : ((a < -180.0) ? 360.0 : 0.0));
}

float roll_error(float roll, float desiredRoll) {
    float a = desiredRoll - roll;
    return a; // XXX does not account for extreme banking (i.e., inverted flight)
}

float pitch_error(float pitch, float desiredPitch) {
    return desiredPitch - pitch;
}

void core_update(core_context *context, float dt) {
    if (context->effector_state.throttle < 1)
        context->effector_state.throttle += .01;

    // some stupid basic mission that is basically to swerve back and forth from starting heading
    static int setHeading = 0;
    static float originalHeading;
    static float acc = 0;
    if (!setHeading++) {
        originalHeading = context->sensor_state.heading;
        context->desiredHeading = originalHeading;
    }
    acc += .0000000002 * dt;
    context->desiredHeading = originalHeading + 70 * sin(acc);


    // set pitch
    float pitchError = pitch_error(context->sensor_state.pitch, context->desiredPitch);
    pid_update(&context->elevatorController, pitchError, dt);
    context->effector_state.elv = -context->elevatorController.output;

    // heading control is a PID cascade controller. 
    // outer loop takes heading error and commands bank angle 
    float bearingError = heading_error(context->sensor_state.heading, context->desiredHeading);
    pid_update(&context->aileronController, bearingError, dt);
    // inner loop takes bank angle and commands elevator deflection
    float roll_error = -context->aileronController.output - context->sensor_state.roll;
    pid_update(&context->aileronController2, roll_error, dt);
    context->effector_state.ail = -context->aileronController2.output;

    // coordinate the turn by zeroing lateral G's
    pid_update(&context->rudderController, context->sensor_state.aB, dt);
    context->effector_state.rud = context->rudderController.output;
    printf("aB = %f\n", context->sensor_state.aB);

};