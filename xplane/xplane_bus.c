#include "xplane_bus.h"

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
    messages[0].data[0] = state_get(state, STATE_EFFECTOR_ELEVATOR);
    messages[0].data[1] = state_get(state, STATE_EFFECTOR_AILERON);
    messages[0].data[2] = state_get(state, STATE_EFFECTOR_RUDDER);
    messages[0].data[3] = 0;
    messages[0].data[4] = 0;
    messages[0].data[5] = 0;
    messages[0].data[6] = 0;
    messages[0].data[7] = 0;

    float t = state_get(state, STATE_EFFECTOR_THROTTLE);
    
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

void xplane_bus_xplane_data_handler_function(xplane_message_data *ms, int count, void *context) {
    state *state = context;

     for (int i = 0; i < count; i++) {
        xplane_message_data message = ms[i];

        switch (message.index) {
            case xplane_data_pitch_roll_heading:
                state_set(state, STATE_SENSOR_PITCH, message.data[0]);
                state_set(state, STATE_SENSOR_ROLL, message.data[1]);
                state_set(state, STATE_SENSOR_HEADING, message.data[2]);
                break;
            case xplane_data_lat_lon_alt:
                state_set(state, STATE_SENSOR_LATITUDE, message.data[0]);
                state_set(state, STATE_SENSOR_LONGITUDE, message.data[1]);
                state_set(state, STATE_SENSOR_ALTITUDE, message.data[2]);
                break;
            case xplane_data_mach_vvi_g:
                state_set(state, STATE_SENSOR_ACCEL_T, message.data[4]);
                state_set(state, STATE_SENSOR_ACCEL_N, message.data[5]);
                state_set(state, STATE_SENSOR_ACCEL_B, message.data[6]);
                break;
        }
    }
}

void xplane_bus_read_sensors(xplane_socket *socket, state *state) {
    socket->handler = xplane_bus_xplane_data_handler_function;
    socket->context = state;
    xplane_socket_read(socket);
}
