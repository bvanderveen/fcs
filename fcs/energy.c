#include "energy.h"
#include <stdio.h>

#define MAX(A, B) (((A) > (B)) ? (A) : (B))
#define MIN(A, B) (((A) < (B)) ? (A) : (B))

#define GRAVITY (9.8)

float totalSpecificEnergy(float alt, float speed) {
    return GRAVITY * alt + speed * speed / 2;
}

void core_energy_setting_update(core_context *context, void *energy_setting, float dt) {
    // flight path angle (FPA) is not really an angle but 
    // a ratio of climb rate to forward speed

    core_energy_setting *setting = (core_energy_setting *)energy_setting;

    // meters/second
    float currentSpeed = MAX(1, context->sensor_state.cas * 0.514444); // kn -> m/s
    float currentVerticalSpeed = context->sensor_state.vvi * 0.00508; // ft/m -> m/s
    float currentAcceleration = context->sensor_state.aT;

    // actually sin(fpa) = hdot/v, but this is a good appx
    float currentFPA = currentVerticalSpeed / currentSpeed;

    float errorFPA = (setting->desiredFPA - currentFPA);
    float errorAccel = (setting->desiredAcceleration - currentAcceleration);

    setting->specificThrustCommand = 
        -1 * setting->throttleP * (currentFPA + currentAcceleration / GRAVITY) + 
        setting->throttleI * (errorFPA, errorAccel / GRAVITY);

    setting->elevatorRateCommand = 
        setting->elevatorP * (currentFPA * (2 - setting->throttleElevatorBias) - currentAcceleration / GRAVITY * setting->throttleElevatorBias) +
        -1 * setting->elevatorI * (errorAccel / G - errorFPA);

    float thrustError = setting->specificThrustCommand - currentAcceleration;
    pid_update(&setting->thrustController, thrust, dt);
    context->effector_state.throttle = -setting->thrustController.output;

    context->effector_state.elv += elevatorRateCommand;
    setting->previousSpeed = currentSpeed;
}
