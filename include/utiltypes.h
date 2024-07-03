#ifndef UTILTYPES_H
#define UTILTYPES_H

#include <cstdint>
#include <limits>

static constexpr float inf = std::numeric_limits<float>::infinity();

struct rgba_t {
    uint8_t r;
    uint8_t g;
    uint8_t b;
    uint8_t a;
};

struct vec2_t {
    double x, y;

    vec2_t &operator +=(vec2_t rhs) { x += rhs.x; y += rhs.y; return *this; }
    vec2_t &operator -=(vec2_t rhs) { x -= rhs.x; y -= rhs.y; return *this; }
    vec2_t &operator *=(double rhs) { x *= rhs; y *= rhs; return *this; }
    vec2_t &operator /=(double rhs) { x /= rhs; y /= rhs; return *this; }
    friend vec2_t operator+(vec2_t lhs, vec2_t const& rhs) { lhs += rhs; return lhs; }
    friend vec2_t operator-(vec2_t lhs, vec2_t const& rhs) { lhs -= rhs; return lhs; }
    friend vec2_t operator *(vec2_t lhs, double rhs) { lhs *= rhs; return lhs; }
    friend vec2_t operator /(vec2_t lhs, double rhs) { lhs /= rhs; return lhs; }
};

struct box_t {
    vec2_t min;
    vec2_t max;

    static bool intersect(box_t const& a, box_t const& b) { return a.intersect(b); }
    bool intersect(auto const& other) const {
        return (max.x >= other.min.x && min.x <= other.max.x
            && max.y >= other.min.y && min.y <= other.max.y);
    }
};

#endif