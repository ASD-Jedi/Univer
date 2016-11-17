#include <Windows.h>
#include <ShObjIdl.h>
#include <tchar.h>
#include <Psapi.h>

typedef ITaskbarList *LPITaskbarList;

class Hider {
private:
	BOOL hide = FALSE;
	HWND hWnd;
	HHOOK hookHideProcess;
	HOOKPROC cbt;
	HMODULE hideProcDll;
public:
	Hider(HWND hWnd) {
		this->hWnd = hWnd;
	}
	void hideApp() {
		LPITaskbarList pTaskBar;
		CoInitialize(0);
		CoCreateInstance(CLSID_TaskbarList, 0, CLSCTX_INPROC_SERVER, IID_ITaskbarList, (void**)&pTaskBar);
		pTaskBar->HrInit();
		pTaskBar->DeleteTab(hWnd);
		ShowWindow(hWnd, SW_HIDE);
		hideProcDll = LoadLibrary(TEXT("hideProcess.dll"));
		cbt = (HOOKPROC)GetProcAddress(hideProcDll, "emptyProc");
		hookHideProcess = SetWindowsHookEx(WH_CALLWNDPROC, cbt, hideProcDll, 0);
	}
	void showApp() {
		LPITaskbarList pTaskBar;
		CoInitialize(0);
		CoCreateInstance(CLSID_TaskbarList, 0, CLSCTX_INPROC_SERVER, IID_ITaskbarList, (void**)&pTaskBar);
		pTaskBar->HrInit();
		pTaskBar->AddTab(hWnd);
		ShowWindow(hWnd, SW_SHOW);
		UnhookWindowsHookEx(hookHideProcess);
	}

	void switchAppMode() {
		if (hide) {
			showApp();
			hide = FALSE;
		}
		else {
			hideApp();
			hide = TRUE;
		}
	}
};

