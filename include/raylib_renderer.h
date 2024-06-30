#ifndef RAYLIBRENDERER_H
#define RAYLIBRENDERER_H

#include "raylib.h"
#include "utiltypes.h"

struct raylib_renderer {

    void start_drawing() {
        BeginDrawing();
    }

    void stop_drawing() {
        EndDrawing();
    }

    void init_window(int width, int height, const char* title){
        InitWindow(width, height, title);
#if !defined(PLATFORM_WEB)
        SetTargetFPS(60);
#endif
    }

    void close_window() {
        CloseWindow();
    }

    bool window_should_close() {
        return WindowShouldClose();
    }

    void clear_screen() {
        ClearBackground(DARKGRAY);
    }

    vec2_t get_mouse_position() {
        Vector2 v = GetMousePosition();
        return {static_cast<double>(v.x), static_cast<double>(v.y)};
    }

    uint64_t texture_from_memory(unsigned char data[], unsigned int size) {
        auto img = LoadImageFromMemory(".png", data, size);

        auto id = texture_handles.size();
        texture_handles.emplace_back();
        texture_handles[id] = LoadTextureFromImage(img);

        return static_cast<uint64_t>(id);
    }

    void draw_rectangle(vec2_t xmin, vec2_t xmax, rgba_t rgba) {
        DrawRectangleLines(
            xmin.x,
            xmin.y,
            xmax.x - xmin.x,
            xmax.y - xmin.y,
            {rgba.r, rgba.g, rgba.b, rgba.a}
        );
    }

    void draw_rectangle_fill(vec2_t xmin, vec2_t xmax, rgba_t rgba) {
        DrawRectangle(
            xmin.x,
            xmin.y,
            xmax.x - xmin.x,
            xmax.y - xmin.y,
            {rgba.r, rgba.g, rgba.b, rgba.a}
        );
    }

    void draw_sprite(vec2_t xy_dest, vec2_t xy_src, double w, double h, uint64_t id) {
        DrawTextureRec(
            texture_handles[id],
            {(float)xy_src.x, (float)xy_src.y, (float)w, (float)h},
            {(float)xy_dest.x, (float)xy_dest.y},
            WHITE);
    }

private:
    std::vector<Texture2D> texture_handles;
};

#endif