#include "core.h"

void core_update(core_context *context, float dt) {
	if (context->effector_state.throttle < .4)
		context->effector_state.throttle += .01;


};