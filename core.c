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

    static int setHeading = 0;
    static float originalHeading;
    static float acc = 0;

    if (!setHeading++) {
        originalHeading = context->sensor_state.heading;
        context->desiredHeading = originalHeading;
    }

    //acc += .00000000003 * dt;
    //context->desiredHeading = originalHeading + 180 * sin(acc);
    // estimate wind.
    // draw line between waypoints. this is commanded path.
    // calculate vector to destination point. this is naive path
    // calculate vector normal to commanded path. rate-limited steering vector


    //waypoint0


    float pitchError = pitch_error(context->sensor_state.pitch, context->desiredPitch);
    pid_update(&context->elevatorController, pitchError, dt);
    context->effector_state.elv = -context->elevatorController.output;

    // control heading with bank angle.
    float bearingError = heading_error(context->sensor_state.heading, context->desiredHeading);

    // reasonable limits. don't break your neck following the carrot.
    float maxBearingError = context->maxBearingError;
    bearingError = (bearingError > maxBearingError) ? 
        maxBearingError : 
        (bearingError < -maxBearingError ? 
            -maxBearingError : 
            bearingError);
    printf("desiredHeading = %f, error = %f\n", context->desiredHeading, bearingError);

    pid_update(&context->aileronController, bearingError, dt);

    context->aileronController2.setpoint = 0;
    float roll_error = -context->aileronController.output - context->sensor_state.roll;
    printf("commanded roll = %f roll error = %f\n", -context->aileronController.output, roll_error);
    pid_update(&context->aileronController2, -context->aileronController.output - context->sensor_state.roll, dt);
    context->effector_state.ail = -context->aileronController2.output;

    printf("lateral accelleration = %f\n", context->sensor_state.aB);
    pid_update(&context->rudderController, context->sensor_state.aB, dt);
    context->effector_state.rud = -context->rudderController.output;
};