#if defined(PLATFORM_WEB)
    #include <emscripten/emscripten.h>
#endif

#include <cassert>
#include <cstdint>
#include <cmath>
#include <vector>

#include "raylib.h"
#include "resources/16x16icons.h"
#include "rand.hpp"
#include "loose_quadtree_artist.hpp"
#include "loose_quadtree.hpp"

Texture2D g_16x16icons_tex;
alh::loose_quadtree_t<alh::loose_quadtree::aabb_t> g_qt;
std::vector<alh::loose_quadtree::aabb_t> g_rects;

void draw_items(std::vector<alh::loose_quadtree::aabb_t> const& rects) {
    for (uint64_t i=0; i<rects.size(); i++) {
        Vector2 xy_dest = {rects[i].min.x, rects[i].min.y};
        Vector2 xy_src = {(float)(i % 16) * 16.f, (float)(i / 16) * 16.f};
        DrawTextureRec(
            g_16x16icons_tex,
            {(float)xy_src.x, (float)xy_src.y, 16.f, 16.f},
            {(float)xy_dest.x, (float)xy_dest.y},
            WHITE
        );
    }
}

void update_draw_frame() {
    static constexpr alh::loose_quadtree::aabb_t cursor_bb = {{-8.0, -8.0}, {8.0, 8.0}};

    auto qt_artist = alh::loose_quadtree_artist_t(g_qt);

    // get query results
    auto mpos = GetMousePosition();

    alh::loose_quadtree::aabb_t offset_bb = {
        {(float)mpos.x + cursor_bb.min.x, (float)mpos.y + cursor_bb.min.y},
        {(float)mpos.x + cursor_bb.max.x, (float)mpos.y + cursor_bb.max.y}
    };

    BeginDrawing();
    ClearBackground(DARKGRAY);

    // draw quadtree
    qt_artist.draw();
    qt_artist.draw_query(offset_bb);
    
    // draw item sprites
    draw_items(g_rects);

    // draw query results
    for (auto it = g_qt.query_start(offset_bb); it != g_qt.query_end(); ++it) {
        auto bb = *it;
        DrawRectangleLines(bb.min.x,
                           bb.min.y,
                           bb.max.x - bb.min.x,
                           bb.max.y - bb.min.y,
                           {0, 255, 0, 255});
    }

    // draw box at cursor
    {
        Vector2 min = {mpos.x + cursor_bb.min.x, mpos.y + cursor_bb.min.y};
        Vector2 max = {mpos.x + cursor_bb.max.x, mpos.y + cursor_bb.max.y};
        DrawRectangle(min.x,
                      min.y,
                      max.x - min.x,
                      max.y - min.y,
                      {0, 228, 48, 255});
    }

    EndDrawing();
}

int main(int argc, char **argv) {
    constexpr int screen_width = 640;
    constexpr int screen_height = 480;

    InitWindow(screen_width, screen_height, "loose_quadtree");
#if !defined(PLATFORM_WEB)
    SetTargetFPS(60);
#endif

    auto img = LoadImageFromMemory(".png", __16x16icons_png, __16x16icons_png_len);
    g_16x16icons_tex = LoadTextureFromImage(img);

    // generate a bunch of boxes and build a quadtree
    auto rng = alh::rand_f32();
    rng.seed(1);

    std::vector<std::uint32_t> indices;
    g_rects.reserve(140u);
    indices.reserve(140u);
    for (auto i = 0u; i < 140u; i++) {
        g_rects.emplace_back();
        g_rects[i].min = {floorf(rng.get_uniform(80+12, 560-12)), floorf(rng.get_uniform(60+12, 420-12))};
        g_rects[i].max = {g_rects[i].min.x + 16.f, g_rects[i].min.y + 16.f};
        indices.push_back(i);
    }

    // create quadtree
    g_qt.build(g_rects, g_rects);

#if defined(PLATFORM_WEB)
    emscripten_set_main_loop(update_draw_frame, 0, 1);
#else
    while(!WindowShouldClose()) {
        update_draw_frame();
    }
#endif

    CloseWindow();

    return 0;
}