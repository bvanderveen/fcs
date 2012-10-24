#include "core.h"
#include "energy.h"
#include "heading.h"
#include "turn_coordinator.h"

#ifndef __FCS_CONFIG
#define __FCS_CONFIG

struct core_config {
    core_heading_setting heading_setting;
    core_pitch_setting pitch_setting;
    core_turn_coordinator turn_coordinator;
};
typedef struct core_config core_config;

void init_config(core_config *config);
void run_config(core_config *config, core_context *context, float dt);

#endif