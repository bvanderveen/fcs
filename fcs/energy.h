#include "core.h"
#include "pid_controller.h"

#ifndef __FCS_ENERGY
#define __FCS_ENERGY

struct core_pitch_setting {
    float desiredPitch;
    pid_controller elevatorController;
};
typedef struct core_pitch_setting core_pitch_setting;

void core_pitch_setting_update(core_context *context, void *pitch_setting, float dt);

#endif