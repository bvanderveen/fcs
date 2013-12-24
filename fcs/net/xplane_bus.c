#include "xplane_bus.h"
#include <stdio.h>
#include <assert.h>

enum xplane_data_index {
    xplane_data_speeds = 3,
    xplane_data_mach_vvi_g = 4,
    xplane_data_ail_elv_rud = 8,
    xplane_data_angular_moments = 15,
    xplane_data_angular_accellerations = 16,
    xplane_data_angular_velocities = 17,
    xplane_data_pitch_roll_heading = 18,
    xplane_data_lat_lon_alt = 20,
    xplane_data_loc_vel_dist = 21,
    xplane_data_throttle = 25
};
typedef int xplane_data_index;

void xplane_bus_write_effectors(xplane_socket *socket, state *state) {
    xplane_message_data messages[2];

    messages[0].index = xplane_data_ail_elv_rud;
    messages[0].data[0] = state_get_float(state, STATE_EFFECTOR_ELEVATOR);
    messages[0].data[1] = state_get_float(state, STATE_EFFECTOR_AILERON);
    messages[0].data[2] = state_get_float(state, STATE_EFFECTOR_RUDDER);
    messages[0].data[3] = 0;
    messages[0].data[4] = 0;
    messages[0].data[5] = 0;
    messages[0].data[6] = 0;
    messages[0].data[7] = 0;

    float t = state_get_float(state, STATE_EFFECTOR_THROTTLE);
    
    messages[1].index = xplane_data_throttle;
    messages[1].data[0] = t;
    messages[1].data[1] = t;
    messages[1].data[2] = t;
    messages[1].data[3] = t;
    messages[1].data[4] = 0;
    messages[1].data[5] = 0;
    messages[1].data[6] = 0;
    messages[1].data[7] = 0;

    xplane_socket_write(socket, messages, 2);
}

void message_xplane_data_handler_function(xplane_message_data *ms, int count, void *context) {
    state *state = context;

     for (int i = 0; i < count; i++) {
        xplane_message_data message = ms[i];

        switch (message.index) {
            case xplane_data_pitch_roll_heading:
                state_set_float(state, STATE_SENSOR_PITCH, message.data[0]);
                state_set_float(state, STATE_SENSOR_ROLL, message.data[1]);
                state_set_float(state, STATE_SENSOR_HEADING, message.data[2]);
                break;
            case xplane_data_lat_lon_alt:
                state_set_float(state, STATE_SENSOR_LATITUDE, message.data[0]);
                state_set_float(state, STATE_SENSOR_LONGITUDE, message.data[1]);
                state_set_float(state, STATE_SENSOR_ALTITUDE, message.data[2]);
                break;
            case xplane_data_mach_vvi_g:
                state_set_float(state, STATE_SENSOR_ACCEL_T, message.data[4]);
                state_set_float(state, STATE_SENSOR_ACCEL_N, message.data[5]);
                state_set_float(state, STATE_SENSOR_ACCEL_B, message.data[6]);
                break;
        }
    }
}

void xplane_bus_read_sensors(xplane_socket *socket, state *state) {
    xplane_socket_read(socket, message_xplane_data_handler_function, state);
}

void message_json_handler_function(yajl_val j, void *context) {
    state *s = context;

    printf("[message_json_handler_function] will read value type\n");
    printf("[message_json_handler_function] value is %d\n", (unsigned int)j);

    printf("[message_json_handler_function] value type is %d\n", j->type);

    if (!YAJL_IS_OBJECT(j)) {
        printf("[message_json_handler_function] value was not an object, bailing out\n");
        return;
    }

    int num_vals = YAJL_GET_OBJECT(j)->len;
    for (int i = 0; i < num_vals; i++) 
    {
        const char *name = YAJL_GET_OBJECT(j)->keys[i];
        printf("[message_json_handler_function] did get name = %s\n", name);

        yajl_val value = YAJL_GET_OBJECT(j)->values[i];

        if (YAJL_IS_INTEGER(value)) {
            printf("[message_json_handler_function] value is integer\n");
            state_set_int(s, name, YAJL_GET_INTEGER(value));
        }
        else if (YAJL_IS_DOUBLE(value)) {
            printf("[message_json_handler_function] value is double\n");
            state_set_float(s, name, (float)YAJL_GET_DOUBLE(value));
        }
        else if (YAJL_IS_TRUE(value)) {
            printf("[message_json_handler_function] value is true\n");
            state_set_int(s, name, 1);
        }
        else if (YAJL_IS_FALSE(value)) {
            printf("[message_json_handler_function] value is false\n");
            state_set_int(s, name, 0);
        }
        else {
            printf("[message_json_handler_function] value is complex\n");
            state_set_json(s, name, value);
        }
    }
}

void message_bus_read_json(json_socket *socket, state *state) {
    printf("[message_bus_read_json] will call json_socket_read handler = %x\n", (unsigned int)message_json_handler_function);
    json_socket_read(socket, message_json_handler_function, state);
    printf("[message_bus_read_json] did call json_socket_read\n");
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
