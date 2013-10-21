#include "fcs/state.h"
#include <assert.h>
#include <stdio.h>

#define TEST(T) {printf("test - " T "\n");}

int main() {
    // state
    {
    printf("testing state\n");

        state *state = NULL;
        TEST("state initializes")
        {
            state = state_alloc(4);

            assert(state != NULL);

            TEST("stores pi")
            {
                const char *key = "PI";

                state_set(state, key, 3.14159);
                float result = state_get(state, key);

                assert((result > 3.14158) && (result < 3.14160));
            }

            TEST("stores 42")
            {
                const char *key = "fourtytwo";

                state_set(state, key, 42);
                float result = state_get(state, key);

                assert(result == 42);
            }
        }

        // state dealloc 
        {
            state_dealloc(state);
        }

        printf("tests passed!\n");
    }
}