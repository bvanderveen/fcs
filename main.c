#include "xplane.h"
#include <stdio.h>



void on_data_message(xplane_message_data *messages, int count) {
    for (int i = 0; i < count; i++) {
        xplane_message_data *message = messages + i * sizeof(xplane_message_data);

        printf("on_data_message (index = %d) ", message->index);
        for (int j = 0; j < 8; j++) {
            printf("%f ", message->data[j]);
        }
        printf("\n");
    }
}

int main() {
    xplane_context xpctx;
    init_xplane_context(&xpctx, 49003, 49000);
    xpctx.data_handler = &on_data_message;

    float throttle = 0;

    while (1) {

        xplane_context_read(&xpctx);

        xplane_message_data data;
        data.index = xplane_data_throttle;
        data.data[0] = throttle;
        data.data[1] = throttle;
        data.data[2] = 0;
        data.data[3] = 0;
        data.data[4] = 0;
        data.data[5] = 0;
        data.data[6] = 0;
        data.data[7] = 0;

        xplane_write_data(&xpctx, &data, 1);

        throttle += .01;

        if (throttle > 1.0)
            throttle = 1.0;
    }

    return 0;
}