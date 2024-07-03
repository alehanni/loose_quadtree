#if defined(PLATFORM_WEB)
    #include <emscripten/emscripten.h>
#endif

#include <cassert>
#include <vector>
#include <random>
#include <cstdint>

#include "resources/16x16icons.h"
#include "raylib_renderer.h"
#include "quadtree.h"
#include "quadtree_artist.h"
#include "utiltypes.h"

#include "entt/entt.hpp"
#include "renderer.h"

using namespace entt::literals;

void draw_items(std::vector<box_t> const& rects, size_t tex_handle) {
    auto r = entt::locator<renderer>::value();
    for (auto i=0U; i<rects.size(); i++)
        r->draw_sprite(rects[i].min, (vec2_t){(float)(i % 16) * 16.f, (float)(i / 16) * 16.f}, 16, 16, tex_handle);
}

void update_draw_frame() {
    auto r = entt::locator<renderer>::value();

    static std::vector<std::uint32_t> query_results;
    static constexpr box_t cursor_bb = {{-8.0, -8.0}, {8.0, 8.0}};
    
    quadtree &&qt = entt::monostate<"quadtree"_hs>{};
    std::vector<box_t> &&rects = entt::monostate<"rect_vector"_hs>{};

    auto qt_artist = quadtree_artist(qt);

    // get query results
    auto mpos = r->get_mouse_position();

    box_t offset_bb = {
        (float)mpos.x + cursor_bb.min.x,
        (float)mpos.y + cursor_bb.min.y,
        (float)mpos.x + cursor_bb.max.x,
        (float)mpos.y + cursor_bb.max.y
    };
    query_results.clear();
    qt.query(offset_bb, query_results);

    r->start_drawing();
    r->clear_screen();

    // draw quadtree
    qt_artist.draw();
    qt_artist.draw_query(offset_bb);
    
    // draw item sprites
    size_t tex_handle = entt::monostate<"16x16icons_tex"_hs>{};
    draw_items(rects, tex_handle);

    // draw query results
    for (auto i = 0U; i < query_results.size(); i++)
        r->draw_rectangle(
            (vec2_t){qt.pointboxes[query_results[i]].min.x, qt.pointboxes[query_results[i]].min.y},
            (vec2_t){qt.pointboxes[query_results[i]].max.x, qt.pointboxes[query_results[i]].max.y},
            (rgba_t){0, 255, 0, 255});

    // draw box at cursor
    r->draw_rectangle_fill(
        (vec2_t){mpos.x + cursor_bb.min.x, mpos.y + cursor_bb.min.y}, 
        (vec2_t){mpos.x + cursor_bb.max.x, mpos.y + cursor_bb.max.y},
        (rgba_t){0, 228, 48, 255});

    r->stop_drawing();
}

int main(int argc, char **argv) {

    renderer r{raylib_renderer{}};
    entt::locator<renderer>::emplace(r);

    constexpr int screen_width = 640;
    constexpr int screen_height = 480;

    r->init_window(screen_width, screen_height, "quadtree");
    entt::monostate<"16x16icons_tex"_hs>{} = r->texture_from_memory(__16x16icons_png, __16x16icons_png_len);

    // generate a bunch of boxes and build a quadtree
    auto e1 = std::default_random_engine(1);
    auto x_dist = std::uniform_int_distribution(80 + 12, 560 - 12);
    auto y_dist = std::uniform_int_distribution(60 + 12, 420 - 12);

    std::vector<box_t> rects;
    std::vector<std::uint32_t> indices;
    rects.reserve(140u);
    indices.reserve(140u);
    for (auto i = 0U; i < 140u; i++) {
        rects.emplace_back();
        rects[i].min = {(float)x_dist(e1), (float)y_dist(e1)};
        rects[i].max = {rects[i].min.x + 16.f, rects[i].min.y + 16.f};
        indices.push_back(i);
    }

    entt::monostate<"rect_vector"_hs>{} = (std::vector<box_t> &)rects;

    // create quadtree
    quadtree &&qt = build<box_t>(rects, indices, [](box_t &bb){ return bb; });
    entt::monostate<"quadtree"_hs>{} = qt;

#if defined(PLATFORM_WEB)
    emscripten_set_main_loop(update_draw_frame, 0, 1);
#else
    while(!r->window_should_close()) {
        update_draw_frame();
    }
#endif

    r->close_window();

    return 0;
}