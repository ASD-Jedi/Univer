#include <windows.h>
#include "camera.hpp"
#include "hider.hpp"
#define WM_KMESSAGE WM_USER + 1
LRESULT CALLBACK WindowProc(HWND, UINT, WPARAM, LPARAM);
int(*SetHook)(HWND, UINT, UINT);
int(*UnSetHook)();
TCHAR chScreen = 'Q';
TCHAR chShowCamera = 'W';
TCHAR chHideCamera = 'E';
TCHAR chStartRecord = 'R';
TCHAR chStopRecord = 'T';
TCHAR chHide = 'A';

HWND hwndMainWindow;
Hider* hider;
HINSTANCE hHookKeyboardDll;
int APIENTRY WinMain(HINSTANCE hInstance,
	HINSTANCE hPrevInstance,
	LPSTR lpCmdLine,
	int nCmdShow)
{
	MSG msg;
	WNDCLASS wc;
	memset(&wc, 0, sizeof(wc));
	wc.lpszClassName = TEXT("WebCamera");
	wc.hInstance = hInstance;
	wc.lpfnWndProc = WindowProc;
	wc.style = CS_HREDRAW | CS_VREDRAW;
	wc.hbrBackground = (HBRUSH)(COLOR_MENU + 1);
	wc.hCursor = LoadCursor(NULL, IDC_ARROW);

	RegisterClass(&wc);

	hwndMainWindow = ::CreateWindowEx(0,
		TEXT("WebCamera"),
		TEXT("Web Camera"),
		WS_POPUP | WS_VISIBLE | WS_CAPTION | WS_SYSMENU | WS_THICKFRAME,
		0, 0, 450, 250,
		NULL,
		NULL,
		hInstance,
		0);

	hHookKeyboardDll = LoadLibrary(TEXT("hideHookDll.dll"));
	if (hHookKeyboardDll)
	{
		SetHook = (int(*)(HWND, UINT, UINT))GetProcAddress(hHookKeyboardDll, "SetHook");
		UnSetHook = (int(*)())GetProcAddress(hHookKeyboardDll, "UnSetHook");
	}
	
	SetHook(hwndMainWindow, WM_KMESSAGE, 0);
	hider = new Hider(hwndMainWindow);
	while (GetMessage(&msg, NULL, 0, 0))
	{
		DispatchMessage(&msg);
		TranslateMessage(&msg);
	}

	UnSetHook();
	FreeLibrary(hHookKeyboardDll);
	return 0;
}
WebCamera webCam(hwndMainWindow);

LRESULT CALLBACK WindowProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
	switch (uMsg) {
	case WM_CTLCOLORSTATIC:
		return (LRESULT)CreateSolidBrush(0xFFFFFF);
	case WM_CREATE:
		CreateWindow(TEXT("Static"), TEXT("'Q' : MAKE SCREEN"), WS_CHILD | WS_VISIBLE | SS_LEFT,  20,  20, 160, 20, hWnd, (HMENU)101, NULL, NULL);
		CreateWindow(TEXT("Static"), TEXT("'W' : OPEN CAMERA"), WS_CHILD | WS_VISIBLE | SS_LEFT,  20,  50, 160, 20, hWnd, (HMENU)102, NULL, NULL);
		CreateWindow(TEXT("Static"), TEXT("'E' : CLOSE CAMERA"), WS_CHILD | WS_VISIBLE | SS_LEFT, 20,  80, 160, 20, hWnd, (HMENU)103, NULL, NULL);
		CreateWindow(TEXT("Static"), TEXT("'R' : START RECORD"), WS_CHILD | WS_VISIBLE | SS_LEFT, 20, 110, 160, 20, hWnd, (HMENU)103, NULL, NULL);
		CreateWindow(TEXT("Static"), TEXT("'T' : STOP RECORD"), WS_CHILD | WS_VISIBLE | SS_LEFT, 20, 140, 160, 20, hWnd, (HMENU)103, NULL, NULL);
		CreateWindow(TEXT("Static"), TEXT("'A' : HIDE"), WS_CHILD | WS_VISIBLE | SS_LEFT, 20, 170, 160, 20, hWnd, (HMENU)103, NULL, NULL);

		break;
	case WM_KMESSAGE:
		if (lParam == 128) {
			if (wParam == chHideCamera) {
				webCam.hideCamera();
			}
			else if (wParam == chShowCamera) {
				webCam.showCamera();
			}
			else if (wParam == chScreen) {
				webCam.saveFrame();
			}
			else if (wParam == chStartRecord) {
				webCam.startRecord();
			}
			else if (wParam == chStopRecord) {
				webCam.stopRecord();
			}
			else if (wParam == chHide) {
				hider->switchAppMode();
			}
		}
		break;
	case WM_DESTROY:
	case WM_ENDSESSION:
		PostQuitMessage(0);
		break;
	default:
		return DefWindowProc(hWnd, uMsg, wParam, lParam);
	}
	return DefWindowProc(hWnd, uMsg, wParam, lParam);
}
