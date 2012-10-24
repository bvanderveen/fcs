#include "turn_coordinator.h"

void core_turn_coordinator_update(core_context *context, void *setting, float dt) {
	core_turn_coordinator *coordinator = (core_turn_coordinator *)setting;
    // coordinate the turn by zeroing lateral G's
    pid_update(&coordinator->rudderController, context->sensor_state.aB, dt);
    context->effector_state.rud = coordinator->rudderController.output;
}