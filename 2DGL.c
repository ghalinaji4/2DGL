#include "libgl.h"
#include <stdlib.h>
#include <windows.h>

struct LibGL_Context {
    HWND hwnd;
    HDC hdc;
    int width;
    int height;
    uint32_t *pixels;
    BITMAPINFO bmi;
    bool should_close;
};

static LRESULT CALLBACK libgl_wndproc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam) {
    LibGL_Context *ctx = (LibGL_Context *)GetWindowLongPtrA(hwnd, GWLP_USERDATA);

    switch (msg) {
        case WM_CLOSE:
            if (ctx) ctx->should_close = true;
            DestroyWindow(hwnd);
            return 0;
        case WM_DESTROY:
            if (ctx) ctx->should_close = true;
            PostQuitMessage(0);
            return 0;
        default:
            return DefWindowProcA(hwnd, msg, wparam, lparam);
    }
}

LibGL_Context *libgl_create(int width, int height, const char *title) {
    if (width <= 0 || height <= 0) return NULL;

    LibGL_Context *ctx = (LibGL_Context *)calloc(1, sizeof(LibGL_Context));
    if (!ctx) return NULL;

    ctx->width = width;
    ctx->height = height;
    ctx->pixels = (uint32_t *)calloc((size_t)width * (size_t)height, sizeof(uint32_t));
    if (!ctx->pixels) {
        free(ctx);
        return NULL;
    }

    HINSTANCE hinstance = GetModuleHandleA(NULL);
    const char *class_name = "LibGLWindowClass";

    WNDCLASSA wc = { 0 };
    wc.lpfnWndProc = libgl_wndproc;
    wc.hInstance = hinstance;
    wc.lpszClassName = class_name;
    wc.hCursor = LoadCursorA(NULL, IDC_ARROW);
    RegisterClassA(&wc);

    RECT rect = { 0, 0, width, height };
    AdjustWindowRect(&rect, WS_OVERLAPPEDWINDOW, FALSE);

    ctx->hwnd = CreateWindowExA(
        0,
        class_name,
        title ? title : "LIBGL",
        WS_OVERLAPPEDWINDOW | WS_VISIBLE,
        CW_USEDEFAULT,
        CW_USEDEFAULT,
        rect.right - rect.left,
        rect.bottom - rect.top,
        NULL,
        NULL,
        hinstance,
        NULL
    );

    if (!ctx->hwnd) {
        free(ctx->pixels);
        free(ctx);
        return NULL;
    }

    SetWindowLongPtrA(ctx->hwnd, GWLP_USERDATA, (LONG_PTR)ctx);
    ctx->hdc = GetDC(ctx->hwnd);

    ctx->bmi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
    ctx->bmi.bmiHeader.biWidth = width;
    ctx->bmi.bmiHeader.biHeight = -height; // Top-down pixel buffer
    ctx->bmi.bmiHeader.biPlanes = 1;
    ctx->bmi.bmiHeader.biBitCount = 32;
    ctx->bmi.bmiHeader.biCompression = BI_RGB;

    return ctx;
}

void libgl_destroy(LibGL_Context *ctx) {
    if (!ctx) return;
    if (ctx->hdc && ctx->hwnd) {
        ReleaseDC(ctx->hwnd, ctx->hdc);
    }
    if (ctx->hwnd) {
        DestroyWindow(ctx->hwnd);
    }
    free(ctx->pixels);
    free(ctx);
}

bool libgl_poll_events(LibGL_Context *ctx) {
    if (!ctx) return false;
    MSG msg;
    while (PeekMessageA(&msg, NULL, 0, 0, PM_REMOVE)) {
        if (msg.message == WM_QUIT) {
            ctx->should_close = true;
        }
        TranslateMessage(&msg);
        DispatchMessageA(&msg);
    }
    return !ctx->should_close;
}

void libgl_clear(LibGL_Context *ctx, uint32_t argb) {
    if (!ctx || !ctx->pixels) return;
    int count = ctx->width * ctx->height;
    for (int i = 0; i < count; i++) {
        ctx->pixels[i] = argb;
    }
}

void libgl_put_pixel(LibGL_Context *ctx, int x, int y, uint32_t argb) {
    if (!ctx || !ctx->pixels) return;
    if (x < 0 || y < 0 || x >= ctx->width || y >= ctx->height) return;
    ctx->pixels[y * ctx->width + x] = argb;
}

void libgl_present(LibGL_Context *ctx) {
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

uint32_t *libgl_get_framebuffer(LibGL_Context *ctx) {
    return ctx ? ctx->pixels : NULL;
}

int libgl_get_width(const LibGL_Context *ctx) {
    return ctx ? ctx->width : 0;
}

int libgl_get_height(const LibGL_Context *ctx) {
    return ctx ? ctx->height : 0;
}