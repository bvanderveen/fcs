
#ifndef __FCS_MATH
#define __FCS_MATH

#define TO_RADIANS(X) ((X) * M_PI / 180)
#define TO_DEGREES(X) ((X) * 180 / M_PI)

struct geopoint {
    float
    lat,
    lon;
};
typedef struct geopoint geopoint;

struct matrix2x2 {
    float m0, m1,
    m2, m3;
};
typedef struct matrix2x2 matrix2x2;

struct vector2 {
    float v0, v1;
};
typedef struct vector2 vector2;

struct vector3 {
    float v0, v1, v2;
};
typedef struct vector3 vector3;

struct matrix3x3 {
    float m0, m1, m2,
    m3, m4, m5,
    m6, m7, m8;
};
typedef struct matrix3x3 matrix3x3;


void inverse(matrix2x2 *m, matrix2x2 *result);
void matrix_mult(matrix2x2 *m, geopoint *v, geopoint *result);
void geopoint_print(geopoint *p);
void difference(geopoint *p0, geopoint *p1, geopoint *result);
float dot(geopoint *p0, geopoint *p1);
void scalar_mult(geopoint *p0, float k, geopoint *result);
void projection(geopoint *u, geopoint *v, geopoint *result);
float magnitude_squared(geopoint *p);
float distance_squared(geopoint *p0, geopoint *p1);
float normalize(geopoint *p0, geopoint *result);

#endif