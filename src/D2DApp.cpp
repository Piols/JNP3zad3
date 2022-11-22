#include <d2d1_3.h>
#include <cmath>
#include <array>
#include <wincodec.h>
#include <combaseapi.h>
#include "D2DApp.h"
#include "clock.h"

namespace {
	// Deklaracje u¿ycia pomocniczych funkcji
	using D2D1::RenderTargetProperties;
	using D2D1::HwndRenderTargetProperties;
	using D2D1::SizeU;
	using D2D1::Point2F;
	using D2D1::BitmapProperties;
	using D2D1::PixelFormat;
	using D2D1::RectF;
	using D2D1::Matrix3x2F;

	// Interfejsy potrzebne do zainicjowania Direct2D
	ID2D1Factory7* d2d_factory = nullptr;
	ID2D1HwndRenderTarget* d2d_render_target = nullptr;
	IWICImagingFactory* wic_factory = nullptr;

	ID2D1SolidColorBrush* brush = nullptr;
	D2D1_COLOR_F const clear_color =
	{ .r = 0.62f, .g = 0.38f, .b = 0.62f, .a = 1.0f };

	ID2D1Bitmap* bitmap_watch = nullptr;
	IWICBitmapFrameDecode* pSource_watch = nullptr;
	IWICBitmapDecoder* pDecoder_watch = nullptr;
	IWICFormatConverter* pConverter_watch = nullptr;

	ID2D1Bitmap* bitmap_digits = nullptr;
	IWICBitmapFrameDecode* pSource_digits = nullptr;
	IWICBitmapDecoder* pDecoder_digits = nullptr;
	IWICFormatConverter* pConverter_digits = nullptr;

	Matrix3x2F transformation, watchTransformation;
	clockTime t;
	int lastTick = -1;
}

void InitDirect2D(HWND hwnd) {
	// Utworzenie fabryki Direct2D
	D2D1CreateFactory(D2D1_FACTORY_TYPE_SINGLE_THREADED, &d2d_factory);
	if (d2d_factory == nullptr) {
		exit(2);
	}

	if (!SUCCEEDED(CoInitializeEx(nullptr, COINIT_APARTMENTTHREADED))) {
		exit(4);
	}
	if (FAILED(
		CoCreateInstance(
			CLSID_WICImagingFactory,
			nullptr,
			CLSCTX_INPROC_SERVER,
			__uuidof(IWICImagingFactory),
			reinterpret_cast<LPVOID*>(&wic_factory)
		))) {
		exit(5);
	}

	HRESULT hr = wic_factory->CreateDecoderFromFilename(
		L"Watch.png",
		nullptr,
		GENERIC_READ,
		WICDecodeMetadataCacheOnLoad,
		&pDecoder_watch
	);
	if (SUCCEEDED(hr))
	{
		// Create the initial frame.
		hr = pDecoder_watch->GetFrame(0, &pSource_watch);
	}
	if (SUCCEEDED(hr))
	{
		// Convert the image format to 32bppPBGRA
		// (DXGI_FORMAT_B8G8R8A8_UNORM + D2D1_ALPHA_MODE_PREMULTIPLIED).
		hr = wic_factory->CreateFormatConverter(&pConverter_watch);
	}
	if (SUCCEEDED(hr))
	{
		hr = pConverter_watch->Initialize(
			pSource_watch,
			GUID_WICPixelFormat32bppPBGRA,
			WICBitmapDitherTypeNone,
			nullptr,
			0.f,
			WICBitmapPaletteTypeMedianCut
		);
	}

	hr = wic_factory->CreateDecoderFromFilename(
		L"Digits.png",
		nullptr,
		GENERIC_READ,
		WICDecodeMetadataCacheOnLoad,
		&pDecoder_digits
	);
	if (SUCCEEDED(hr))
	{
		// Create the initial frame.
		hr = pDecoder_digits->GetFrame(0, &pSource_digits);
	}
	if (SUCCEEDED(hr))
	{
		// Convert the image format to 32bppPBGRA
		// (DXGI_FORMAT_B8G8R8A8_UNORM + D2D1_ALPHA_MODE_PREMULTIPLIED).
		hr = wic_factory->CreateFormatConverter(&pConverter_digits);
	}
	if (SUCCEEDED(hr))
	{
		hr = pConverter_digits->Initialize(
			pSource_digits,
			GUID_WICPixelFormat32bppPBGRA,
			WICBitmapDitherTypeNone,
			nullptr,
			0.f,
			WICBitmapPaletteTypeMedianCut
		);
	}

	t = randTime();

	RecreateRenderTarget(hwnd);
}

void RecreateRenderTarget(HWND hwnd) {
	RECT rc;
	GetClientRect(hwnd, &rc);
	// Utworzenie celu renderowania w oknie Windows
	d2d_factory->CreateHwndRenderTarget(
		RenderTargetProperties(),
		HwndRenderTargetProperties(hwnd,
			SizeU(static_cast<UINT32>(rc.right) -
				static_cast<UINT32>(rc.left),
				static_cast<UINT32>(rc.bottom) -
				static_cast<UINT32>(rc.top))),
		&d2d_render_target);

	if (d2d_render_target == nullptr) {
		exit(3);
	}

	// Create a Direct2D bitmap from the WIC bitmap.
	d2d_render_target->CreateBitmapFromWicBitmap(
		pConverter_watch,
		nullptr,
		&bitmap_watch
	);

	d2d_render_target->CreateBitmapFromWicBitmap(
		pConverter_digits,
		nullptr,
		&bitmap_digits
	);

	FLOAT height = rc.bottom - rc.top;
	FLOAT width = rc.right - rc.left;
	transformation = Matrix3x2F::Rotation(-4.0f, Point2F(0, 0));
	transformation.SetProduct(transformation, Matrix3x2F::Translation(width / 2.0f, height / 2.0f));
	watchTransformation.SetProduct(Matrix3x2F::Scale(1.05f, 1.05f, Point2F(0, 0)), transformation);
}

void DestroyRenderTarget() {
	if (d2d_render_target) {
		d2d_render_target->Release();
		d2d_render_target = nullptr;
	}
}

void DestroyDirect2D() {
	// Bezpieczne zwolnienie zasobów
	if (d2d_render_target) d2d_render_target->Release();
	if (d2d_factory) d2d_factory->Release();
}

D2D1_RECT_F getDigit(char digit) {
	return RectF(108 * (digit - '0'), 0, 108 * ((digit - '0') + 1), 192);
}

void OnPaint(HWND hwnd, int tick) {
	if (!d2d_render_target) RecreateRenderTarget(hwnd);

	if (tick != lastTick && tick % 60 == 0) {
		t = nextTime(t);
	}
	lastTick = tick;

	d2d_render_target->BeginDraw();
	d2d_render_target->Clear(clear_color);

	d2d_render_target->SetTransform(watchTransformation);
	d2d_render_target->DrawBitmap(
		bitmap_watch,
		RectF(-335, -200, 335, 200),
		1.0f,
		//D2D1_BITMAP_INTERPOLATION_MODE_NEAREST_NEIGHBOR);
		D2D1_BITMAP_INTERPOLATION_MODE_LINEAR);

	d2d_render_target->SetTransform(transformation);
	
	d2d_render_target->DrawBitmap(
		bitmap_digits,
		RectF(-240, -96, -132, 96),
		0.6f,
		//D2D1_BITMAP_INTERPOLATION_MODE_NEAREST_NEIGHBOR);
		D2D1_BITMAP_INTERPOLATION_MODE_LINEAR,
		getDigit(t[0])
	);
	d2d_render_target->DrawBitmap(
		bitmap_digits,
		RectF(-132, -96, -24, 96),
		0.6f,
		//D2D1_BITMAP_INTERPOLATION_MODE_NEAREST_NEIGHBOR);
		D2D1_BITMAP_INTERPOLATION_MODE_LINEAR,
		getDigit(t[1])
	);
	d2d_render_target->DrawBitmap(
		bitmap_digits,
		RectF(24, -96, 132, 96),
		0.6f,
		//D2D1_BITMAP_INTERPOLATION_MODE_NEAREST_NEIGHBOR);
		D2D1_BITMAP_INTERPOLATION_MODE_LINEAR,
		getDigit(t[2])
	);
	d2d_render_target->DrawBitmap(
		bitmap_digits,
		RectF(132, -96, 240, 96),
		0.6f,
		//D2D1_BITMAP_INTERPOLATION_MODE_NEAREST_NEIGHBOR);
		D2D1_BITMAP_INTERPOLATION_MODE_LINEAR,
		getDigit(t[3])
	);

	if (tick % 2) {
		d2d_render_target->DrawBitmap(
			bitmap_digits,
			RectF(-50, -96, 50, 96),
			0.6f,
			//D2D1_BITMAP_INTERPOLATION_MODE_NEAREST_NEIGHBOR);
			D2D1_BITMAP_INTERPOLATION_MODE_LINEAR,
			RectF(108 * 10, 0, 108 * 10 + 100, 192)
		);
	}


	if (d2d_render_target->EndDraw() == D2DERR_RECREATE_TARGET) {
		DestroyRenderTarget();
		OnPaint(hwnd, tick);
	}
}