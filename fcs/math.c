#import "math.h"
#include <math.h>
#include <stdio.h>

void inverse(matrix2x2 *m, matrix2x2 *result) {
    float det = 1 / (m->m0 * m->m3 - m->m1 * m->m2);

    result->m0 = det * m->m3;
    result->m1 = det * -1 * m->m1;
    result->m2 = det * -1 * m->m2;
    result->m3 = det * m->m0;
}

void matrix_mult(matrix2x2 *m, geopoint *v, geopoint *result) {
    result->lat = m->m0 * v->lat + m->m1 * v->lon;
    result->lon = m->m2 * v->lat + m->m3 * v->lon;
}

void geopoint_print(geopoint *p) {
    printf("(%f, %f)\n", p->lat, p->lon);
}

void difference(geopoint *p0, geopoint *p1, geopoint *result) {
    result->lat = p0->lat - p1->lat;
    result->lon = p0->lon - p1->lon;
}

float dot(geopoint *p0, geopoint *p1) {
    return p0->lat * p1->lat + p0->lon * p1->lon;
}

void scalar_mult(geopoint *p0, float k, geopoint *result) {
    result->lat = k * p0->lat;
    result->lon = k * p0->lon;
}

void projection(geopoint *u, geopoint *v, geopoint *result) {
    // projection := u, v { (v dot u) / (u dot u) * u }
    scalar_mult(u, dot(v, u) / dot(u, u), result);
}

float magnitude_squared(geopoint *p) {
    return p->lat * p->lat + p->lon * p->lon;
}

float distance_squared(geopoint *p0, geopoint *p1) {
    geopoint diff;
    difference(p0, p1, &diff);
    return magnitude_squared(&diff);
}

float normalize(geopoint *p0, geopoint *result) {
    float mag = 1 / sqrt(magnitude_squared(p0));
    scalar_mult(p0, mag, result);
}