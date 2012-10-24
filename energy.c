#include "energy.h"

float pitch_error(float pitch, float desiredPitch) {
    return desiredPitch - pitch;
}

void core_pitch_setting_update(core_context *context, void *pitch_setting, float dt) {
	core_pitch_setting *setting = (core_pitch_setting *)pitch_setting;
    float pitchError = pitch_error(context->sensor_state.pitch, setting->desiredPitch);
    pid_update(&setting->elevatorController, pitchError, dt);
    context->effector_state.elv = -setting->elevatorController.output;
}
