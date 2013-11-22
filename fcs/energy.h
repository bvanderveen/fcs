#include "core.h"
#include "pid_controller.h"

#ifndef __FCS_ENERGY
#define __FCS_ENERGY

struct core_energy_setting {
    float desiredFPA;
    float desiredAcceleration;

    float throttleP;
    float throttleI;

    float elevatorP;
    float elevatorI;

    float throttleErrorIntegral;
    float elevatorErrorIntegral;

    float throttleElevatorBias;

    float specificThrustCommand;
    float elevatorRateCommand;

    pid_controller thrustController;
};
typedef struct core_energy_setting core_energy_setting;

void core_energy_setting_update(core_context *context, void *energy_setting, float dt);

#endif