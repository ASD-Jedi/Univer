#include <Windows.h>
#include <iostream>
#include <conio.h>
#include <highlevelmonitorconfigurationapi.h>
#include <powrprof.h>

#pragma comment (lib, "dxva2.lib")

using namespace std;

int isBatteryExist() {
	SYSTEM_POWER_STATUS status = { 0 };
	int res = GetSystemPowerStatus(&status);
	if (!res) {
		cout << "Error GetSystemPowerStatus() : ErrorCode - " << GetLastError() << endl;
		return 0;
	}
	if (status.BatteryFlag == 128) {
		cout << "ystemBattery doesn't exist" << endl;
		return 0;
	}
	return 1;
}



void changeGamma(WORD wBrightness) {
	WORD  GammaArray[3][256];
	HDC   hGammaDC = ::GetDC(NULL);
	GetDeviceGammaRamp(hGammaDC, GammaArray);
	for (int ik = 0; ik < 256; ik++) {
		int iArrayValue = ik * (wBrightness + 128);
		if (iArrayValue > 0xffff) iArrayValue = 0xffff;
		GammaArray[0][ik] = (WORD)iArrayValue;
		GammaArray[1][ik] = (WORD)iArrayValue;
		GammaArray[2][ik] = (WORD)iArrayValue;
	}
	SetDeviceGammaRamp(hGammaDC, GammaArray);
}

void shutdownScreen(DWORD shutdownTime) {
	//128 to 0 for time; I have fixed steps amount, so I need count the delay, delay = time/steps
	int delay = shutdownTime / 16; //128 max brightness, 0 min brghtness, from 128 to 8 in 16 steps by 8 point per step
	for (int i = 128; i > 0; i-=8) {
		changeGamma(i);
		Sleep(delay);
	}

	WORD  GammaArray[3][256];
	HDC   hGammaDC = ::GetDC(NULL);
	WORD  wBrightness;


	wBrightness = 128;     // reduce the brightness
	Sleep(3000);
	wBrightness = 128;    // set the brightness back to normal
	for (int ik = 0; ik < 256; ik++) {
		int iArrayValue = ik * (wBrightness + 128);
		if (iArrayValue > 0xffff) iArrayValue = 0xffff;
		GammaArray[0][ik] = (WORD)iArrayValue;
		GammaArray[1][ik] = (WORD)iArrayValue;
		GammaArray[2][ik] = (WORD)iArrayValue;
	}

	::SetDeviceGammaRamp(hGammaDC, GammaArray);
	//SendMessage(HWND_BROADCAST, WM_SYSCOMMAND, SC_MONITORPOWER, (LPARAM)2);
}

void turnoffScreen() {
	SendMessage(HWND_BROADCAST, WM_SYSCOMMAND, SC_MONITORPOWER, (LPARAM)2);
}

void turnonScreen() {
	SendMessage(HWND_BROADCAST, WM_SYSCOMMAND, SC_MONITORPOWER, (LPARAM)-1);
}

int a = 12;
HHOOK hkKeyboard;
HHOOK hkMouse;

void setA() {
	a = 0;
}

int main() {
	string powertype;

	double r = MathLibrary::Functions::Add(2, 5);
	hkKeyboard = SetWindowsHookEx(WH_KEYBOARD, &MathLibrary::Functions::Zero, NULL, GetCurrentThreadId());
	hkMouse = SetWindowsHookEx(WH_MOUSE, &MathLibrary::Functions::Zero, NULL, GetCurrentThreadId());
	cout << GetLastError() << endl;
	getchar();
	cout << a;
	getchar();
	UnhookWindowsHookEx(hkKeyboard);
	UnhookWindowsHookEx(hkMouse);

	return 0;
	int sleepTime = 5;
	cout << "Enter sleep time in seconds: ";
	cin >> sleepTime;
	int monitorState = 2;
	while (a<100) {
		SYSTEM_POWER_STATUS status = { 0 };
		int res = GetSystemPowerStatus(&status);
		if (status.ACLineStatus == 0) {
			cout << "Mode: Battery"<<endl;
		}
		else {
			cout << "Mode: AC" << endl;
		}
		if (!res) {
			cout << "Error GetSystemPowerStatus() : ErrorCode - " << GetLastError()<<endl;
			break;
		}
		if (a % sleepTime == 0 && status.ACLineStatus == 0) {
			turnoffScreen();
		}
		
		DWORD percent = status.BatteryLifePercent;
		cout << "Charge level: " << percent  << "%" << endl;
		if (status.ACLineStatus == 0) {
			cout << "Time left: " << status.BatteryLifeTime << " seconds" << endl;
		}
		cout << a++;
		Sleep(1000);
		system("cls");
	}

	cout << endl << endl;
	return 0;
}