
#ifndef __FCS_STATE
#define __FCS_STATE

struct state_value {
    const char *name;
    float value;
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

#endif
