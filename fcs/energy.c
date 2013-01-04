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

    // meters
    float currentAltitude = context->sensor_state.alt * 0.3048; // ft -> m



    //float currentAcceleration = (currentSpeed - setting->previousSpeed) / dt;
    //float currentFlightPathAngle = currentVerticalSpeed / currentSpeed;
    float speedError = (setting->desiredAirspeed - currentSpeed);
    float altitudeError = (setting->desiredAltitude - currentAltitude);

    float speedErrorDerivative = speedError * setting->accelerationGain;
    float altitudeErrorDerivative = altitudeError * setting->verticalSpeedGain;

    // float speedErrorDerivative = (speedError - setting->previousSpeedError) / dt;
    // float altitudeErrorDerivative = (altitudeError - setting->previousAltitudeError) / dt;

    float accelerationError = speedErrorDerivative / GRAVITY;
    float fpaError = altitudeErrorDerivative / currentSpeed;

    // pid_update(&setting->totalEnergyController, accelerationError + fpaError, dt);
    // float commandedThrust = setting->totalEnergyController.output;
    setting->fpaIntegral += (accelerationError + fpaError) * dt;

    float p = setting->totalEnergyController.p * (-(currentVerticalSpeed / currentSpeed) - accelerationError);
    float i = setting->totalEnergyController.i * setting->fpaIntegral;
    printf("p = %f i = %f\n", p, i);
    float commandedThrust = p + i;
    context->effector_state.throttle = -commandedThrust;

    //pid_update(&setting->throttleController, commandedThrust, dt);
    //context->effector_state.throttle = setting->throttleController.output;

    setting->accellIntegral += ();

    pid_update(&setting->totalEnergyDistributionController, accelerationError - fpaError, dt);
    float commandedPitch = setting->totalEnergyDistributionController.output;

    context->effector_state.elv = 0.2;//-commandedPitch;
    // pid_update(&setting->elevatorController, commandedPitch, dt);
    // context->effector_state.elv = -setting->elevatorController.output;

    printf("desired total specific energy %f\n", totalSpecificEnergy(setting->desiredAltitude, setting->desiredAirspeed));
    printf("current total specific energy %f\n", totalSpecificEnergy(currentAltitude, currentSpeed));
    
    printf("currentSpeed = %f\n", currentSpeed);
    printf("currentVerticalSpeed = %f\n", currentVerticalSpeed);

    printf("speedError = %f\n", speedError);
    printf("altitudeError = %f\n", altitudeError);

    printf("speedErrorDerivative = %f\n", speedErrorDerivative);
    printf("altitudeErrorDerivative = %f\n", altitudeErrorDerivative);

    printf("accelerationError = %f\n", accelerationError);
    printf("fpaError = %f\n", fpaError);
    //printf("flightPathAngleError = %f\n", flightPathAngleError);
    //printf("accelerationError + flightPathAngleError = %f\n", accelerationError + flightPathAngleError);
    //printf("accelerationError - flightPathAngleError = %f\n", accelerationError - flightPathAngleError);
    printf("commanded ∂ thrust = %f\n", commandedThrust);
    printf("commanded ∂ pitch = %f\n", commandedPitch);

    setting->previousSpeed = currentSpeed;
}
