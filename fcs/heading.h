#include "core.h"
#include "pid_controller.h"

#ifndef __FCS_HEADING
#define __FCS_HEADING

struct core_heading_setting {
    float desiredHeading;
    pid_controller aileronController;
    pid_controller aileronController2;
};
typedef struct core_heading_setting core_heading_setting;

void core_heading_setting_update(core_context *context, void *setting, float dt);

#endif