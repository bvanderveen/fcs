#include "yajl/yajl_tree.h"

#ifndef __FCS_STATE
#define __FCS_STATE

typedef enum {
    state_value_type_float,
    state_value_type_int,
    state_value_type_json
} state_value_type;

struct state_value {
    const char *name;
    state_value_type type;
    union {
        float float_value;
        int int_value;
        yajl_val json_value;
    } u;
};
typedef struct state_value state_value;

struct state {
    int last, count;
    state_value* values;
};
typedef struct state state;

state *state_alloc(int count);
void state_dealloc(state *s);

state_value_type state_get_value_type(state *s, const char *name);

void state_set_float(state *s, const char *name, float value);
float state_get_float(state *s, const char *name);

void state_set_int(state *s, const char *name, int value);
int state_get_int(state *s, const char *name);

void state_set_json(state *s, const char *name, yajl_val value);
yajl_val state_get_json(state *s, const char *name);

#define STATE_OUTPUT_VALUES "state.output.values"

#define STATE_EFFECTOR_AILERON "state.effector.aileron"
#define STATE_EFFECTOR_ELEVATOR "state.effector.elevator"
#define STATE_EFFECTOR_RUDDER "state.effector.rudder"
#define STATE_EFFECTOR_THROTTLE "state.effector.throttle"

#define STATE_SENSOR_LATITUDE "state.sensor.latitude"
#define STATE_SENSOR_LONGITUDE "state.sensor.longitude"
#define STATE_SENSOR_ALTITUDE "state.sensor.altitude"
#define STATE_SENSOR_AIRSPEED "state.sensor.airspeed"
#define STATE_SENSOR_CLIMB_RATE "state.sensor.climbrate"
#define STATE_SENSOR_ACCEL_T "state.sensor.aT"
#define STATE_SENSOR_ACCEL_N "state.sensor.aN"
#define STATE_SENSOR_ACCEL_B "state.sensor.aB"
#define STATE_SENSOR_PITCH "state.sensor.pitch"
#define STATE_SENSOR_ROLL "state.sensor.roll"
#define STATE_SENSOR_HEADING "state.sensor.heading"

#endif
