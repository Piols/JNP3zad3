#include "WinMain.h"
#include "D2DApp.h"
#include <cstdlib>
#include <ctime>

LRESULT CALLBACK WindowProc(HWND hwnd, UINT Msg, WPARAM wParam, LPARAM lParam) {
	static INT tick = 0;
	switch (Msg) {
	case WM_CREATE:
		SetTimer(hwnd, 501, 200, nullptr);
		InitDirect2D(hwnd);
		return 0;
	case WM_DESTROY:
		DestroyDirect2D();
		PostQuitMessage(0);
		return 0;
	case WM_TIMER:
		tick++;
		InvalidateRect(hwnd, nullptr, true);
		return 0;
	case WM_PAINT:
		OnPaint(hwnd, tick);
		ValidateRect(hwnd, nullptr);
		return 0;
	case WM_SIZE:
		DestroyRenderTarget();
		return 0;
	}
	return DefWindowProc(hwnd, Msg, wParam, lParam);
}

INT WINAPI wWinMain(_In_ [[maybe_unused]] HINSTANCE instance,
	_In_opt_ [[maybe_unused]] HINSTANCE prev_instance,
	_In_ [[maybe_unused]] PWSTR cmd_line,
	_In_ [[maybe_unused]] INT cmd_show) {

	srand(time(nullptr));
	WNDCLASSEX wcex;
	wcex.cbSize = sizeof(WNDCLASSEX);
	wcex.style = CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc = WindowProc;
	wcex.cbClsExtra = 0;
	wcex.cbWndExtra = 0;
	wcex.hInstance = instance;
	wcex.hIcon = nullptr;
	wcex.hCursor = LoadCursor(instance, IDC_ARROW);
	wcex.hbrBackground = nullptr, //static_cast<HBRUSH>(GetStockObject(WHITE_BRUSH));
		wcex.lpszMenuName = nullptr;
	wcex.lpszClassName = TEXT("Template Window Class");
	wcex.hIconSm = nullptr;

	RegisterClassEx(&wcex);

	HWND hwnd = CreateWindowEx(
		0, // Optional window styles.
		wcex.lpszClassName, // Window class
		TEXT("Clock"), // Window text
		WS_OVERLAPPEDWINDOW, // Window style

		// Size and position
		CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT,

		nullptr, // Parent window
		nullptr, // Menu
		wcex.hInstance, // Instance handle
		nullptr // Additional application data
	);

	if (hwnd == nullptr) {
		return 1;
	}

	ShowWindow(hwnd, cmd_show);

	MSG msg = {};
	while (BOOL rv = GetMessage(&msg, nullptr, 0, 0) != 0) {
		if (rv < 0) {
			DestroyWindow(hwnd);
			return 1;
		}
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}

	DestroyWindow(hwnd);
	return 0;
}
