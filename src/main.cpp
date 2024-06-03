#include <cassert>
#include <vector>

#include "raylib.h"
#include "quadtree.h"

void draw_quadtree_impl(quadtree &qt, node_id nid, box const &bbox) {
    // draw bbox
    DrawRectangleLines(
        (int)bbox.min.x,
        (int)bbox.min.y,
        (int)(bbox.max.x - bbox.min.x),
        (int)(bbox.max.y - bbox.min.y),
        LIGHTGRAY
    );

    // split recursively if there are children, draw points otherwise
    point center = middle(bbox.min, bbox.max);
    if (qt.nodes[nid].children[0][0] != null) draw_quadtree_impl(qt, qt.nodes[nid].children[0][0], {bbox.min, center});
    if (qt.nodes[nid].children[0][1] != null) draw_quadtree_impl(qt, qt.nodes[nid].children[0][1], {{center.x, bbox.min.y}, {bbox.max.x, center.y}});
    if (qt.nodes[nid].children[1][0] != null) draw_quadtree_impl(qt, qt.nodes[nid].children[1][0], {{bbox.min.x, center.y}, {center.x, bbox.max.y}});
    if (qt.nodes[nid].children[1][1] != null) draw_quadtree_impl(qt, qt.nodes[nid].children[1][1], {center, bbox.max});

    if (qt.nodes[nid].children[0][0] == null &&
        qt.nodes[nid].children[0][1] == null &&
        qt.nodes[nid].children[1][0] == null &&
        qt.nodes[nid].children[1][1] == null){
        auto start_inc = qt.node_points_begin[nid];
        auto end_excl = qt.node_points_begin[nid + 1];
        for (auto i = start_inc; i < end_excl; i++)
            DrawCircle(qt.points[i].x, qt.points[i].y, 2.f, PINK);
    }
}

void draw_quadtree(quadtree &qt) {
    draw_quadtree_impl(qt, qt.root, bbox(&qt.points[0], &qt.points[qt.points.size()]));
}

quadtree _qt;

void update_draw_frame() {
    BeginDrawing();
    ClearBackground(DARKGRAY);
    draw_quadtree(_qt);
    EndDrawing();
}

int main(int argc, char **argv) {
    constexpr int screen_width = 640;
    constexpr int screen_height = 480;

    InitWindow(screen_width, screen_height, "quadtree");

    // generate a bunch of points and build quadtree
    SetRandomSeed(1);

    std::vector<point> points;
    points.reserve(100U);
    for (auto i = 0U; i < 100U; i++) {
        float x, y;
        x = (float)GetRandomValue(80, 560);
        y = (float)GetRandomValue(60, 420);
        points.push_back({x, y});
    }

    _qt = build(points);

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