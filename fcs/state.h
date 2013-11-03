#include "yajl/yajl_tree.h"

#ifndef __FCS_STATE
#define __FCS_STATE

typedef enum {
	state_value_type_float,
	state_value_type_json
} state_value_type;

struct state_value {
    const char *name;
    state_value_type type;
    union {
	    float float_value;
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

void state_set(state *s, const char *name, float value);
float state_get(state *s, const char *name);

void state_set_json(state *s, const char *name, yajl_val value);
yajl_val state_get_json(state *s, const char *name);

#endif
