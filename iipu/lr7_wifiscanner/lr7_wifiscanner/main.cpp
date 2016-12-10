#include "wifiScanner.hpp"

std::vector<HWND> createdWindows;
WifiScanner wifiScanner;
void timerTick(HWND hWnd) {
	int y = 0;
	std::vector<WifiInfo> wifiNetworks;

	wifiScanner.startScanning();
	while (wifiScanner.isScanning());
	wifiNetworks = wifiScanner.getLastScanResults();
	for (int i = 0; i < createdWindows.size(); ++i) {
		DestroyWindow(createdWindows[i]);
	}
	Sleep(1000);
	createdWindows.clear();
	for (int i = 0, y = 0; i < wifiNetworks.size(); ++i, y += 50) {
		if (_tcscmp(wifiNetworks[i].name.c_str(), TEXT("")) == 0) {
			wifiNetworks[i].name = TEXT("UNKNOWN");
		}
		createdWindows.push_back(CreateWindow(TEXT("STATIC"), wifiNetworks[i].name.c_str(), WS_CHILD | WS_VISIBLE, 0, y, 150, 40, hWnd, HMENU(i), NULL, NULL));
		createdWindows.push_back(CreateWindow(TEXT("STATIC"), wifiNetworks[i].auth.c_str(), WS_CHILD | WS_VISIBLE, 150, y, 150, 40, hWnd, HMENU(i + 1), NULL, NULL));
		createdWindows.push_back(CreateWindow(TEXT("STATIC"), wifiNetworks[i].macAddress.c_str(), WS_CHILD | WS_VISIBLE, 300, y, 300, 40, hWnd, HMENU(i + 2), NULL, NULL));
		createdWindows.push_back(CreateWindow(TEXT("STATIC"), wifiNetworks[i].quality.c_str(), WS_CHILD | WS_VISIBLE, 600, y, 300, 40, hWnd, HMENU(i + 3), NULL, NULL));
	}
}

LRESULT CALLBACK WindowProc(HWND, UINT, WPARAM, LPARAM);
int APIENTRY WinMain(HINSTANCE hInstance,
	HINSTANCE hPrevInstance,
	LPSTR lpCmdLine,
	int nCmdShow)
{
	HWND hwnd;
	MSG  msg;
	WNDCLASSW wc = { 0 };

	wc.lpszClassName = L"Application";
	wc.hInstance = hInstance;
	wc.hbrBackground = GetSysColorBrush(COLOR_3DFACE);
	wc.lpfnWndProc = WindowProc;
	wc.hCursor = LoadCursor(0, IDC_ARROW);


	RegisterClassW(&wc);
	hwnd = CreateWindowW(wc.lpszClassName, L"Wifi scanner",
		WS_OVERLAPPEDWINDOW | WS_VISIBLE | WS_CLIPCHILDREN,
		100, 100, 900, 450, 0, 0, hInstance, 0);
	ShowWindow(hwnd, SW_SHOW);
	while (GetMessage(&msg, NULL, 0, 0))
	{
		DispatchMessage(&msg);
		TranslateMessage(&msg);
	}
}


UINT timerID = 10;
LRESULT CALLBACK WindowProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
	switch (uMsg) {
	case WM_CREATE:
		timerTick(hWnd);
		SetTimer(hWnd, timerID, 10000, NULL);
		break;
	case WM_TIMER:
		timerTick(hWnd);
		break;
	case WM_DESTROY:
		KillTimer(hWnd, timerID);
		PostQuitMessage(0);
		break;
	}
	return DefWindowProc(hWnd, uMsg, wParam, lParam);
}

