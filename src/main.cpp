#include <cassert>
#include <vector>
#include <random>
#include <cstdint>

#include "resources/16x16icons.h"
#include "renderer.h"
#include "quadtree.h"

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

void draw_quadtree_impl(quadtree &qt, node_id nid, bbox const &bb, auto const& r, auto const& tex_handle) {

    r.draw_rectangle(
        (vec2){bb.minx, bb.miny},
        (vec2){bb.maxx, bb.maxy},
        (color){130, 130, 130, 255});

    // split recursively if there are children, draw points otherwise
    bbox bb_nw, bb_ne, bb_sw, bb_se;
    bb.split_4(bb_nw, bb_ne, bb_sw, bb_se);

    if (qt.nodes[nid].nw != empty) draw_quadtree_impl(qt, qt.nodes[nid].nw, bb_nw, r, tex_handle);
    if (qt.nodes[nid].ne != empty) draw_quadtree_impl(qt, qt.nodes[nid].ne, bb_ne, r, tex_handle);
    if (qt.nodes[nid].sw != empty) draw_quadtree_impl(qt, qt.nodes[nid].sw, bb_sw, r, tex_handle);
    if (qt.nodes[nid].se != empty) draw_quadtree_impl(qt, qt.nodes[nid].se, bb_se, r, tex_handle);

    if (qt.nodes[nid].nw == empty &&
        qt.nodes[nid].ne == empty &&
        qt.nodes[nid].sw == empty &&
        qt.nodes[nid].se == empty){
        auto start_inc = qt.node_points_begin[nid];
        auto end_excl = qt.node_points_begin[nid + 1];
        for (auto i = start_inc; i < end_excl; i++) {
            r.draw_rectangle(
                (vec2){qt.pointboxes[i].bb.minx, qt.pointboxes[i].bb.miny},
                (vec2){qt.pointboxes[i].bb.maxx, qt.pointboxes[i].bb.maxy},
                (color){200, 200, 200, 255});
            r.draw_sprite(
                (vec2){qt.pointboxes[i].bb.minx, qt.pointboxes[i].bb.miny},
                (vec2){(i % 16) * 16, (i / 16) * 16},
                16, 16, tex_handle);
        }
    }
}

void draw_quadtree(quadtree &qt, auto const& r, auto const& tex_handle) {
    bbox big_box = bbox();
    std::for_each(
        &qt.pointboxes.front(),
        &qt.pointboxes.back(),
        [&big_box](pointbox &pb) mutable {big_box |= pb.mid;}
    );
    draw_quadtree_impl(qt, qt.root, big_box, r, tex_handle);
}

quadtree _qt;
std::vector<rectangle> rects;
std::vector<std::uint32_t> indices;

void draw_query(auto const& r) {

    static constexpr bbox bb = {
        -8.0f, -8.0f, 8.0f, 8.0f
    };

    auto mpos = r.get_mouse_position();

    // highlight query results
    static std::vector<std::uint32_t> query_results;
    bbox offset_bb = {
        mpos.x + bb.minx, mpos.y + bb.miny, mpos.x + bb.maxx, mpos.y + bb.maxy
    };
    query_results.clear();
    _qt.query(offset_bb, query_results);

    for (auto i = 0U; i < query_results.size(); i++)
        r.draw_rectangle(
            (vec2){_qt.pointboxes[query_results[i]].bb.minx, _qt.pointboxes[query_results[i]].bb.miny},
            (vec2){_qt.pointboxes[query_results[i]].bb.maxx, _qt.pointboxes[query_results[i]].bb.maxy},
            (color){255, 0, 0, 255});
    
    // draw box at cursor
    r.draw_rectangle_fill(
        (vec2){mpos.x + bb.minx, mpos.y + bb.miny}, 
        (vec2){mpos.x + bb.maxx, mpos.y + bb.maxy},
        (color){0, 228, 48, 255});
}

void update_draw_frame(auto const& r, auto const& tex_handle) {
    r.start_drawing();
    r.clear_screen();
    draw_quadtree(_qt, r, tex_handle);
    draw_query(r);
    r.stop_drawing();
}

int main(int argc, char **argv) {
    constexpr int screen_width = 640;
    constexpr int screen_height = 480;

    raylib_renderer r;
    r.init_window(screen_width, screen_height, "quadtree");
    auto icon_tex = r.texture_from_memory(__16x16icons_png, __16x16icons_png_len);

    // generate a bunch of boxes and build a quadtree
    auto e1 = std::default_random_engine(1);
    auto x_dist = std::uniform_int_distribution(80 + 12, 560 - 12);
    auto y_dist = std::uniform_int_distribution(60 + 12, 420 - 12);

    rects.reserve(140U);
    indices.reserve(140U);
    for (auto i = 0U; i < 140U; i++) {
        rects.emplace_back();
        rects[i].min = {(float)x_dist(e1), (float)y_dist(e1)};
        rects[i].max = {rects[i].min.x + 16.f, rects[i].min.y + 16.f};
        indices.push_back(i);
    }

    _qt = build<rectangle>(rects, indices, [](rectangle &r){
        bbox bb;
        bb.minx = r.min.x;
        bb.miny = r.min.y;
        bb.maxx = r.max.x;
        bb.maxy = r.max.y;
        return bb;
    });

#if defined(PLATFORM_WEB)
    emscripten_set_main_loop(update_draw_frame, 0, 1);
#else
    while(!r.window_should_close()) {
        update_draw_frame(r, icon_tex);
    }
#endif

    r.close_window();

    return 0;
}