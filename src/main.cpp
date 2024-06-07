#include <cassert>
#include <vector>

#include "raylib.h"
#include "quadtree.h"

void draw_quadtree_impl(qtree &qt, node_id nid, bbox const &bb) {
    // draw quadtree box
    DrawRectangleLines(
        bb.minx + 0.5f,
        bb.miny + 0.5f,
        (bb.maxx - bb.minx) + 0.5f,
        (bb.maxy - bb.miny) + 0.5f,
        GRAY
    );

    // split recursively if there are children, draw points otherwise
    bbox bb_nw, bb_ne, bb_sw, bb_se;
    bb.split_4(bb_nw, bb_ne, bb_sw, bb_se);

    if (qt.nodes[nid].nw != null) draw_quadtree_impl(qt, qt.nodes[nid].nw, bb_nw);
    if (qt.nodes[nid].ne != null) draw_quadtree_impl(qt, qt.nodes[nid].ne, bb_ne);
    if (qt.nodes[nid].sw != null) draw_quadtree_impl(qt, qt.nodes[nid].sw, bb_sw);
    if (qt.nodes[nid].se != null) draw_quadtree_impl(qt, qt.nodes[nid].se, bb_se);

    if (qt.nodes[nid].nw == null &&
        qt.nodes[nid].ne == null &&
        qt.nodes[nid].sw == null &&
        qt.nodes[nid].se == null){
        auto start_inc = qt.node_points_begin[nid];
        auto end_excl = qt.node_points_begin[nid + 1];
        for (auto i = start_inc; i < end_excl; i++)
            DrawRectangle(
                qt.pointboxes[i].bb.minx + 0.5f,
                qt.pointboxes[i].bb.miny + 0.5f,
                qt.pointboxes[i].bb.maxx - qt.pointboxes[i].bb.minx + 0.5f,
                qt.pointboxes[i].bb.maxy - qt.pointboxes[i].bb.miny + 0.5f,
                LIGHTGRAY
            );
    }
}

void draw_quadtree(qtree &qt) {
    bbox big_box = bbox();
    std::for_each(
        &qt.pointboxes.front(),
        &qt.pointboxes.back(),
        [&big_box](pointbox &pb) mutable {big_box |= pb.mid;}
    );
    draw_quadtree_impl(qt, qt.root, big_box);
}

qtree _qt;
std::vector<Rectangle> rects;
std::vector<std::uint32_t> indices;

void draw_query() {

    static constexpr bbox bb = {
        -8.0f, -8.0f, 8.0f, 8.0f
    };

    auto mpos = GetMousePosition();

    // highlight query results
    static std::vector<std::uint32_t> query_results;
    bbox offset_bb = {
        mpos.x + bb.minx, mpos.y + bb.miny, mpos.x + bb.maxx, mpos.y + bb.maxy
    };
    query_results.clear();
    _qt.query(offset_bb, query_results);

    for (auto i = 0U; i < query_results.size(); i++)
        DrawRectangle(
            _qt.pointboxes[query_results[i]].bb.minx + 0.5f,
            _qt.pointboxes[query_results[i]].bb.miny + 0.5f,
            _qt.pointboxes[query_results[i]].bb.maxx - _qt.pointboxes[query_results[i]].bb.minx + 0.5f,
            _qt.pointboxes[query_results[i]].bb.maxy - _qt.pointboxes[query_results[i]].bb.miny + 0.5f,
            ORANGE
        );
    
    // draw box at cursor
    DrawRectangle(mpos.x + bb.minx, mpos.y + bb.miny, bb.maxx - bb.minx, bb.maxy - bb.miny, GREEN);
}

void update_draw_frame() {
    BeginDrawing();
    ClearBackground(DARKGRAY);
    draw_quadtree(_qt);
    draw_query();
    EndDrawing();
}

int main(int argc, char **argv) {
    constexpr int screen_width = 640;
    constexpr int screen_height = 480;

    InitWindow(screen_width, screen_height, "quadtree");

    // generate a bunch of boxes and build a quadtree
    SetRandomSeed(1);

    rects.reserve(100U);
    indices.reserve(100U);
    for (auto i = 0U; i < 100U; i++) {
        rects.emplace_back();
        rects[i].x = (float)GetRandomValue(80 + 12, 560 - 12);
        rects[i].y = (float)GetRandomValue(60 + 12, 420 - 12);
        rects[i].width = (float)GetRandomValue(8, 16);
        rects[i].height = (float)GetRandomValue(8, 16);
        indices.push_back(i);
    }

    _qt = build<Rectangle>(rects, indices, [](Rectangle &r){
        bbox bb;
        bb.minx = r.x;
        bb.miny = r.y;
        bb.maxx = r.x + r.width;
        bb.maxy = r.y + r.height;
        return bb;
    });

#if defined(PLATFORM_WEB)
    emscripten_set_main_loop(update_draw_frame, 0, 1);
#else
    SetTargetFPS(60);
    while(!WindowShouldClose()) {
        update_draw_frame();
    }
#endif

    CloseWindow();

    return 0;
}