// dllmain.cpp : Defines the entry point for the DLL application.
#include "stdafx.h"

DWORD WINAPI Hook_thread(LPVOID);

BOOL APIENTRY DllMain( HMODULE hModule,
                       DWORD  ul_reason_for_call,
                       LPVOID lpReserved
					 )
{
	switch (ul_reason_for_call)
	{
	case DLL_PROCESS_ATTACH:
		CreateThread(NULL,NULL,Hook_thread, NULL, NULL, NULL);
	case DLL_THREAD_ATTACH:
	case DLL_THREAD_DETACH:
	case DLL_PROCESS_DETACH:
		break;
	}
	return TRUE;
}

int WINAPI HookedMessageBox(
	_In_opt_ HWND hWnd,
	_In_opt_ LPCSTR lpText,
	_In_opt_ LPCSTR lpCaption,
	_In_ UINT uType
)
{
	return MessageBox(hWnd, L"Hooked", L"Hooked", MB_OK | MB_ICONERROR);
}
DWORD WINAPI Hook_thread(LPVOID) {
	DWORD hooked_messagbox = (DWORD)HookedMessageBox;
	DWORD call_address = 0x012D116A;
	::ExitThread(1337);
}