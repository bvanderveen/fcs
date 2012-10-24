#include "pid_controller.h"
#include "core.h"

#ifndef __FCS_TURN_COORDINATOR
#define __FCS_TURN_COORDINATOR

struct core_turn_coordinator {
    pid_controller rudderController;
};
typedef struct core_turn_coordinator core_turn_coordinator;

void core_turn_coordinator_update(core_context *context, void *setting, float dt);

#endif
