#include "fcs/state.h"
#include "fcs/udp.h"
#include "fcs/net/json_socket.h"
#include "fcs/net/xplane_socket.h"
#include "fcs/net/xplane_bus.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <arpa/inet.h>
#include <stdlib.h>

#define TEST_GROUP(T) {printf("--- " T "\n");}
#define IT_SHOULD(WHAT, HOW) { \
printf("it should " WHAT "..."); \
HOW; \
printf("OK\n"); \
}

#define assert_float(ACTUAL, EXPECTED) assert(((ACTUAL) > ((EXPECTED) - .01)) && ((ACTUAL) < ((EXPECTED) + .01)))

void test_udp_data_handler_function(udp_packet *p, void *context) {
    udp_packet **packet = context;
    *packet = udp_packet_copy(p);
}

void test_xplane_data_handler_function(xplane_message_data *ms, int count, void *context) {
    xplane_message_data *messages = context;

    memcpy(&messages[0], &ms[0], sizeof(xplane_message_data));
    memcpy(&messages[1], &ms[1], sizeof(xplane_message_data));
}

void test_json_handler_function(yajl_val j, void *context) {
    yajl_val *json = context;
    *json = j;
}

int main() {

    udp_endpoint xplane_listen_ep;
    udp_endpoint_init(&xplane_listen_ep, INADDR_ANY, 49000);

    udp_endpoint xplane_broadcast_ep;
    udp_endpoint_init(&xplane_broadcast_ep, INADDR_ANY, 49001);

    udp_endpoint any_ep;
    udp_endpoint_init(&any_ep, INADDR_ANY, 0);

    
    // TEST_GROUP("state")
    // {
    //     state *state = NULL;
    //     IT_SHOULD("initialize state",
    //     {
    //         state = state_alloc(4);

    //         assert(state != NULL);
    //     });

    //     IT_SHOULD("store pi", {
    //         const char *key = "PI";

    //         state_set(state, key, 3.14159);
    //         float result = state_get(state, key);

    //         assert((result > 3.14158) && (result < 3.14160));
    //     });

    //     IT_SHOULD("store 42", {
    //         const char *key = "fourtytwo";

    //         state_set(state, key, 42);
    //         float result = state_get(state, key);

    //         assert(result == 42);
    //     });

    //     IT_SHOULD("store json", {
    //         const char *key = "jaysahn";

    //         char *json = "{\"data\":123}";            

    //         yajl_val v = yajl_tree_parse(json, NULL, 0);

    //         state_set_json(state, key, v);
    //         yajl_val result = state_get_json(state, key);

    //         assert(YAJL_IS_OBJECT(result));
    //         assert(strncmp(YAJL_GET_OBJECT(result)->keys[0], "data", 4) == 0);
    //         assert(YAJL_GET_INTEGER(YAJL_GET_OBJECT(result)->values[0]) == 123);
    //     });

    //     state_dealloc(state);
    // }

    udp_packet *received_packet;

    // TEST_GROUP("udp")
    // {
    //     udp_endpoint ep;
    //     udp_endpoint_init(&ep, ipv4_address_from_string("192.168.1.1"), 90);

    //     IT_SHOULD("get address", {
    //         uint32_t addr = udp_endpoint_address(&ep);
    //         assert(addr == 0xc0a80101);
    //     });

    //     IT_SHOULD("get port", {
    //         int port = udp_endpoint_port(&ep);
    //         assert(port == 90);
    //     });

    //     udp_endpoint destination_ep;
    //     udp_endpoint_init(&destination_ep, INADDR_ANY, 49001);
    //     udp_endpoint source_ep;
    //     udp_endpoint_init(&source_ep, INADDR_ANY, 49002);

    //     IT_SHOULD("round trip data", {
    //         udp_socket *listener = udp_socket_alloc(&destination_ep, &source_ep, test_udp_data_handler_function, &received_packet);
    //         udp_socket *broadcaster = udp_socket_alloc(&source_ep, &destination_ep, NULL, NULL);

    //         char *packet_data = "some data";
    //         udp_socket_write(broadcaster, packet_data, 9);
    //         udp_socket_read(listener);

    //         assert(udp_endpoint_port(received_packet->ep) == 49002);
    //         assert(strncmp(received_packet->data, packet_data, 9) == 0);

    //         udp_socket_dealloc(listener);
    //         udp_socket_dealloc(broadcaster);
    //     });
    // }

    xplane_message_data *received_messages = NULL;

    TEST_GROUP("xplane")
    {
        xplane_message_data fake_xplane_data[2] = {
            { .index = 1, .data = { .1, .2, .3, .4, .5, .6, .7, .8 } },
            { .index = 2, .data = { .8, .7, .6, .5, .4, .3, .2, .1 } }
        };

        IT_SHOULD("send data to xplane", {
            udp_socket *raw = udp_socket_alloc(&xplane_broadcast_ep, &any_ep);
            raw->context = &received_packet;
            udp_socket *wrapped = udp_socket_alloc(&any_ep, &xplane_broadcast_ep);
            xplane_socket *xplane_sock = xplane_socket_alloc(wrapped);
            xplane_sock->context = &received_messages;

            {
                xplane_socket_write(xplane_sock, fake_xplane_data, 2);

                udp_socket_read(raw, &test_udp_data_handler_function);

                assert(strncmp(received_packet->data, "DATA\0", 5) == 0);

                xplane_message_data *data0 = (xplane_message_data *)(received_packet->data + 5);
                xplane_message_data *data1 = (xplane_message_data *)(received_packet->data + 5 + sizeof(xplane_message_data));
                
                assert(data0->index == 1);
                assert(data1->index == 2);
            }

            udp_socket_dealloc(wrapped);
            udp_socket_dealloc(raw);
            xplane_socket_dealloc(xplane_sock);
        });

        IT_SHOULD("receive data from xplane", {
            printf("1\n");
            received_messages = malloc(sizeof(xplane_message_data) * 2);
            printf("2\n");
            udp_socket *raw = udp_socket_alloc(&any_ep, &xplane_broadcast_ep);
            printf("3\n");
            udp_socket *wrapped = udp_socket_alloc(&xplane_broadcast_ep, &any_ep);
            printf("4\n");
            xplane_socket *xplane_sock = xplane_socket_alloc(wrapped);
            xplane_sock->context = received_messages;
            printf("5\n");
            char *xplane_data_bytes = malloc(5 + sizeof(xplane_message_data));
            printf("6\n");

            {
                char *p = xplane_data_bytes;

                memcpy(p, "DATA\0", 5);
                p += 5;
                printf("6-\n");

                memcpy(p, &fake_xplane_data[0], sizeof(xplane_message_data));
                p += sizeof(xplane_message_data);
                printf("6--\n");

                memcpy(p, &fake_xplane_data[1], sizeof(xplane_message_data));
                printf("6---\n");

                udp_socket_write(raw, xplane_data_bytes, (sizeof(xplane_message_data) * 2 + 5));
                printf("6----\n");

                xplane_socket_read(xplane_sock, &test_xplane_data_handler_function);
                printf("6-----\n");

                assert(received_messages[0].index == 1);
                assert_float(received_messages[0].data[0], .1);
                assert_float(received_messages[0].data[1], .2);
                assert_float(received_messages[0].data[2], .3);
                assert_float(received_messages[0].data[3], .4);
                assert_float(received_messages[0].data[4], .5);
                assert_float(received_messages[0].data[5], .6);
                assert_float(received_messages[0].data[6], .7);
                assert_float(received_messages[0].data[7], .8);

                assert(received_messages[1].index == 2);
                assert_float(received_messages[1].data[7], .1);
                assert_float(received_messages[1].data[6], .2);
                assert_float(received_messages[1].data[5], .3);
                assert_float(received_messages[1].data[4], .4);
                assert_float(received_messages[1].data[3], .5);
                assert_float(received_messages[1].data[2], .6);
                assert_float(received_messages[1].data[1], .7);
                assert_float(received_messages[1].data[0], .8);
            }

            printf("7\n");
            udp_socket_dealloc(raw);
            printf("8\n");
            udp_socket_dealloc(wrapped);
            printf("9\n");
            xplane_socket_dealloc(xplane_sock);
            printf("10\n");
            free(received_messages);
            printf("11\n");
            free(xplane_data_bytes);
            printf("12\n");
        });
    }

    TEST_GROUP("messages") 
    {
        xplane_message_data mock_xplane_data[3] = {
            { .index = 18, .data = { .9, .8, .7, 0, 0, 0, 0, 0 } },
            { .index = 20, .data = { 42, -122, 3000, 0, 0, 0, 0, 0 } },
            { .index = 4, .data = { 0, 0, 0, 0, .5, -1, .1, 0 } }
        };

        // IT_SHOULD("read sensors from xplane to state", {
        //     printf("1\n");
        //     state *state = state_alloc(13);
        //     printf("2\n");
        //     udp_socket *raw = udp_socket_alloc(&any_ep, &xplane_broadcast_ep, NULL, NULL);
        //     printf("3\n");
        //     udp_socket *wrapped = udp_socket_alloc(&xplane_broadcast_ep, &any_ep, NULL, NULL);
        //     printf("4\n");
        //     xplane_socket *xplane_sock = xplane_socket_alloc(wrapped, NULL, NULL);
        //     printf("5\n");
        //     char *xplane_data_bytes = malloc(5 + sizeof(xplane_message_data) * 3);
        //     printf("6\n");

        //     {
        //         char *p = xplane_data_bytes;

        //         memcpy(p, "DATA\0", 5);
        //         p += 5;

        //         memcpy(p, &mock_xplane_data[0], sizeof(xplane_message_data));
        //         p += sizeof(xplane_message_data);

        //         memcpy(p, &mock_xplane_data[1], sizeof(xplane_message_data));
        //         p += sizeof(xplane_message_data);

        //         memcpy(p, &mock_xplane_data[2], sizeof(xplane_message_data));

        //         printf("1\n");
        //         udp_socket_write(raw, xplane_data_bytes, sizeof(xplane_message_data) * 3 + 5);
        //         printf("2\n");

        //         xplane_bus_read_sensors(xplane_sock, state);
        //         printf("3\n");

        //         assert_float(state_get(state, STATE_SENSOR_PITCH), .9);
        //         assert_float(state_get(state, STATE_SENSOR_ROLL), .8);
        //         assert_float(state_get(state, STATE_SENSOR_HEADING), .7);

        //         assert_float(state_get(state, STATE_SENSOR_LATITUDE), 42);
        //         assert_float(state_get(state, STATE_SENSOR_LONGITUDE), -122);
        //         assert_float(state_get(state, STATE_SENSOR_ALTITUDE), 3000);

        //         assert_float(state_get(state, STATE_SENSOR_ACCEL_T), .5);
        //         assert_float(state_get(state, STATE_SENSOR_ACCEL_N), -1);
        //         assert_float(state_get(state, STATE_SENSOR_ACCEL_B), .1);
        //     }

        //     printf("7\n");
        //     udp_socket_dealloc(wrapped);
        //     printf("8\n");
        //     udp_socket_dealloc(raw);
        //     printf("9\n");
        //     xplane_socket_dealloc(xplane_sock);
        //     printf("10\n");
        //     state_dealloc(state);
        //     printf("11\n");
        //     free(xplane_data_bytes);
        //     printf("12\n");
        // });

        // IT_SHOULD("write effectors from state to xplane", {
        //     printf("1\n");
        //     state *state = state_alloc(13);
        //     printf("2\n");
        //     udp_socket *raw = udp_socket_alloc(&xplane_broadcast_ep, &any_ep, test_udp_data_handler_function, &received_packet);
        //     printf("3\n");
        //     udp_socket *wrapped = udp_socket_alloc(&any_ep, &xplane_broadcast_ep, NULL, NULL);
        //     printf("4\n");
        //     xplane_socket *xplane_sock = xplane_socket_alloc(wrapped, NULL, NULL);

        //     {
        //         state_set(state, STATE_EFFECTOR_AILERON, .4);
        //         state_set(state, STATE_EFFECTOR_ELEVATOR, .5);
        //         state_set(state, STATE_EFFECTOR_RUDDER, .6);
        //         state_set(state, STATE_EFFECTOR_THROTTLE, .7);

        //         xplane_bus_write_effectors(xplane_sock, state);

        //         udp_socket_read(raw);

        //         assert(strncmp(received_packet->data, "DATA\0", 5) == 0);

        //         xplane_message_data *messages = (xplane_message_data *)(received_packet->data + 5);
                
        //         assert(messages[0].index == 8);
        //         assert_float(messages[0].data[0], .5);
        //         assert_float(messages[0].data[1], .4);
        //         assert_float(messages[0].data[2], .6);
        //         assert_float(messages[0].data[3], .0);
        //         assert_float(messages[0].data[4], .0);
        //         assert_float(messages[0].data[5], .0);
        //         assert_float(messages[0].data[6], .0);
        //         assert_float(messages[0].data[7], .0);

        //         assert(messages[1].index == 25);
        //         assert_float(messages[1].data[0], .7);
        //         assert_float(messages[1].data[1], .7);
        //         assert_float(messages[1].data[2], .7);
        //         assert_float(messages[1].data[3], .7);
        //         assert_float(messages[1].data[4], .0);
        //         assert_float(messages[1].data[5], .0);
        //         assert_float(messages[1].data[6], .0);
        //         assert_float(messages[1].data[7], .0);
        //     }

        //     udp_socket_dealloc(wrapped);
        //     udp_socket_dealloc(raw);
        //     xplane_socket_dealloc(xplane_sock);
        //     state_dealloc(state);
        // });

        IT_SHOULD("read json messages from upstream controller", {
            printf("1\n");
            state *state = state_alloc(13);
            printf("2\n");
            udp_socket *raw = udp_socket_alloc(&any_ep, &xplane_broadcast_ep);
            printf("3\n");
            udp_socket *wrapped = udp_socket_alloc(&xplane_broadcast_ep, &any_ep);
            printf("4\n");
            json_socket *json_sock = json_socket_alloc(wrapped);
            printf("5\n");
            char *xplane_data_bytes = malloc(5 + sizeof(xplane_message_data) * 3);
            printf("6\n");

            {
                char *json = "{\"name\":\"state.butts.ability\", \"value\":1024}";

                printf("6-\n");
                udp_socket_write(raw, json, sizeof(json));
                printf("6--\n");

                message_bus_read_json(json_sock, state);
                printf("6---\n");

                assert_float(state_get(state, "state.butts.ability"), 1024);
                printf("6----\n");
            }

            printf("7\n");
            udp_socket_dealloc(wrapped);
            printf("8\n");
            udp_socket_dealloc(raw);
            printf("9\n");
            json_socket_dealloc(json_sock);
            printf("10\n");
            state_dealloc(state);
            printf("11\n");
            free(xplane_data_bytes);
            printf("12\n");

        });
    }
}