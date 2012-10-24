#include "pid_controller.h"
#include "core.h"

#ifndef __CORE_TURN_COORDINATOR_H
#define __CORE_TURN_COORDINATOR_H

struct core_turn_coordinator {
    pid_controller rudderController;
};
typedef struct core_turn_coordinator core_turn_coordinator;

void core_turn_coordinator_update(core_context *context, void *setting, float dt);

#endif
