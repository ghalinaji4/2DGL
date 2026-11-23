#define WIN32_LEAN_AND_MEAN
#include "2dgl.h"
#include <windows.h>
#include <math.h>
#include <stdlib.h>
#include <string.h>

struct D2DGL_Context {
    HWND hwnd;
    HDC hdc;
    BITMAPINFO bmi;
    uint32_t *pixels;
    int width;
    int height;
    bool running;
    bool keyDown[D2DGL_KEY_COUNT];
    bool keyPrev[D2DGL_KEY_COUNT];
    bool mouseDown;
    int mouseX;
    int mouseY;
    LARGE_INTEGER freq;
    LARGE_INTEGER lastCounter;
    float dt;
};

static int map_vk_to_key(WPARAM vk) {
    switch (vk) {
        case VK_ESCAPE: return D2DGL_KEY_ESC;
        case VK_SPACE:  return D2DGL_KEY_SPACE;
        case 'W':       return D2DGL_KEY_W;
        case 'A':       return D2DGL_KEY_A;
        case 'S':       return D2DGL_KEY_S;
        case 'D':       return D2DGL_KEY_D;
        case 'E':       return D2DGL_KEY_E;
        case 'H':       return D2DGL_KEY_H;
        case VK_UP:     return D2DGL_KEY_UP;
        case VK_DOWN:   return D2DGL_KEY_DOWN;
        case VK_LEFT:   return D2DGL_KEY_LEFT;
        case VK_RIGHT:  return D2DGL_KEY_RIGHT;
        case VK_F5:     return D2DGL_KEY_F5;
        case VK_F9:     return D2DGL_KEY_F9;
        default:        return -1;
    }
}

static LRESULT CALLBACK d2dgl_wndproc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    D2DGL_Context *ctx = (D2DGL_Context *)GetWindowLongPtrA(hwnd, GWLP_USERDATA);

    switch (msg) {
        case WM_CREATE: {
            CREATESTRUCT *cs = (CREATESTRUCT *)lParam;
            SetWindowLongPtrA(hwnd, GWLP_USERDATA, (LONG_PTR)cs->lpCreateParams);
            return 0;
        }
        case WM_CLOSE:
            if (ctx) ctx->running = false;
            DestroyWindow(hwnd);
            return 0;
        case WM_DESTROY:
            PostQuitMessage(0);
            return 0;
        case WM_KEYDOWN:
            if (ctx) {
                int k = map_vk_to_key(wParam);
                if (k >= 0 && k < D2DGL_KEY_COUNT) ctx->keyDown[k] = true;
            }
            return 0;
        case WM_KEYUP:
            if (ctx) {
                int k = map_vk_to_key(wParam);
                if (k >= 0 && k < D2DGL_KEY_COUNT) ctx->keyDown[k] = false;
            }
            return 0;
        case WM_LBUTTONDOWN:
            if (ctx) {
                ctx->mouseDown = true;
                ctx->mouseX = (int)(short)LOWORD(lParam);
                ctx->mouseY = (int)(short)HIWORD(lParam);
            }
            return 0;
        case WM_LBUTTONUP:
            if (ctx) {
                ctx->mouseDown = false;
                ctx->mouseX = (int)(short)LOWORD(lParam);
                ctx->mouseY = (int)(short)HIWORD(lParam);
            }
            return 0;
        case WM_MOUSEMOVE:
            if (ctx) {
                ctx->mouseX = (int)(short)LOWORD(lParam);
                ctx->mouseY = (int)(short)HIWORD(lParam);
            }
            return 0;
    }
    return DefWindowProcA(hwnd, msg, wParam, lParam);
}

D2DGL_Context *d2dgl_create(int width, int height, const char *title) {
    if (width <= 0 || height <= 0) return NULL;

    D2DGL_Context *ctx = (D2DGL_Context *)calloc(1, sizeof(D2DGL_Context));
    if (!ctx) return NULL;

    ctx->pixels = (uint32_t *)calloc((size_t)(width * height), sizeof(uint32_t));
    if (!ctx->pixels) {
        free(ctx);
        return NULL;
    }

    ctx->width = width;
    ctx->height = height;
    ctx->running = true;
    ctx->dt = 1.0f / 60.0f;

    QueryPerformanceFrequency(&ctx->freq);
    QueryPerformanceCounter(&ctx->lastCounter);

    HINSTANCE hInstance = GetModuleHandleA(NULL);
    WNDCLASSA wc = { 0 };
    wc.lpfnWndProc = d2dgl_wndproc;
    wc.hInstance = hInstance;
    wc.lpszClassName = "D2DGL_WindowClass";
    wc.hCursor = LoadCursorA(NULL, IDC_CROSS);
    RegisterClassA(&wc);

    RECT rect = { 0, 0, width, height };
    AdjustWindowRect(&rect, WS_OVERLAPPEDWINDOW, FALSE);

    ctx->hwnd = CreateWindowExA(
        0,
        wc.lpszClassName,
        title ? title : "2DGL",
        WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, CW_USEDEFAULT,
        rect.right - rect.left,
        rect.bottom - rect.top,
        NULL, NULL, hInstance, ctx
    );

    if (!ctx->hwnd) {
        free(ctx->pixels);
        free(ctx);
        return NULL;
    }

    ctx->hdc = GetDC(ctx->hwnd);
    ctx->bmi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
    ctx->bmi.bmiHeader.biWidth = width;
    ctx->bmi.bmiHeader.biHeight = -height; // top-down DIB
    ctx->bmi.bmiHeader.biPlanes = 1;
    ctx->bmi.bmiHeader.biBitCount = 32;
    ctx->bmi.bmiHeader.biCompression = BI_RGB;

    ShowWindow(ctx->hwnd, SW_SHOW);
    UpdateWindow(ctx->hwnd);
    return ctx;
}

void d2dgl_destroy(D2DGL_Context *ctx) {
    if (!ctx) return;
    if (ctx->hdc && ctx->hwnd) ReleaseDC(ctx->hwnd, ctx->hdc);
    if (ctx->hwnd) DestroyWindow(ctx->hwnd);
    free(ctx->pixels);
    free(ctx);
}

bool d2dgl_poll_events(D2DGL_Context *ctx) {
    if (!ctx || !ctx->running) return false;

    memcpy(ctx->keyPrev, ctx->keyDown, sizeof(ctx->keyDown));

    MSG msg;
    while (PeekMessageA(&msg, NULL, 0, 0, PM_REMOVE)) {
        if (msg.message == WM_QUIT) {
            ctx->running = false;
            return false;
        }
        TranslateMessage(&msg);
        DispatchMessageA(&msg);
    }

    LARGE_INTEGER now;
    QueryPerformanceCounter(&now);
    ctx->dt = (float)(now.QuadPart - ctx->lastCounter.QuadPart) / (float)ctx->freq.QuadPart;
    ctx->lastCounter = now;
    if (ctx->dt <= 0.0f) ctx->dt = 1.0f / 60.0f;
    if (ctx->dt > 0.05f) ctx->dt = 0.05f;

    return ctx->running;
}

float d2dgl_delta_time(const D2DGL_Context *ctx) {
    return ctx ? ctx->dt : (1.0f / 60.0f);
}

void d2dgl_present(D2DGL_Context *ctx) {
    if (!ctx || !ctx->hdc || !ctx->pixels) return;
    StretchDIBits(
        ctx->hdc,
        0, 0, ctx->width, ctx->height,
        0, 0, ctx->width, ctx->height,
        ctx->pixels,
        &ctx->bmi,
        DIB_RGB_COLORS,
        SRCCOPY
    );
}

void d2dgl_clear(D2DGL_Context *ctx, uint32_t argb) {
    if (!ctx || !ctx->pixels) return;
    int count = ctx->width * ctx->height;
    for (int i = 0; i < count; i++) {
        ctx->pixels[i] = argb;
    }
}

void d2dgl_put_pixel(D2DGL_Context *ctx, int x, int y, uint32_t argb) {
    if (!ctx || !ctx->pixels) return;
    if (x < 0 || x >= ctx->width || y < 0 || y >= ctx->height) return;
    ctx->pixels[y * ctx->width + x] = argb;
}

void d2dgl_fill_rect(D2DGL_Context *ctx, int x, int y, int w, int h, uint32_t argb) {
    if (!ctx || !ctx->pixels || w <= 0 || h <= 0) return;

    int x0 = x < 0 ? 0 : x;
    int y0 = y < 0 ? 0 : y;
    int x1 = x + w;
    int y1 = y + h;
    if (x1 > ctx->width) x1 = ctx->width;
    if (y1 > ctx->height) y1 = ctx->height;

    for (int py = y0; py < y1; py++) {
        uint32_t *row = &ctx->pixels[py * ctx->width];
        for (int px = x0; px < x1; px++) {
            row[px] = argb;
        }
    }
}

void d2dgl_fill_circle(D2DGL_Context *ctx, int cx, int cy, int radius, uint32_t argb) {
    if (!ctx || !ctx->pixels || radius <= 0) return;
    int r2 = radius * radius;
    int y0 = cy - radius < 0 ? 0 : cy - radius;
    int y1 = cy + radius >= ctx->height ? ctx->height - 1 : cy + radius;
    int x0 = cx - radius < 0 ? 0 : cx - radius;
    int x1 = cx + radius >= ctx->width ? ctx->width - 1 : cx + radius;

    for (int y = y0; y <= y1; y++) {
        int dy = y - cy;
        for (int x = x0; x <= x1; x++) {
            int dx = x - cx;
            if (dx * dx + dy * dy <= r2) {
                ctx->pixels[y * ctx->width + x] = argb;
            }
        }
    }
}

void d2dgl_draw_ring(D2DGL_Context *ctx, int cx, int cy, int outerRadius, int innerRadius, uint32_t argb) {
    if (!ctx || !ctx->pixels || outerRadius <= 0) return;
    if (innerRadius < 0) innerRadius = 0;
    int rOut2 = outerRadius * outerRadius;
    int rIn2 = innerRadius * innerRadius;

    int y0 = cy - outerRadius < 0 ? 0 : cy - outerRadius;
    int y1 = cy + outerRadius >= ctx->height ? ctx->height - 1 : cy + outerRadius;
    int x0 = cx - outerRadius < 0 ? 0 : cx - outerRadius;
    int x1 = cx + outerRadius >= ctx->width ? ctx->width - 1 : cx + outerRadius;

    for (int y = y0; y <= y1; y++) {
        int dy = y - cy;
        for (int x = x0; x <= x1; x++) {
            int dx = x - cx;
            int d2 = dx * dx + dy * dy;
            if (d2 <= rOut2 && d2 >= rIn2) {
                ctx->pixels[y * ctx->width + x] = argb;
            }
        }
    }
}

void d2dgl_fill_rotated_rect(D2DGL_Context *ctx, float cx, float cy, float w, float h, float angleDeg, uint32_t argb) {
    if (!ctx || !ctx->pixels || w <= 0.5f || h <= 0.5f) return;

    float rad = -angleDeg * 0.0174532925f;
    float cosA = cosf(rad);
    float sinA = sinf(rad);

    float hw = w * 0.5f;
    float hh = h * 0.5f;
    float bound = sqrtf(hw * hw + hh * hh);

    int x0 = (int)floorf(cx - bound);
    int x1 = (int)ceilf(cx + bound);
    int y0 = (int)floorf(cy - bound);
    int y1 = (int)ceilf(cy + bound);

    if (x0 < 0) x0 = 0;
    if (y0 < 0) y0 = 0;
    if (x1 >= ctx->width) x1 = ctx->width - 1;
    if (y1 >= ctx->height) y1 = ctx->height - 1;

    for (int y = y0; y <= y1; y++) {
        float dy = (float)y - cy;
        for (int x = x0; x <= x1; x++) {
            float dx = (float)x - cx;
            float lx = dx * cosA - dy * sinA;
            float ly = dx * sinA + dy * cosA;
            if (fabsf(lx) <= hw && fabsf(ly) <= hh) {
                ctx->pixels[y * ctx->width + x] = argb;
            }
        }
    }
}

static uint32_t d2dgl_modulate_color(uint32_t texel, uint32_t tint) {
    if ((texel >> 24) == 0u) return 0u;
    uint32_t r = (((texel >> 16) & 0xFFu) * ((tint >> 16) & 0xFFu)) / 255u;
    uint32_t g = (((texel >> 8)  & 0xFFu) * ((tint >> 8)  & 0xFFu)) / 255u;
    uint32_t b = (( texel         & 0xFFu) * ( tint         & 0xFFu)) / 255u;
    return 0xFF000000u | (r << 16) | (g << 8) | b;
}

void d2dgl_draw_sprite(
    D2DGL_Context *ctx,
    const D2DGL_Sprite *spr,
    int dstX, int dstY,
    int dstW, int dstH,
    uint32_t tint
) {
    if (!ctx || !ctx->pixels || !spr || !spr->pixels || spr->w <= 0 || spr->h <= 0 || dstW <= 0 || dstH <= 0) {
        return;
    }

    int x0 = dstX < 0 ? 0 : dstX;
    int y0 = dstY < 0 ? 0 : dstY;
    int x1 = dstX + dstW > ctx->width ? ctx->width : dstX + dstW;
    int y1 = dstY + dstH > ctx->height ? ctx->height : dstY + dstH;

    for (int py = y0; py < y1; py++) {
        int sy = ((py - dstY) * spr->h) / dstH;
        if (sy < 0) sy = 0;
        if (sy >= spr->h) sy = spr->h - 1;

        for (int px = x0; px < x1; px++) {
            int sx = ((px - dstX) * spr->w) / dstW;
            if (sx < 0) sx = 0;
            if (sx >= spr->w) sx = spr->w - 1;

            uint32_t col = d2dgl_modulate_color(spr->pixels[sy * spr->w + sx], tint);
            if ((col >> 24) != 0u) {
                ctx->pixels[py * ctx->width + px] = col;
            }
        }
    }
}

void d2dgl_draw_sprite_rotated(
    D2DGL_Context *ctx,
    const D2DGL_Sprite *spr,
    float centerX, float centerY,
    float dstW, float dstH,
    float angleDeg,
    uint32_t tint
) {
    if (!ctx || !ctx->pixels || !spr || !spr->pixels || spr->w <= 0 || spr->h <= 0 || dstW <= 1.0f || dstH <= 1.0f) {
        return;
    }

    float rad = -angleDeg * 0.0174532925f;
    float cosA = cosf(rad);
    float sinA = sinf(rad);

    float halfW = dstW * 0.5f;
    float halfH = dstH * 0.5f;
    float bound = sqrtf(halfW * halfW + halfH * halfH);

    int x0 = (int)floorf(centerX - bound);
    int x1 = (int)ceilf(centerX + bound);
    int y0 = (int)floorf(centerY - bound);
    int y1 = (int)ceilf(centerY + bound);

    if (x0 < 0) x0 = 0;
    if (y0 < 0) y0 = 0;
    if (x1 >= ctx->width) x1 = ctx->width - 1;
    if (y1 >= ctx->height) y1 = ctx->height - 1;

    for (int y = y0; y <= y1; y++) {
        float dy = (float)y - centerY;
        for (int x = x0; x <= x1; x++) {
            float dx = (float)x - centerX;

            float localX = dx * cosA - dy * sinA;
            float localY = dx * sinA + dy * cosA;

            float u = (localX + halfW) / dstW;
            float v = (localY + halfH) / dstH;
            if (u < 0.0f || u >= 1.0f || v < 0.0f || v >= 1.0f) continue;

            int sx = (int)(u * (float)spr->w);
            int sy = (int)(v * (float)spr->h);
            uint32_t col = d2dgl_modulate_color(spr->pixels[sy * spr->w + sx], tint);
            if ((col >> 24) != 0u) {
                ctx->pixels[y * ctx->width + x] = col;
            }
        }
    }
}

bool d2dgl_key_down(const D2DGL_Context *ctx, int key) {
    if (!ctx || key < 0 || key >= D2DGL_KEY_COUNT) return false;
    return ctx->keyDown[key];
}

bool d2dgl_key_pressed(const D2DGL_Context *ctx, int key) {
    if (!ctx || key < 0 || key >= D2DGL_KEY_COUNT) return false;
    return ctx->keyDown[key] && !ctx->keyPrev[key];
}

bool d2dgl_mouse_down(const D2DGL_Context *ctx) {
    return ctx ? ctx->mouseDown : false;
}

void d2dgl_mouse_pos(const D2DGL_Context *ctx, int *outX, int *outY) {
    if (!ctx) return;
    if (outX) *outX = ctx->mouseX;
    if (outY) *outY = ctx->mouseY;
}
