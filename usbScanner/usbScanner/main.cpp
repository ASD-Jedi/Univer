#include <windows.h>
#include "device.hpp"
#include "resource.h"
#include "devicePrinter.hpp"
#include <Dbt.h>
#include <CommCtrl.h>

LRESULT CALLBACK WindowProc(HWND, UINT, WPARAM, LPARAM);
BOOL DoRegisterDeviceInterfaceToHwnd(
	IN GUID InterfaceClassGuid,
	IN HWND hWnd,
	OUT HDEVNOTIFY *hDeviceNotify
);
UsbDevice usbDevice;
DevicePrinter usbPrinter(NULL, NULL);
HINSTANCE globalInstance;
int APIENTRY WinMain(HINSTANCE hInstance,
	HINSTANCE hPrevInstance,
	LPSTR lpCmdLine,
	int nCmdShow)
{
	globalInstance = hInstance;
	HWND hwnd;
	MSG  msg;
	WNDCLASSW wc = { 0 };

	wc.lpszClassName = L"Application";
	wc.hInstance = hInstance;
	wc.hbrBackground = GetSysColorBrush(COLOR_3DFACE);
	wc.lpfnWndProc = WindowProc;
	wc.hCursor = LoadCursor(0, IDC_ARROW);


	RegisterClassW(&wc);
	hwnd = CreateWindowW(wc.lpszClassName, L"Usb devices",
		WS_OVERLAPPEDWINDOW | WS_VISIBLE | WS_CLIPCHILDREN,
		100, 100, 600, 250, 0, 0, hInstance, 0);
	ShowWindow(hwnd, SW_SHOW);
	while (GetMessage(&msg, NULL, 0, 0))
	{
		DispatchMessage(&msg);
		TranslateMessage(&msg);
	}
	int a;
	a = 9;
}

LRESULT CALLBACK WindowProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
	static HDEVNOTIFY hDeviceNotify;
	BOOL a;
	static GUID WceusbshGUID = { 0x25dbce51, 0x6c8f, 0x4a72,
		0x8a,0x6d,0xb5,0x4c,0x2b,0x4f,0xc8,0x35 };
	switch (uMsg) {
	case WM_CREATE:
		usbPrinter = DevicePrinter(hWnd, globalInstance);
		usbPrinter.printDevices(&usbDevice);
		a = DoRegisterDeviceInterfaceToHwnd(WceusbshGUID, hWnd, &hDeviceNotify);
		break;
	case WM_DEVICECHANGE:
		usbPrinter.printDevices(&usbDevice);
		break;
	case WM_DESTROY:
		PostQuitMessage(0);
		break;
	}
	return DefWindowProc(hWnd, uMsg, wParam, lParam);
}

BOOL DoRegisterDeviceInterfaceToHwnd(
	IN GUID InterfaceClassGuid,
	IN HWND hWnd,
	OUT HDEVNOTIFY *hDeviceNotify
)
{
	DEV_BROADCAST_DEVICEINTERFACE NotificationFilter;

	ZeroMemory(&NotificationFilter, sizeof(NotificationFilter));
	NotificationFilter.dbcc_size = sizeof(DEV_BROADCAST_DEVICEINTERFACE);
	NotificationFilter.dbcc_devicetype = DBT_DEVTYP_DEVICEINTERFACE;
	NotificationFilter.dbcc_classguid = InterfaceClassGuid;

	*hDeviceNotify = RegisterDeviceNotification(
		hWnd,                       // events recipient
		&NotificationFilter,        // type of device
		DEVICE_NOTIFY_WINDOW_HANDLE // type of recipient handle
	);

	if (NULL == *hDeviceNotify)
	{
		return FALSE;
	}

	return TRUE;
}