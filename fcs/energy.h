#include "core.h"
#include "pid_controller.h"

#ifndef __FCS_ENERGY
#define __FCS_ENERGY

struct core_energy_setting {
    float desiredAirspeed;
    float desiredAltitude;

    float accelerationGain;
    float verticalSpeedGain;

    pid_controller totalEnergyController;
    pid_controller totalEnergyDistributionController;

    pid_controller throttleController;
    pid_controller elevatorController;

    float previousSpeed;
    float previousSpeedError;
    float previousAltitudeError;
    float fpaIntegral;
    float accelIntegral;

    float thrustCommand;
    float flightPathAngleCommand;
};
typedef struct core_energy_setting core_energy_setting;

void core_energy_setting_update(core_context *context, void *pitch_setting, float dt);

#endif