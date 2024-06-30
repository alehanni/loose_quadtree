#ifndef RENDERER_H
#define RENDERER_H

#include <cstdint>

#include "entt/entt.hpp"
#include "utiltypes.h"

struct Renderer: entt::type_list<> {
    template<typename Base>
    struct type: Base {
        void start_drawing() {
            entt::poly_call<0u>(*this);
        }

        void stop_drawing() {
            entt::poly_call<1u>(*this);
        }

        void init_window(int width, int height, const char *title) {
            entt::poly_call<2u>(*this, width, height, title);
        }

        void close_window() {
            entt::poly_call<3u>(*this);
        }

        [[nodiscard]] bool window_should_close() {
            return entt::poly_call<4u>(*this);
        }

        void clear_screen() {
            entt::poly_call<5u>(*this);
        }

        [[nodiscard]] vec2_t get_mouse_position() {
            return entt::poly_call<6u>(*this);
        }

        [[nodiscard]] uint64_t texture_from_memory(unsigned char data[], unsigned int size) {
            return entt::poly_call<7u>(*this, data, size);
        }

        void draw_rectangle(vec2_t xmin, vec2_t xmax, rgba_t rgba) {
            entt::poly_call<8u>(*this, xmin, xmax, rgba);
        }

        void draw_rectangle_fill(vec2_t xmin, vec2_t xmax, rgba_t rgba) {
            entt::poly_call<9u>(*this, xmin, xmax, rgba);
        }
        
        void draw_sprite(vec2_t xy_dest, vec2_t xy_src, double w, double h, uint64_t id) {
            entt::poly_call<10u>(*this, xy_dest, xy_src, w, h, id);
        }
    };

    template<typename Type>
    using impl = entt::value_list<
        &Type::start_drawing,
        &Type::stop_drawing,
        &Type::init_window,
        &Type::close_window,
        &Type::window_should_close,
        &Type::clear_screen,
        &Type::get_mouse_position,
        &Type::texture_from_memory,
        &Type::draw_rectangle,
        &Type::draw_rectangle_fill,
        &Type::draw_sprite>;
};

struct noop_renderer {
    void start_drawing() { }
    void stop_drawing() { }
    void init_window(int, int, const char *) { }
    void close_window() { }
    bool window_should_close() { return false; }
    void clear_screen() { }
    vec2_t get_mouse_position() { return vec2_t{}; }
    uint64_t texture_from_memory(unsigned char [], unsigned int) { return 0u; }
    void draw_rectangle(vec2_t, vec2_t, rgba_t) { }
    void draw_rectangle_fill(vec2_t, vec2_t, rgba_t) { }
    void draw_sprite(vec2_t, vec2_t, double, double, uint64_t) { }
};

using renderer = entt::poly<Renderer>;

#endif