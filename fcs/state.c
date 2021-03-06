#include "state.h"
#include <stdlib.h>
#include <assert.h>
#include <string.h>

state *state_alloc(int count) {
    state *result = (state *)malloc(sizeof(state));
    result->count = count;
    result->last = 0;
    result->values = (state_value *)malloc(sizeof(state_value) * count);
    return result;
}

void state_dealloc(state *s) {
    free(s->values);
    free(s);
}

state_value *state_find_by_name(state *s, const char *name) {
    // mad dumb O(n) linear traversal
    for (int i = 0; i < s->last; i++) {
        // XXX buffer overflow
        if (strcmp(s->values[i].name, name) == 0) {
            return &s->values[i];
        }
    }
    return NULL;
}

state_value_type state_get_value_type(state *s, const char *name) {
    state_value *v = state_find_by_name(s, name);

    return v->type;
}

void state_set_float(state *s, const char *name, float value) {
    state_value *v = state_find_by_name(s, name);

    if (!v) {
        if (s->last > s->count)
            assert(0);

        v = &s->values[s->last];
        s->last++;
    }
    else if (v->type != state_value_type_float)
        assert(0);

    v->name = name;
    v->type = state_value_type_float;
    v->u.float_value = value;
}

float state_get_float(state *s, const char *name) {
    state_value *v = state_find_by_name(s, name);

    assert(v != NULL);
    assert(v->type == state_value_type_float);

    return v->u.float_value;
}

void state_set_int(state *s, const char *name, int value) {
    state_value *v = state_find_by_name(s, name);

    if (!v) {
        if (s->last > s->count)
            assert(0);

        v = &s->values[s->last];
        s->last++;
    }
    else if (v->type != state_value_type_int)
        assert(0);

    v->name = name;
    v->type = state_value_type_int;
    v->u.int_value = value;
}

int state_get_int(state *s, const char *name) {
    state_value *v = state_find_by_name(s, name);

    if (v == NULL)
        assert(0);
    else if (v->type != state_value_type_int)
        assert(0);

    return v->u.int_value;
}


void state_set_json(state *s, const char *name, yajl_val value) {
    state_value *v = state_find_by_name(s, name);

    if (!v) {
        if (s->last > s->count)
            assert(0);

        v = &s->values[s->last];
        s->last++;
    }
    else {
        if (v->type != state_value_type_json)
            assert(0);

        yajl_tree_free(v->u.json_value);
    }

    v->name = name;
    v->type = state_value_type_json;
    v->u.json_value = value;
}

yajl_val state_get_json(state *s, const char *name) {
    state_value *v = state_find_by_name(s, name);

    if (!v)
        assert(0);
    else if (v->type != state_value_type_json)
        assert(0);

    return v->u.json_value;
}