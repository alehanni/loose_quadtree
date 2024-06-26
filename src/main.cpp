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

struct ctx {
    quadtree qt;
    size_t icon_tex_handle;
    std::vector<rectangle> rects;
    std::vector<std::uint32_t> indices;
};

void draw_items(std::vector<rectangle> const& rects, size_t tex_handle) {
    auto r = entt::locator<renderer>::value();
    for (auto i=0U; i<rects.size(); i++)
        r->draw_sprite(rects[i].min, (vec2_t){(float)(i % 16) * 16.f, (float)(i / 16) * 16.f}, 16, 16, tex_handle);
}

void update_draw_frame(void *ctx_arg) {
    auto r = entt::locator<renderer>::value();

    static std::vector<std::uint32_t> query_results;
    static constexpr bbox cursor_bb = {
        -8.0f, -8.0f, 8.0f, 8.0f
    };
    
    //auto &r = ((ctx *)ctx_arg)->r;
    auto &qt = ((ctx *)ctx_arg)->qt;
    auto &icon_tex_handle = ((ctx *)ctx_arg)->icon_tex_handle;
    auto &rects = ((ctx *)ctx_arg)->rects;

    auto qt_artist = quadtree_artist(qt);

    // get query results
    auto mpos = r->get_mouse_position();

    bbox offset_bb = {
        (float)mpos.x + cursor_bb.minx,
        (float)mpos.y + cursor_bb.miny,
        (float)mpos.x + cursor_bb.maxx,
        (float)mpos.y + cursor_bb.maxy
    };
    query_results.clear();
    qt.query(offset_bb, query_results);

    r->start_drawing();
    r->clear_screen();

    // draw quadtree
    qt_artist.draw();
    qt_artist.draw_query(offset_bb);
    
    // draw item sprites
    draw_items(rects, icon_tex_handle);

    // draw query results
    for (auto i = 0U; i < query_results.size(); i++)
        r->draw_rectangle(
            (vec2_t){qt.pointboxes[query_results[i]].bb.minx, qt.pointboxes[query_results[i]].bb.miny},
            (vec2_t){qt.pointboxes[query_results[i]].bb.maxx, qt.pointboxes[query_results[i]].bb.maxy},
            (rgba_t){0, 255, 0, 255});

    // draw box at cursor
    r->draw_rectangle_fill(
        (vec2_t){mpos.x + cursor_bb.minx, mpos.y + cursor_bb.miny}, 
        (vec2_t){mpos.x + cursor_bb.maxx, mpos.y + cursor_bb.maxy},
        (rgba_t){0, 228, 48, 255});

    r->stop_drawing();
}

int main(int argc, char **argv) {

    renderer r{raylib_renderer{}};
    entt::locator<renderer>::emplace(r);

    constexpr int screen_width = 640;
    constexpr int screen_height = 480;

    ctx my_ctx;

    r->init_window(screen_width, screen_height, "quadtree");
    my_ctx.icon_tex_handle = r->texture_from_memory(__16x16icons_png, __16x16icons_png_len);

    // generate a bunch of boxes and build a quadtree
    auto e1 = std::default_random_engine(1);
    auto x_dist = std::uniform_int_distribution(80 + 12, 560 - 12);
    auto y_dist = std::uniform_int_distribution(60 + 12, 420 - 12);

    my_ctx.rects.reserve(140U);
    my_ctx.indices.reserve(140U);
    for (auto i = 0U; i < 140U; i++) {
        my_ctx.rects.emplace_back();
        my_ctx.rects[i].min = {(float)x_dist(e1), (float)y_dist(e1)};
        my_ctx.rects[i].max = {my_ctx.rects[i].min.x + 16.f, my_ctx.rects[i].min.y + 16.f};
        my_ctx.indices.push_back(i);
    }

    // create quadtree
    my_ctx.qt = build<rectangle>(my_ctx.rects, my_ctx.indices, [](rectangle &rect){
        bbox bb;
        bb.minx = rect.min.x;
        bb.miny = rect.min.y;
        bb.maxx = rect.max.x;
        bb.maxy = rect.max.y;
        return bb;
    });

#if defined(PLATFORM_WEB)
    emscripten_set_main_loop_arg(update_draw_frame, (void *)&my_ctx, 0, 1);
#else
    while(!r->window_should_close()) {
        update_draw_frame((void *)&my_ctx);
    }
#endif

    r->close_window();

    return 0;
}