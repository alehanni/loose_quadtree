#ifndef UTILTYPES_H
#define UTILTYPES_H

#include <cstdint>
#include <variant>

using num64_t = std::variant<double, int64_t, uint64_t>;

struct vec2_64_t {
    num64_t x;
    num64_t y;
};

struct rgba_t {
    uint8_t r;
    uint8_t g;
    uint8_t b;
    uint8_t a;
};

struct vec2_t {
    double x, y;
};

struct rectangle {
    vec2_t min;
    vec2_t max;
};

#endif