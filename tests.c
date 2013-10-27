#include "fcs/state.h"
#include "fcs/udp.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <arpa/inet.h>

#define TEST_GROUP(T) {printf("--- " T "\n");}
#define IT_SHOULD(WHAT, HOW) { \
printf("it should " WHAT "..."); \
HOW; \
printf("OK\n"); \
}

void udp_data_handler_function(udp_packet *p, void *context) {
    udp_packet **packet = context;
    *packet = udp_packet_copy(p);
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

        IT_SHOULD("round trip data", {
            udp_endpoint socket_a_listen;
            udp_endpoint_init(&socket_a_listen, INADDR_ANY, 49001);
            udp_endpoint socket_a_broadcast;
            udp_endpoint_init(&socket_a_broadcast, INADDR_ANY, 49002);
            udp_endpoint socket_b_listen;
            udp_endpoint_init(&socket_b_listen, INADDR_ANY, 49002);
            udp_endpoint socket_b_broadcast;
            udp_endpoint_init(&socket_b_broadcast, INADDR_ANY, 49001);

            udp_packet *received_packet;
            udp_socket *socket_a = udp_socket_alloc(&socket_a_listen, &socket_a_broadcast, &udp_data_handler_function, &received_packet);
            udp_socket *socket_b = udp_socket_alloc(&socket_b_listen, &socket_b_broadcast, NULL, NULL);

            char *packet_data = "some data";
            udp_socket_write(socket_b, packet_data, 9);
            udp_socket_read(socket_a);

            assert(udp_endpoint_port(received_packet->ep) == 49002);
            assert(strncmp(received_packet->data, packet_data, 9) == 0);

            udp_socket_dealloc(socket_a);
            udp_socket_dealloc(socket_b);
        });
    }
}