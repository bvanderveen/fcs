#include "state.h"

#ifndef __FCS_CORE
#define __FCS_CORE

typedef void(*core_callback)(state *, void *);

struct core_callbacks {
    core_callback read_sensors;
    core_callback read_commands;
    core_callback write_effectors;
    core_callback write_indicators;
};
typedef struct core_callbacks core_callbacks;

struct core_control_state {
	fcs_course_state course_state;
};
typedef struct core_control_state state;

void core_loop(core_callbacks *callbacks, state *state, core_control_state *control_state, struct timeval t0, struct timeval *t1, void *context);

#endif
