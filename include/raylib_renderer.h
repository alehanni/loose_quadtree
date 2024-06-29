#ifndef RAYLIBRENDERER_H
#define RAYLIBRENDERER_H

#include "raylib.h"

struct raylib_renderer {

    static void start_drawing() {
        BeginDrawing();
    }

    static void stop_drawing() {
        EndDrawing();
    }

    static void init_window(int width, int height, const char* title){
        InitWindow(width, height, title);
#if !defined(PLATFORM_WEB)
        SetTargetFPS(60);
#endif
    }

    static void close_window() {
        CloseWindow();
    }

    static bool window_should_close() {
        return WindowShouldClose();
    }

    static void clear_screen() {
        ClearBackground(DARKGRAY);
    }

    static auto get_mouse_position() {
        return GetMousePosition();
    }

    static auto texture_from_memory(unsigned char data[], unsigned int size) {
        auto img = LoadImageFromMemory(".png", data, size);
        return LoadTextureFromImage(img);
    }

    static void draw_rectangle(auto xmin, auto xmax, auto rgba) {
        DrawRectangleLines(xmin.x, xmin.y, xmax.x - xmin.x, xmax.y - xmin.y, {rgba.r, rgba.g, rgba.b, rgba.a});
    }

    static void draw_rectangle_fill(auto xmin, auto xmax, auto rgba) {
        DrawRectangle(xmin.x, xmin.y, xmax.x - xmin.x, xmax.y - xmin.y, {rgba.r, rgba.g, rgba.b, rgba.a});
    }

    static void draw_sprite(auto xy_dest, auto xy_src, auto w, auto h, auto id) {
        DrawTextureRec(id, {x: xy_src.x, y: xy_src.y, width: w, height: h}, {x: xy_dest.x, y: xy_dest.y}, WHITE);
    }

};

#endif