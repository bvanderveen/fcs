#include "message.h"
#include <assert.h>

void message_json_handler_function(yajl_val j, void *context) {
    state *s = context;

    LLog("[message_json_handler_function] will read value type\n");
    LLog("[message_json_handler_function] value is %d\n", (unsigned int)j);

    LLog("[message_json_handler_function] value type is %d\n", j->type);

    if (!YAJL_IS_OBJECT(j)) {
        LLog("[message_json_handler_function] value was not an object, bailing out\n");
        return;
    }

    int num_vals = YAJL_GET_OBJECT(j)->len;
    for (int i = 0; i < num_vals; i++) 
    {
        const char *name = YAJL_GET_OBJECT(j)->keys[i];
        LLog("[message_json_handler_function] did get name = %s\n", name);

        yajl_val value = YAJL_GET_OBJECT(j)->values[i];

        if (YAJL_IS_INTEGER(value)) {
            LLog("[message_json_handler_function] value is integer\n");
            state_set_int(s, name, YAJL_GET_INTEGER(value));
        }
        else if (YAJL_IS_DOUBLE(value)) {
            LLog("[message_json_handler_function] value is double\n");
            state_set_float(s, name, (float)YAJL_GET_DOUBLE(value));
        }
        else if (YAJL_IS_TRUE(value)) {
            LLog("[message_json_handler_function] value is true\n");
            state_set_int(s, name, 1);
        }
        else if (YAJL_IS_FALSE(value)) {
            LLog("[message_json_handler_function] value is false\n");
            state_set_int(s, name, 0);
        }
        else {
            LLog("[message_json_handler_function] value is complex\n");
            state_set_json(s, name, value);
        }
    }
}

void message_bus_read_json(json_socket *socket, state *state) {
    LLog("[message_bus_read_json] will call json_socket_read handler = %x\n", (unsigned int)message_json_handler_function);
    json_socket_read(socket, message_json_handler_function, state);
    LLog("[message_bus_read_json] did call json_socket_read\n");
}

void message_bus_write_json(yajl_gen g, void *context) {
    state *state = context;

    yajl_val output_values = state_get_json(state, STATE_OUTPUT_VALUES);

    assert(YAJL_IS_ARRAY(output_values));

    yajl_gen_map_open(g);

    int len = YAJL_GET_ARRAY(output_values)->len;
    for (int i = 0; i < len; i++) {
        char *k = YAJL_GET_STRING(YAJL_GET_ARRAY(output_values)->values[i]);

        state_value_type type = state_get_value_type(state, k);

        yajl_gen_string(g, k, strlen(k));

        float float_value;
        int int_value;
        yajl_val json_value;
        switch (type) {
            case state_value_type_float:
                float_value = state_get_float(state, k);
                yajl_gen_double(g, (double)float_value);
                break;
            case state_value_type_int:
                int_value = state_get_int(state, k);
                yajl_gen_integer(g, int_value);
                break;
            case state_value_type_json:
                json_value = state_get_json(state, k);
                json_write_value(g, json_value);
                break;
            default:
                assert(0);
                break;
        }
    }

    yajl_gen_map_close(g);
}

void message_bus_write_values(json_socket *socket, state *state) {
    json_socket_write(socket, message_bus_write_json, state);
}