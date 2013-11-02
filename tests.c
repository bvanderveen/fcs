#include "fcs/state.h"
#include "fcs/udp.h"
#include "xplane/xplane.h"
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

int main() {
    
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

            state_set(state, key, 3.14159);
            float result = state_get(state, key);

            assert((result > 3.14158) && (result < 3.14160));
        });

        IT_SHOULD("store 42", {
            const char *key = "fourtytwo";

            state_set(state, key, 42);
            float result = state_get(state, key);

            assert(result == 42);
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
            udp_socket *listener = udp_socket_alloc(&destination_ep, &source_ep, test_udp_data_handler_function, &received_packet);
            udp_socket *broadcaster = udp_socket_alloc(&source_ep, &destination_ep, NULL, NULL);

            char *packet_data = "some data";
            udp_socket_write(broadcaster, packet_data, 9);
            udp_socket_read(listener);

            assert(udp_endpoint_port(received_packet->ep) == 49002);
            assert(strncmp(received_packet->data, packet_data, 9) == 0);

            udp_socket_dealloc(listener);
            udp_socket_dealloc(broadcaster);
        });
    }

    TEST_GROUP("xplane")
    {
        udp_endpoint xplane_listen_ep;
        udp_endpoint_init(&xplane_listen_ep, INADDR_ANY, 49000);

        udp_endpoint xplane_broadcast_ep;
        udp_endpoint_init(&xplane_broadcast_ep, INADDR_ANY, 49001);

        udp_endpoint any_ep;
        udp_endpoint_init(&any_ep, INADDR_ANY, 0);

        xplane_message_data *received_messages = malloc(sizeof(xplane_message_data) * 2);

        xplane_message_data fake_xplane_data[2] = {
            { .index = 1, .data = { .1, .2, .3, .4, .5, .6, .7, .8 } },
            { .index = 2, .data = { .8, .7, .6, .5, .4, .3, .2, .1 } }
        };

        IT_SHOULD("send data to xplane", {
            udp_socket *raw = udp_socket_alloc(&xplane_broadcast_ep, &any_ep, test_udp_data_handler_function, &received_packet);
            udp_socket *wrapped = udp_socket_alloc(&any_ep, &xplane_broadcast_ep, NULL, NULL);
            xplane_socket *xplane_sock = xplane_socket_alloc(wrapped, NULL, &received_messages);

            {

                xplane_socket_write(xplane_sock, fake_xplane_data, 2);

                udp_socket_read(raw);

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
            udp_socket *raw = udp_socket_alloc(&any_ep, &xplane_broadcast_ep, &test_udp_data_handler_function, &received_packet);
            udp_socket *wrapped = udp_socket_alloc(&xplane_broadcast_ep, &any_ep, NULL, NULL);
            xplane_socket *xplane_sock = xplane_socket_alloc(wrapped, &test_xplane_data_handler_function, received_messages);

            {

                char *xplane_data_bytes = malloc(5 + sizeof(xplane_message_data));
                char *p = xplane_data_bytes;

                memcpy(p, "DATA\0", 5);
                p += 5;

                memcpy(p, &fake_xplane_data[0], sizeof(xplane_message_data));
                p += sizeof(xplane_message_data);

                memcpy(p, &fake_xplane_data[1], sizeof(xplane_message_data));

                udp_socket_write(raw, xplane_data_bytes, (sizeof(xplane_message_data) * 2 + 5));
                xplane_socket_read(xplane_sock);

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

            udp_socket_dealloc(raw);
            udp_socket_dealloc(wrapped);

            xplane_socket_dealloc(xplane_sock);
        });
    }
}