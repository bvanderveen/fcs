#include "core.h"
#include <stdio.h>

void core_sensor_state_print(core_sensor_state *state) {
    printf("lat %04f lon %04f alt %04f pitch %04f roll %04f heading %04f aT %04f aN %04f aB %04f\n", 
        state->lat,
        state->lon, 
        state->alt, 
        state->pitch, 
        state->roll, 
        state->heading,
        state->aT, 
        state->aN, 
        state->aB);
}
