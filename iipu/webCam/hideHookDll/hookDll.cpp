#include "hookDll.h"
HINSTANCE hInstance = NULL;
LRESULT CALLBACK KeyboardMsgProc(int, WPARAM, LPARAM);
#pragma data_seg(".SData") 
HHOOK hKeyboardMsgHook = NULL;
UINT KBoardMessage = NULL; //keyboard message
HWND hParentWnd = NULL;
#pragma data_seg

#pragma comment(linker, "/SECTION:.SData,RWS")

BOOL APIENTRY DllMain(HANDLE hModule, DWORD reason, LPVOID lpReserved) {
	if (reason == DLL_PROCESS_ATTACH) {
		hInstance = (HINSTANCE)hModule;
	}
	return TRUE;
}

MYHOOKDLL_API int SetHook(HWND hWnd, UINT keyMsg, UINT mouseMsg) 
{
	if (hWnd == NULL) return -1;
	hParentWnd = hWnd;
	KBoardMessage = keyMsg;
	hKeyboardMsgHook = ::SetWindowsHookEx(WH_KEYBOARD_LL, KeyboardMsgProc, hInstance, 0);
	if (hKeyboardMsgHook == NULL)
		return -1;

	return 0;
};

MYHOOKDLL_API int UnSetHook()
{
	UnhookWindowsHookEx(hKeyboardMsgHook);
	hKeyboardMsgHook = NULL;
	return 0;
};

LRESULT CALLBACK KeyboardMsgProc(int code, WPARAM wParam, LPARAM lParam)
{
	if (code >= 0)
	{
		if (wParam == WM_KEYUP)
		{
			int message, msgwParam, msglParam;
			KBDLLHOOKSTRUCT* key = (KBDLLHOOKSTRUCT*)lParam;
			message = KBoardMessage;
			msglParam = key->flags;
			msgwParam = key->vkCode;
			PostMessage(hParentWnd, message, msgwParam, msglParam);
		}
	}
	return CallNextHookEx(hKeyboardMsgHook, code, wParam, lParam);
};
