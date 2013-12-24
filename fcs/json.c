#include "json.h"

void json_write_value(yajl_gen g, yajl_val value) {
    if (YAJL_IS_STRING(value)) {
        char *s = YAJL_GET_STRING(value);
        // grrrrr schlemiel
        yajl_gen_string(g, s, strlen(s));
    }
    else if (YAJL_IS_INTEGER(value)) {
        yajl_gen_integer(g, YAJL_GET_INTEGER(value));
    }
    else if (YAJL_IS_DOUBLE(value)) {
        yajl_gen_double(g, YAJL_GET_DOUBLE(value));
    }
    else if (YAJL_IS_TRUE(value)) {
        yajl_gen_bool(g, 1);
    }
    else if (YAJL_IS_FALSE(value)) {
        yajl_gen_bool(g, 0);
    }
    else if (YAJL_IS_NULL(value)) {
        yajl_gen_null(g);
    }
    else if (YAJL_IS_OBJECT(value)) {
        yajl_gen_map_open(g);

        int len = YAJL_GET_OBJECT(value)->len;
        for (int i = 0; i < len; i++) {
            const char *k = YAJL_GET_OBJECT(value)->keys[i];
            yajl_gen_string(g, k, strlen(k));
            json_write_value(g, YAJL_GET_OBJECT(value)->values[i]);
        }

        yajl_gen_map_close(g);
    }
    else if (YAJL_IS_ARRAY(value)) {
        yajl_gen_array_open(g);

        int len = YAJL_GET_ARRAY(value)->len;
        for (int i = 0; i < len; i++) {
            json_write_value(g, YAJL_GET_OBJECT(value)->values[i]);
        }

        yajl_gen_array_close(g);
    }
}