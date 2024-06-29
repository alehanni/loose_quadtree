#ifndef UTILTYPES_H
#define UTILTYPES_H

#include <cstdint>

struct vec2 {
    float x;
    float y;
};

struct rectangle {
    vec2 min;
    vec2 max;
};

struct color {
    uint8_t r;
    uint8_t g;
    uint8_t b;
    uint8_t a;
};

#endif