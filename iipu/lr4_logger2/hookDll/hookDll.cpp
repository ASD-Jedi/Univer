#include "hookDll.h"
HINSTANCE hInstance = NULL;
LRESULT CALLBACK KeyboardMsgProc(int, WPARAM, LPARAM);
LRESULT CALLBACK MouseMsgProc(int, WPARAM, LPARAM);
LRESULT CALLBACK HideKeysProc(int, WPARAM, LPARAM);
LRESULT CALLBACK EmulateKeysProc(int, WPARAM, LPARAM);
#pragma data_seg(".SData") 
std::vector<TCHAR> hideKeys;
std::vector<EmulateKey> emulateKeys;
int hideHookFlag;
int emulateHookFlag;
HHOOK hKeyboardMsgHook = NULL;
HHOOK hMouseMsgHook = NULL;
HHOOK hHideHook = NULL;
HHOOK hEmulateHook = NULL;
UINT KBoardMessage = NULL; //keyboard message
UINT MBoardMessage = NULL; //mouse message
HWND hParentWnd = NULL;
int counter = 0;
#pragma data_seg

#pragma comment(linker, "/SECTION:.SData,RWS")

void messageBoxError() {
	TCHAR str[10] = TEXT("error");
	MessageBox(NULL, str, TEXT("Error code"), MB_OK | MB_ICONERROR);
}

void messageBoxInt(DWORD a, TCHAR* caption) {
	TCHAR str[10] = TEXT("Sorry");
	
	MessageBox(NULL, str, caption, MB_OK | MB_ICONINFORMATION);
}

BOOL APIENTRY DllMain(HANDLE hModule, DWORD reason, LPVOID lpReserved) {
	if (reason == DLL_PROCESS_ATTACH) {
		hInstance = (HINSTANCE)hModule;
	}
	return TRUE;
}

MYHOOKDLL_API int SetHook(HWND hWnd, UINT keyMsg, UINT mouseMsg) 
{
	if (hWnd == NULL) return -1;

	// Save received parameters
	hParentWnd = hWnd;
	KBoardMessage = keyMsg;
	MBoardMessage = mouseMsg;
	// Set hook
	hKeyboardMsgHook = ::SetWindowsHookEx(WH_KEYBOARD_LL, KeyboardMsgProc, hInstance, 0);
	hMouseMsgHook = ::SetWindowsHookEx(WH_MOUSE_LL, MouseMsgProc, hInstance, 0);
	if (hMouseMsgHook == NULL) {
		messageBoxError();
	}
	// If we are failed...
	if (hKeyboardMsgHook == NULL || hMouseMsgHook == NULL)
		return -1;

	return 0;
};

MYHOOKDLL_API int UnSetHook()
{
	UnhookWindowsHookEx(hKeyboardMsgHook);
	UnhookWindowsHookEx(hMouseMsgHook);
	hKeyboardMsgHook = NULL;
	hMouseMsgHook = NULL;
	return 0;
};
MYHOOKDLL_API int SetHideKeys(std::vector<TCHAR> hides) {
	hideKeys = hides;
	ODPRINTF(("Before hide set"));
	hHideHook = SetWindowsHookEx(WH_KEYBOARD_LL, HideKeysProc, hInstance, 0);
	if (hHideHook == NULL) {
		return -1;
	}
	return 0;
}
MYHOOKDLL_API int SetEmulateKeys(std::vector<EmulateKey> emulates) {
	emulateKeys = emulates;
	hEmulateHook = SetWindowsHookEx(WH_KEYBOARD_LL, EmulateKeysProc, hInstance, 0);
	if (hEmulateHook == NULL) {
		return -1;
	}
	return 0;
}
MYHOOKDLL_API int UnSetHideKeys() {
	UnhookWindowsHookEx(hHideHook);
	hHideHook = NULL;
	return 0;
}
MYHOOKDLL_API int UnSetEmulateKeys() {
	UnhookWindowsHookEx(hEmulateHook);
	hEmulateHook = NULL;
	return 0;
}
LRESULT CALLBACK KeyboardMsgProc(int code, WPARAM wParam, LPARAM lParam)
{
	if (code >= 0)
	{
		if (wParam == WM_KEYDOWN)
		{
			int message, msgwParam, msglParam;
			KBDLLHOOKSTRUCT* key = (KBDLLHOOKSTRUCT*)lParam;
			switch (key->vkCode) {	
				default:
					message = KBoardMessage;
					msglParam = key->scanCode;
					msgwParam = key->vkCode;
					break;

			}
			PostMessage(hParentWnd, message, msgwParam, msglParam);
		}
		return CallNextHookEx(hKeyboardMsgHook, code, wParam, lParam);
	}
	return CallNextHookEx(hKeyboardMsgHook, code, wParam, lParam);

};

LRESULT CALLBACK MouseMsgProc(int code, WPARAM wParam, LPARAM lParam) {
	if (code >= 0)
	{
		
		int message = MBoardMessage, msgwParam = 0, msglParam = 0;
		switch (wParam) {
			case WM_LBUTTONUP:
				msgwParam = WM_LBUTTONUP;
				break;
			case WM_RBUTTONUP:
				msgwParam = WM_RBUTTONUP;
				break;
			default:
				msgwParam = 0;
				break;
		}
		if (msgwParam) {
			PostMessage(hParentWnd, message, msgwParam, msglParam);
		}
	}

	return CallNextHookEx(hMouseMsgHook, code, wParam, lParam);
}

LRESULT CALLBACK HideKeysProc(int code, WPARAM wParam, LPARAM lParam) {
	if (code >= 0)
	{
		ODPRINTF(("HIDE"));
		KBDLLHOOKSTRUCT* key = (KBDLLHOOKSTRUCT*)lParam;
		TCHAR t = (TCHAR)key->vkCode;
		for (int i = 0; i < hideKeys.size(); ++i) {
			if (hideKeys[i] == t) {
				return 1;
			}
		}
	}
	return CallNextHookEx(hKeyboardMsgHook, code, wParam, lParam);
}

LRESULT CALLBACK EmulateKeysProc(int code, WPARAM wParam, LPARAM lParam) {
	if (code >= 0)
	{
		ODPRINTF(("EMULATE"));
		TCHAR t = (TCHAR)((KBDLLHOOKSTRUCT*)lParam)->vkCode;
		for (int i = 0; i < emulateKeys.size(); ++i) {
			if (emulateKeys[i].key == t) {
				if (wParam == WM_KEYDOWN) {
					INPUT ip;
					ip.type = INPUT_KEYBOARD;
					ip.ki.wScan = 0;
					ip.ki.time = 0;
					ip.ki.dwExtraInfo = 0;
					HKL currkeyboard = GetKeyboardLayout(0);
					ip.ki.dwFlags = 0; //Key press
					ip.ki.wVk = emulateKeys[i].newKey;
					SendInput(1, &ip, sizeof(INPUT));
					ip.ki.dwFlags = KEYEVENTF_KEYUP; //key release
					SendInput(1, &ip, sizeof(INPUT));
				}
				return 1;
			}
		}
	}
	return CallNextHookEx(hKeyboardMsgHook, code, wParam, lParam);
}