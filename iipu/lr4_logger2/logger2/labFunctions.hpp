#pragma once
#include <windows.h>
#include <TlHelp32.h>
#include <ShObjIdl.h>
#include <tchar.h>
#include <Psapi.h>

typedef ITaskbarList *LPITaskbarList;
int hide;

void startProcess(TCHAR* name) {
	STARTUPINFO siForNotepad = { sizeof(siForNotepad) };
	PROCESS_INFORMATION piForNotepad;
	if (!CreateProcess(NULL, name, NULL, NULL, FALSE, 0, NULL, NULL, &siForNotepad, &piForNotepad))
	{
		return;
	}
}
void killProcess(const TCHAR* name) {
	HANDLE hSnapShot = CreateToolhelp32Snapshot(TH32CS_SNAPALL, NULL);
	PROCESSENTRY32 pEntry;
	pEntry.dwSize = sizeof(pEntry);
	BOOL hRes = Process32First(hSnapShot, &pEntry);
	while (hRes)
	{
		if (_tccmp(pEntry.szExeFile, name) == 0)
		{
			HANDLE hProcess = OpenProcess(PROCESS_TERMINATE, 0,
				(DWORD)pEntry.th32ProcessID);
			if (hProcess != NULL)
			{
				TerminateProcess(hProcess, 9);
				CloseHandle(hProcess);
			}
		}
		hRes = Process32Next(hSnapShot, &pEntry);
	}
	CloseHandle(hSnapShot);
}

HHOOK hookHideProcess;
HOOKPROC cbt;
HMODULE hideProcDll;


void hideApp(HWND hWnd) {
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
void showApp(HWND hWnd) {
	LPITaskbarList pTaskBar;
	CoInitialize(0);
	CoCreateInstance(CLSID_TaskbarList, 0, CLSCTX_INPROC_SERVER, IID_ITaskbarList, (void**)&pTaskBar);
	pTaskBar->HrInit();
	pTaskBar->AddTab(hWnd);
	ShowWindow(hWnd, SW_SHOW);
	UnhookWindowsHookEx(hookHideProcess);
}
void switchAppMode(HWND hWnd) {
	if (hide) {
		showApp(hWnd);
		hide = FALSE;
	}
	else {
		hideApp(hWnd);
		hide = TRUE;
	}
}
