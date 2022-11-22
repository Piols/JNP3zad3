#pragma once

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

void InitDirect2D(HWND hwnd);
void RecreateRenderTarget(HWND hwnd);
void DestroyRenderTarget();
void DestroyDirect2D();
void OnPaint(HWND hwnd, int tick);