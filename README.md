# 2dgl

Lightweight 2D pixel framebuffer, drawing, input, and timing library in C for Windows (Win32 + GDI), licensed under MIT.

## Build Example (MSVC 2022)

```bat
cl /nologo /std:c17 /W4 /O2 /Fe:demo.exe demo.c src\2dgl.c /Iinclude /link user32.lib gdi32.lib
