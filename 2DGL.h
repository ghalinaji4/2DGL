#ifndef LIBGL_H
#define LIBGL_H

#include <stdbool.h>
#include <stdint.h>

typedef struct LibGL_Context LibGL_Context;

enum {
    LIBGL_KEY_W = 0,
    LIBGL_KEY_A,
    LIBGL_KEY_S,
    LIBGL_KEY_D,
    LIBGL_KEY_E,
    LIBGL_KEY_UP,
    LIBGL_KEY_DOWN,
    LIBGL_KEY_LEFT,
    LIBGL_KEY_RIGHT,
    LIBGL_KEY_SPACE,
    LIBGL_KEY_ESC,
    LIBGL_KEY_F5,
    LIBGL_KEY_F9,
    LIBGL_KEY_COUNT
};

LibGL_Context *libgl_create(int width, int height, const char *title);
void libgl_destroy(LibGL_Context *ctx);

bool libgl_poll_events(LibGL_Context *ctx);
float libgl_delta_time(LibGL_Context *ctx);

bool libgl_key_down(const LibGL_Context *ctx, int key);
bool libgl_key_pressed(const LibGL_Context *ctx, int key);
bool libgl_mouse_down(const LibGL_Context *ctx);
void libgl_mouse_pos(const LibGL_Context *ctx, int *out_x, int *out_y);

uint32_t libgl_rgb(uint8_t r, uint8_t g, uint8_t b);
uint32_t libgl_rgba(uint8_t r, uint8_t g, uint8_t b, uint8_t a);

void libgl_clear(LibGL_Context *ctx, uint32_t argb);
void libgl_put_pixel(LibGL_Context *ctx, int x, int y, uint32_t argb);
void libgl_blend_pixel(LibGL_Context *ctx, int x, int y, uint32_t argb);
void libgl_fill_rect(LibGL_Context *ctx, int x, int y, int w, int h, uint32_t argb);
void libgl_fill_circle(LibGL_Context *ctx, int cx, int cy, int radius, uint32_t argb);
void libgl_draw_ring(LibGL_Context *ctx, int cx, int cy, int outer_r, int inner_r, uint32_t argb);
void libgl_fill_rotated_rect(
    LibGL_Context *ctx,
    float cx, float cy,
    float length, float width,
    float angle_deg,
    uint32_t argb
);

void libgl_present(LibGL_Context *ctx);

uint32_t *libgl_get_framebuffer(LibGL_Context *ctx);
int libgl_get_width(const LibGL_Context *ctx);
int libgl_get_height(const LibGL_Context *ctx);

#endif