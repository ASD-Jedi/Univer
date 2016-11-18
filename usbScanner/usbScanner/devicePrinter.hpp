#pragma once
#include "device.hpp"
#include <Windows.h>
#include "deviceRemover.hpp"

typedef struct DrawnDevice {
	HWND hWnd;
	TCHAR deviceLetter;
	int x;
	int y;
	int indexInVector;
} DrawnDevice;

#define WIDTH_NAME 400
#define HEIGHT_USB 75
#define MARGIN 10

//ћетоды не статические, потому что при наличии, например, двух окон вывода, можно назначить каждому экземппл€ру свой набор хэндлов
class DevicePrinter {
private:
	static UsbDevice usbDevice;
	HWND hGroup;
	static WNDCLASS rwc;
	static WNDCLASS wcMem;
	static std::vector<DrawnDevice> drawnDevices;
	int widthUsb = WIDTH_NAME;
	int heightUsb = HEIGHT_USB;
	int margin = MARGIN;
	static LRESULT CALLBACK MemoryDrawer(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
			HBRUSH hBrushYellow, holdBrush;
			HDC hdc;
			PAINTSTRUCT ps;
			RECT rect;
			HPEN hPen, holdPen;
			int right;
			int bottom = HEIGHT_USB/3;
			TCHAR t[MAX_PATH];
			double busyGb, allGb;
		switch (uMsg)
		{
		case WM_PAINT:
			if (numUsb >= usbDevice.getDevices().size())
				break;
			right = WIDTH_NAME * ((double)usbDevice.getDevice(numUsb).busyMemory / usbDevice.getDevice(numUsb).allMemory);
			hdc = BeginPaint(hWnd, &ps);
			hBrushYellow = CreateSolidBrush(RGB(255, 255, 184));
			holdBrush = (HBRUSH)SelectObject(hdc, hBrushYellow);
			Rectangle(hdc, 0,0,right, bottom);
			rect.left = 0;
			rect.top = 0;
			rect.right = 120;
			rect.bottom = 25;
			busyGb = (double)usbDevice.getDevice(numUsb).busyMemory / 1024 / 1024 / 1024;
			allGb = (double)usbDevice.getDevice(numUsb).allMemory / 1024 / 1024 / 1024;
			_stprintf_s(t, TEXT("%.2fgb of %.2fgb"), busyGb, allGb);
			
			SetBkMode(hdc, TRANSPARENT);
			DrawText(hdc, t, _tcslen(t), &rect, DT_CENTER | DT_VCENTER);
			EndPaint(hWnd, &ps);
			SelectObject(hdc, holdBrush);
			DeleteObject(hBrushYellow);
			++numUsb;

			break;
		default:
			return DefWindowProc(hWnd, uMsg, wParam, lParam);
		}
		return DefWindowProc(hWnd, uMsg, wParam, lParam);
	}
	static LRESULT CALLBACK UsbWindowProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
		switch (uMsg) {
		case WM_CREATE:
			CreateWindow(TEXT("Static"), tempUsb.volumeName, WS_CHILD | WS_VISIBLE, 0, 0, WIDTH_NAME, HEIGHT_USB/3, hWnd, (HMENU)50, NULL, NULL);
			CreateWindow(TEXT("Static"), tempUsb.volumeLetter, WS_CHILD | WS_VISIBLE, 0, HEIGHT_USB/3, WIDTH_NAME, HEIGHT_USB / 3, hWnd, (HMENU)52, NULL, NULL);
			CreateWindowEx(WS_EX_STATICEDGE, TEXT("Memory"), NULL, WS_CHILD | WS_VISIBLE, 0, HEIGHT_USB / 3 * 2, WIDTH_NAME, HEIGHT_USB / 3, hWnd, (HMENU)51, NULL, NULL);
			break;
		case WM_RBUTTONUP:
			UsbRemover remover;
			for (int i = 0; i < drawnDevices.size(); ++i) {
				if (drawnDevices[i].hWnd == hWnd) {
					BOOL res = remover.EjectVolume(drawnDevices[i].deviceLetter);
					MessageBox(NULL, remover.GetLastMessage(), L"Info", MB_OK);
					SendMessage(GetParent(hWnd), WM_DEVICECHANGE, 0, 0);
				}
			}
			break;
		}
		return DefWindowProc(hWnd, uMsg, wParam, lParam);
	}
	static UsbDeviceDescription tempUsb;
	static int numUsb;
public:
	DevicePrinter(HWND hGroup, HINSTANCE hInst) {
		this->hGroup = hGroup;

		rwc = { 0 };
		rwc.lpszClassName = TEXT("UsbPrinter");
		rwc.lpfnWndProc = UsbWindowProc;
		rwc.hCursor = LoadCursor(0, IDC_ARROW);
		rwc.hbrBackground = GetSysColorBrush(COLOR_BTNFACE);
		rwc.style = CS_HREDRAW;
		RegisterClass(&rwc);

		wcMem = { 0 };
		wcMem.lpszClassName = TEXT("Memory");
		wcMem.lpfnWndProc = MemoryDrawer;
		wcMem.hCursor = LoadCursor(0, IDC_ARROW);
		wcMem.hbrBackground = GetSysColorBrush(COLOR_BTNFACE);
		wcMem.style = CS_HREDRAW;
		RegisterClass(&wcMem);
	}
	HWND printDevice(UsbDevice* device, int num) {
		RECT rect = { 0 };
		GetClientRect(hGroup, &rect);
		int width = rect.right - rect.left;
		int height = rect.bottom - rect.top;
		int colAmount = width / widthUsb;
		int x = (num % colAmount)*(width + margin);
		int y = (num / colAmount)*(HEIGHT_USB + margin);
		tempUsb = device->getDevice(num);
		HWND hNewDevice = CreateWindowEx(WS_EX_STATICEDGE, TEXT("UsbPrinter"), NULL, WS_CHILD | WS_VISIBLE | WS_BORDER | WS_CLIPCHILDREN, x, y, width, height, hGroup, HMENU(num), NULL, NULL);
		DrawnDevice drawnDevice = { hNewDevice, device->getDevice(num).volumeLetter[0], x, y, num };
		drawnDevices.push_back(drawnDevice);
		return hNewDevice;
	}
	void printDevices(UsbDevice* device) {
		std::vector<UsbDeviceDescription> devices = device->getDevices();
		usbDevice = *device;
		numUsb = 0;
		for (int i = 0; i < drawnDevices.size(); ++i) {
			DestroyWindow(drawnDevices[i].hWnd);
		}
		for (int i = 0; i < devices.size(); ++i) {
			printDevice(device, i);
		}
	}
};

WNDCLASS DevicePrinter::rwc;
WNDCLASS DevicePrinter::wcMem;
UsbDeviceDescription DevicePrinter::tempUsb;
UsbDevice DevicePrinter::usbDevice;
int DevicePrinter::numUsb = 0;
std::vector<DrawnDevice> DevicePrinter::drawnDevices;