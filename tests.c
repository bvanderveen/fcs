#include "fcs/state.h"
#include "fcs/net/udp_socket.h"
#include "fcs/net/json_socket.h"
#include "fcs/net/xplane_socket.h"
#include "fcs/net/hxstream.h"
#include "fcs/bus/xplane.h"
#include "fcs/bus/message.h"
#include "fcs/debug.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <arpa/inet.h>
#include <stdlib.h>

typedef enum {
    hx_stream_callback_type_start,
    hx_stream_callback_type_tag,
    hx_stream_callback_type_data,
}
hx_stream_callback_type;

typedef struct {
    hx_stream_callback_type type;
    uint8_t data;
} hx_stream_callback_info;

#define TEST_GROUP(T) {printf("\033[1;36m--- " T "\033[0m\n");}
#define IT_SHOULD(WHAT, HOW) { \
printf("\033[1;34mit should " WHAT "...\033[0m"); \
HOW; \
printf("\033[1;32mOK\033[0m\n"); \
}

#define assert_float(ACTUAL, EXPECTED) assert(((ACTUAL) > ((EXPECTED) - .001)) && ((ACTUAL) < ((EXPECTED) + .001)))

void test_udp_data_handler_function(udp_packet *p, void *context) {
    udp_packet **packet = context;
    *packet = udp_packet_copy(p);
}

void test_xplane_data_handler_function(xplane_message_data *ms, int count, void *context) {
    xplane_message_data *messages = context;

    memcpy(&messages[0], &ms[0], XPLANE_MESSAGE_DATA_SIZE);
    memcpy(&messages[1], &ms[1], XPLANE_MESSAGE_DATA_SIZE);
}

void test_json_handler_function(yajl_val j, void *context) {
    yajl_val *json = context;
    *json = j;
}

void test_hx_stream_decoder_frame_start_callback(void *context) {
    hx_stream_callback_info info = {
        .type = hx_stream_callback_type_start,
    };

    hx_stream_callback_info *infoContext = (hx_stream_callback_info *)context;
    *infoContext = info;
}

void test_hx_stream_decoder_frame_tag_callback(void *context, uint8_t tag) {
    hx_stream_callback_info info = {
        .type = hx_stream_callback_type_tag,
        .data = tag
    };

    hx_stream_callback_info *infoContext = (hx_stream_callback_info *)context;
    *infoContext = info;
}

void test_hx_stream_decoder_frame_data_callback(void *context, uint8_t data) {
    hx_stream_callback_info info = {
        .type = hx_stream_callback_type_data,
        .data = data
    };

    hx_stream_callback_info *infoContext = (hx_stream_callback_info *)context;
    *infoContext = info;
}

void test() {

    udp_endpoint xplane_listen_ep;
    udp_endpoint_init(&xplane_listen_ep, INADDR_ANY, 49000);

    udp_endpoint xplane_broadcast_ep;
    udp_endpoint_init(&xplane_broadcast_ep, INADDR_ANY, 49001);

    udp_endpoint upstream_controller_broadcast_ep;
    udp_endpoint_init(&upstream_controller_broadcast_ep, INADDR_ANY, 49002);

    udp_endpoint monitor_listen_ep;
    udp_endpoint_init(&monitor_listen_ep, INADDR_ANY, 49003);

    udp_endpoint any_ep;
    udp_endpoint_init(&any_ep, INADDR_ANY, 0);

    
    TEST_GROUP("state")
    {
        state *state = NULL;
        IT_SHOULD("initialize state",
        {
            state = state_alloc(4);

            assert(state != NULL);
        });

        IT_SHOULD("store pi", {
            const char *key = "PI";

            state_set_float(state, key, 3.14159);
            float result = state_get_float(state, key);

            assert(state_get_value_type(state, key) == state_value_type_float);
            assert((result > 3.14158) && (result < 3.14160));
        });

        IT_SHOULD("store 42", {
            const char *key = "fourtytwo";

            state_set_int(state, key, 42);
            assert(state_get_value_type(state, key) == state_value_type_int);
            int result = state_get_int(state, key);

            assert(result == 42);
        });

        IT_SHOULD("store json", {
            const char *key = "jaysahn";

            char *json = "{\"data\":123}";            

            yajl_val v = yajl_tree_parse(json, NULL, 0);

            state_set_json(state, key, v);
            yajl_val result = state_get_json(state, key);

            assert(state_get_value_type(state, key) == state_value_type_json);
            assert(YAJL_IS_OBJECT(result));
            assert(strncmp(YAJL_GET_OBJECT(result)->keys[0], "data", 4) == 0);
            assert(YAJL_GET_INTEGER(YAJL_GET_OBJECT(result)->values[0]) == 123);
        });

        state_dealloc(state);
    }

    udp_packet *received_packet;

    TEST_GROUP("udp")
    {
        udp_endpoint ep;
        udp_endpoint_init(&ep, ipv4_address_from_string("192.168.1.1"), 90);

        IT_SHOULD("get address", {
            uint32_t addr = udp_endpoint_address(&ep);
            assert(addr == 0xc0a80101);
        });

        IT_SHOULD("get port", {
            int port = udp_endpoint_port(&ep);
            assert(port == 90);
        });

        udp_endpoint destination_ep;
        udp_endpoint_init(&destination_ep, INADDR_ANY, 49001);
        udp_endpoint source_ep;
        udp_endpoint_init(&source_ep, INADDR_ANY, 49002);

        IT_SHOULD("round trip data", {
            udp_socket *listener = udp_socket_alloc(&destination_ep, &source_ep);
            udp_socket *broadcaster = udp_socket_alloc(&source_ep, &destination_ep);

            char *packet_data = "some data";
            udp_socket_write(broadcaster, (unsigned char *)packet_data, 9);
            udp_socket_read(listener, test_udp_data_handler_function, &received_packet);

            assert(udp_endpoint_port(received_packet->ep) == 49002);
            assert(strncmp(received_packet->data, packet_data, 9) == 0);

            udp_socket_dealloc(listener);
            udp_socket_dealloc(broadcaster);
        });
    }

    xplane_message_data *received_messages = NULL;

    TEST_GROUP("xplane")
    {
        xplane_message_data fake_xplane_data[2] = {
            { .index = 1, .data = { .1, .2, .3, .4, .5, .6, .7, .8 } },
            { .index = 2, .data = { .8, .7, .6, .5, .4, .3, .2, .1 } }
        };

        IT_SHOULD("send data to xplane", {
            udp_socket *raw = udp_socket_alloc(&xplane_broadcast_ep, &any_ep);
            udp_socket *wrapped = udp_socket_alloc(&any_ep, &xplane_broadcast_ep);
            xplane_socket *xplane_sock = xplane_socket_alloc(wrapped);

            {
                xplane_socket_write(xplane_sock, &fake_xplane_data[0], 2);

                udp_socket_read(raw, &test_udp_data_handler_function, &received_packet);

                assert(strncmp(received_packet->data, "DATA\0", 5) == 0);

                xplane_message_data *data0 = (xplane_message_data *)(received_packet->data + 5);
                xplane_message_data *data1 = (xplane_message_data *)(received_packet->data + 5 + XPLANE_MESSAGE_DATA_SIZE);
                
                assert(data0->index == 1);
                assert_float(data0->data[0], .1);
                assert(data1->index == 2);
                assert_float(data1->data[0], .8);
            }

            udp_socket_dealloc(wrapped);
            udp_socket_dealloc(raw);
            xplane_socket_dealloc(xplane_sock);
        });

        IT_SHOULD("receive data from xplane", {
            received_messages = calloc(2, XPLANE_MESSAGE_DATA_SIZE);
            unsigned char *xplane_data_bytes = malloc(XPLANE_MESSAGE_HEADER_SIZE + 2 * XPLANE_MESSAGE_DATA_SIZE);
            udp_socket *raw = udp_socket_alloc(&any_ep, &xplane_broadcast_ep);
            udp_socket *wrapped = udp_socket_alloc(&xplane_broadcast_ep, &any_ep);
            xplane_socket *xplane_sock = xplane_socket_alloc(wrapped);
            
            {
                unsigned char *p = &xplane_data_bytes[0];

                memcpy(p, "DATA\0", 5);
                p += 5;

                memcpy(p, &fake_xplane_data[0], XPLANE_MESSAGE_DATA_SIZE);
                p += XPLANE_MESSAGE_DATA_SIZE;

                memcpy(p, &fake_xplane_data[1], XPLANE_MESSAGE_DATA_SIZE);

                udp_socket_write(raw, &xplane_data_bytes[0], (XPLANE_MESSAGE_DATA_SIZE * 2 + XPLANE_MESSAGE_HEADER_SIZE));

                xplane_socket_read(xplane_sock, &test_xplane_data_handler_function, received_messages);

                xplane_message_data *d0;
                d0 = &received_messages[0];
                xplane_message_data *d1;
                d1 = &received_messages[1];

                assert(d0->index == 1);
                assert_float(d0->data[0], .1);
                assert_float(d0->data[1], .2);
                assert_float(d0->data[2], .3);
                assert_float(d0->data[3], .4);
                assert_float(d0->data[4], .5);
                assert_float(d0->data[5], .6);
                assert_float(d0->data[6], .7);
                assert_float(d0->data[7], .8);

                assert(d1->index == 2);
                assert_float(d1->data[0], .8);
                assert_float(d1->data[1], .7);
                assert_float(d1->data[2], .6);
                assert_float(d1->data[3], .5);
                assert_float(d1->data[4], .4);
                assert_float(d1->data[5], .3);
                assert_float(d1->data[6], .2);
                assert_float(d1->data[7], .1);
            }

            udp_socket_dealloc(raw);
            udp_socket_dealloc(wrapped);
            xplane_socket_dealloc(xplane_sock);
            free(received_messages);
            free(xplane_data_bytes);
        });
    }

    TEST_GROUP("messages") 
    {
        xplane_message_data mock_xplane_data[3] = {
            { .index = 18, .data = { .9, .8, .7, 0, 0, 0, 0, 0 } },
            { .index = 20, .data = { 42, -122, 3000, 0, 0, 0, 0, 0 } },
            { .index = 4, .data = { 0, 0, 0, 0, .5, -1, .1, 0 } }
        };

        IT_SHOULD("read sensors from xplane to state", {
            state *state = state_alloc(13);
            udp_socket *raw = udp_socket_alloc(&any_ep, &xplane_broadcast_ep);
            udp_socket *wrapped = udp_socket_alloc(&xplane_broadcast_ep, &any_ep);
            xplane_socket *xplane_sock = xplane_socket_alloc(wrapped);
            unsigned char *xplane_data_bytes = malloc(XPLANE_MESSAGE_HEADER_SIZE + XPLANE_MESSAGE_DATA_SIZE * 3);

            {
                unsigned char *p = xplane_data_bytes;

                memcpy(p, "DATA\0", 5);
                p += 5;

                memcpy(p, &mock_xplane_data[0], XPLANE_MESSAGE_DATA_SIZE);
                p += XPLANE_MESSAGE_DATA_SIZE;

                memcpy(p, &mock_xplane_data[1], XPLANE_MESSAGE_DATA_SIZE);
                p += XPLANE_MESSAGE_DATA_SIZE;

                memcpy(p, &mock_xplane_data[2], XPLANE_MESSAGE_DATA_SIZE);

                udp_socket_write(raw, xplane_data_bytes, XPLANE_MESSAGE_HEADER_SIZE + XPLANE_MESSAGE_DATA_SIZE * 3);

                xplane_bus_read_sensors(xplane_sock, state);

                assert_float(state_get_float(state, STATE_SENSOR_PITCH), .9);
                assert_float(state_get_float(state, STATE_SENSOR_ROLL), .8);
                assert_float(state_get_float(state, STATE_SENSOR_HEADING), .7);

                assert_float(state_get_float(state, STATE_SENSOR_LATITUDE), 42);
                assert_float(state_get_float(state, STATE_SENSOR_LONGITUDE), -122);
                assert_float(state_get_float(state, STATE_SENSOR_ALTITUDE), 3000);

                assert_float(state_get_float(state, STATE_SENSOR_ACCEL_T), .5);
                assert_float(state_get_float(state, STATE_SENSOR_ACCEL_N), -1);
                assert_float(state_get_float(state, STATE_SENSOR_ACCEL_B), .1);
            }

            udp_socket_dealloc(wrapped);
            udp_socket_dealloc(raw);
            xplane_socket_dealloc(xplane_sock);
            state_dealloc(state);
            free(xplane_data_bytes);
        });

        IT_SHOULD("write effectors from state to xplane", {
            state *state = state_alloc(13);
            udp_socket *raw = udp_socket_alloc(&xplane_broadcast_ep, &any_ep);
            udp_socket *wrapped = udp_socket_alloc(&any_ep, &xplane_broadcast_ep);
            xplane_socket *xplane_sock = xplane_socket_alloc(wrapped);

            {
                state_set_float(state, STATE_EFFECTOR_AILERON, .4);
                state_set_float(state, STATE_EFFECTOR_ELEVATOR, .5);
                state_set_float(state, STATE_EFFECTOR_RUDDER, .6);
                state_set_float(state, STATE_EFFECTOR_THROTTLE, .7);

                xplane_bus_write_effectors(xplane_sock, state);

                udp_socket_read(raw, test_udp_data_handler_function, &received_packet);

                assert(strncmp(received_packet->data, "DATA\0", 5) == 0);

                xplane_message_data *messages = (xplane_message_data *)(received_packet->data + 5);
                
                assert(messages[0].index == 8);
                assert_float(messages[0].data[0], .5);
                assert_float(messages[0].data[1], .4);
                assert_float(messages[0].data[2], .6);
                assert_float(messages[0].data[3], .0);
                assert_float(messages[0].data[4], .0);
                assert_float(messages[0].data[5], .0);
                assert_float(messages[0].data[6], .0);
                assert_float(messages[0].data[7], .0);

                assert(messages[1].index == 25);
                assert_float(messages[1].data[0], .7);
                assert_float(messages[1].data[1], .7);
                assert_float(messages[1].data[2], .7);
                assert_float(messages[1].data[3], .7);
                assert_float(messages[1].data[4], .0);
                assert_float(messages[1].data[5], .0);
                assert_float(messages[1].data[6], .0);
                assert_float(messages[1].data[7], .0);
            }

            udp_socket_dealloc(wrapped);
            udp_socket_dealloc(raw);
            xplane_socket_dealloc(xplane_sock);
            state_dealloc(state);
        });

        IT_SHOULD("read json messages from upstream controller", {
            state *state = state_alloc(13);
            udp_socket *raw = udp_socket_alloc(&any_ep, &xplane_broadcast_ep);
            udp_socket *wrapped = udp_socket_alloc(&xplane_broadcast_ep, &any_ep);
            json_socket *json_sock = json_socket_alloc(wrapped);
            
            {
                char *json2 = "{" \
                    "\"a_float\":1024.0,"\
                    "\"an_int\":42,"\
                    "\"a_boolean_true\":true," \
                    "\"a_boolean_false\":false," \
                    "\"some_waypoints\":[[-122.2, 47.7],[-122.25, 47.75]]"\
                    "}";

                udp_socket_write(raw, (unsigned char *)json2, strlen(json2));
            
                message_bus_read_json(json_sock, state);
            
                int an_int = state_get_int(state, "an_int");
                assert(an_int == 42);

                int a_bool = state_get_int(state, "a_boolean_true");
                assert(a_bool != 0);

                int a_bool_false = state_get_int(state, "a_boolean_false");
                assert(a_bool_false == 0);

                float a_float = state_get_float(state, "a_float");
                assert_float(a_float, 1024);

                yajl_val waypoints = state_get_json(state, "some_waypoints");

                assert(YAJL_IS_ARRAY(waypoints));
                assert(YAJL_GET_ARRAY(waypoints)->len == 2);

                yajl_val w0 = YAJL_GET_ARRAY(waypoints)->values[0];
                assert(YAJL_IS_ARRAY(w0));
                assert_float(YAJL_GET_DOUBLE(YAJL_GET_ARRAY(w0)->values[0]), -122.2);
                assert_float(YAJL_GET_DOUBLE(YAJL_GET_ARRAY(w0)->values[1]), 47.7);

                yajl_val w1 = YAJL_GET_ARRAY(waypoints)->values[1];
                assert(YAJL_IS_ARRAY(w1));
                assert_float(YAJL_GET_DOUBLE(YAJL_GET_ARRAY(w1)->values[0]), -122.25);
                assert_float(YAJL_GET_DOUBLE(YAJL_GET_ARRAY(w1)->values[1]), 47.75);
            }

            udp_socket_dealloc(wrapped);
            udp_socket_dealloc(raw);
            json_socket_dealloc(json_sock);
            state_dealloc(state);
        });

        IT_SHOULD("write selected output values to downstream monitor", {

            state *state = state_alloc(13);
            udp_socket *raw = udp_socket_alloc(&monitor_listen_ep, &xplane_broadcast_ep);
            udp_socket *wrapped = udp_socket_alloc(&xplane_broadcast_ep, &monitor_listen_ep);
            json_socket *json_sock = json_socket_alloc(wrapped);

            {
                state_set_float(state, "should_not_appear", 69);
                state_set_float(state, "a_float", 3.14);
                state_set_int(state, "an_int", 42);

                yajl_val waypoints_json = yajl_tree_parse("[[-122.2, 47.7], [-122.3, 47.8]]", NULL, 0);
                state_set_json(state, "some_waypoints", waypoints_json);


                yajl_val dict_json = yajl_tree_parse("{\"foo\":\"bar\", \"number\": 128, \"array\": [1,2]}", NULL, 0);
                state_set_json(state, "some_dict", dict_json);

                char *json = "{" \
                    "\"state.output.values\":[\"an_int\", \"a_float\", \"some_waypoints\", \"some_dict\"]"\
                    "}";

                udp_socket_write(raw, (unsigned char *)json, strlen(json));
                message_bus_read_json(json_sock, state);

                message_bus_write_values(json_sock, state);

                udp_socket_read(raw, test_udp_data_handler_function, &received_packet);

                char *expected = "{\"an_int\":42,\"a_float\":3.1400001049041748047,\"some_waypoints\":[[-122.20000000000000284,47.700000000000002842],[-122.29999999999999716,47.799999999999997158]],\"some_dict\":{\"foo\":\"bar\",\"number\":128,\"array\":[1,2]}}";
                int len = strlen(expected);
                assert(strncmp(received_packet->data, expected, len) == 0);
            }

            udp_socket_dealloc(wrapped);
            udp_socket_dealloc(raw);
            json_socket_dealloc(json_sock);
            state_dealloc(state);
        });
    }

    TEST_GROUP("hxstream")
    {
        IT_SHOULD("encode data", {
            void *sample_data = "hello world";
            size_t sample_data_length = 11;
            uint8_t tag = 0x7f;

            size_t output_length;
            const char *output = hx_stream_encode_frame(sample_data, sample_data_length, tag, &output_length);

            assert(output[0] == 0x7e);
            assert(output[1] == 0x7f);
            assert(strncmp(&output[2], "hello world", 11) == 0);
            assert(output_length == 13);
        });

        IT_SHOULD("parse data", {
            hx_stream_decoder_delegate *delegate = calloc(sizeof(hx_stream_decoder_delegate), 1);

            delegate->on_start = test_hx_stream_decoder_frame_start_callback;
            delegate->on_tag = test_hx_stream_decoder_frame_tag_callback;
            delegate->on_data = test_hx_stream_decoder_frame_data_callback;

            hx_stream_decoder *decoder = hx_stream_decoder_init(delegate);

            char *data_string = "  hello  world     ";

            int length = strlen(data_string);
            uint8_t *data = malloc(strlen(data_string));
            memcpy(data, data_string, length);

            uint8_t escape_octet = 0x7c;
            data[0] = 0x7e;
            data[1] = 0x42;
            data[7] = escape_octet;
            data[8] = 0x7e ^ 0x20; // escaped frame boundary char in data
            data[14] = 0x7e;
            data[15] = 0x7e;
            data[16] = 0x01;
            data[17] = escape_octet;
            data[18] = escape_octet ^ 0x20;

            hx_stream_callback_info callbackInfo[length];

            for (int i = 0; i < length; i++) {
                hx_stream_decoder_execute(decoder, &data[i], &callbackInfo[i]);
            }

            hx_stream_decoder_dealloc(decoder);

            assert(callbackInfo[0].type == hx_stream_callback_type_start);
            assert(callbackInfo[1].type == hx_stream_callback_type_tag);
            assert(callbackInfo[1].data == 0x42);
            assert(callbackInfo[2].type == hx_stream_callback_type_data);
            assert(callbackInfo[2].data == 'h');
            assert(callbackInfo[3].data == 'e');
            assert(callbackInfo[4].data == 'l');
            assert(callbackInfo[5].data == 'l');
            assert(callbackInfo[6].data == 'o');
            // index 7 contains the escape char and thus didn't generate a callback
            assert(callbackInfo[8].data == 0x7e);
            assert(callbackInfo[9].data == 'w');
            assert(callbackInfo[10].data == 'o');
            assert(callbackInfo[11].data == 'r');
            assert(callbackInfo[12].data == 'l');
            assert(callbackInfo[13].data == 'd');
            assert(callbackInfo[14].type == hx_stream_callback_type_start);
            assert(callbackInfo[15].type == hx_stream_callback_type_start);
            assert(callbackInfo[16].type == hx_stream_callback_type_tag);
            assert(callbackInfo[16].data == 0x01);
            // index 17 contains the escape char and thus didn't generate a callback
            assert(callbackInfo[18].data == escape_octet);

            free(data);
            free(delegate);
        });
    }

    // TEST_GROUP("core") 
    // {
    //     IT_SHOULD("call all callbacks", {

    //     });

    //     IT_SHOULD("return t1 > t0", {

    //     });

    //     IT_SHOULD("fly the airplane", {
            
    //     });
    // }
}

int main() {
    for (int i = 0; i < 100; i++) {
        printf("------------------\n");
        test();
    }
}