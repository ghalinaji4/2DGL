#ifndef D2DGL_H
#define D2DGL_H

#include <stdbool.h>
#include <stdint.h>

typedef struct D2DGL_Context D2DGL_Context;

typedef struct {
    int w;
    int h;
    uint32_t *pixels; // 0xAARRGGBB; alpha == 0 is transparent
} D2DGL_Sprite;

enum {
    D2DGL_KEY_ESC = 0,
    D2DGL_KEY_SPACE,
    D2DGL_KEY_W,
    D2DGL_KEY_A,
    D2DGL_KEY_S,
    D2DGL_KEY_D,
    D2DGL_KEY_E,
    D2DGL_KEY_H,
    D2DGL_KEY_UP,
    D2DGL_KEY_DOWN,
    D2DGL_KEY_LEFT,
    D2DGL_KEY_RIGHT,
    D2DGL_KEY_F5,
    D2DGL_KEY_F9,
    D2DGL_KEY_COUNT
};

D2DGL_Context *d2dgl_create(int width, int height, const char *title);
void d2dgl_destroy(D2DGL_Context *ctx);

bool d2dgl_poll_events(D2DGL_Context *ctx);
float d2dgl_delta_time(const D2DGL_Context *ctx);
void d2dgl_present(D2DGL_Context *ctx);

void d2dgl_clear(D2DGL_Context *ctx, uint32_t argb);
void d2dgl_put_pixel(D2DGL_Context *ctx, int x, int y, uint32_t argb);
void d2dgl_fill_rect(D2DGL_Context *ctx, int x, int y, int w, int h, uint32_t argb);
void d2dgl_fill_circle(D2DGL_Context *ctx, int cx, int cy, int radius, uint32_t argb);
void d2dgl_draw_ring(D2DGL_Context *ctx, int cx, int cy, int outerRadius, int innerRadius, uint32_t argb);
void d2dgl_fill_rotated_rect(D2DGL_Context *ctx, float cx, float cy, float w, float h, float angleDeg, uint32_t argb);

void d2dgl_draw_sprite(
    D2DGL_Context *ctx,
    const D2DGL_Sprite *spr,
    int dstX, int dstY,
    int dstW, int dstH,
    uint32_t tint
);

void d2dgl_draw_sprite_rotated(
    D2DGL_Context *ctx,
    const D2DGL_Sprite *spr,
    float centerX, float centerY,
    float dstW, float dstH,
    float angleDeg,
    uint32_t tint
);

bool d2dgl_key_down(const D2DGL_Context *ctx, int key);
bool d2dgl_key_pressed(const D2DGL_Context *ctx, int key);
bool d2dgl_mouse_down(const D2DGL_Context *ctx);
void d2dgl_mouse_pos(const D2DGL_Context *ctx, int *outX, int *outY);

#endif
