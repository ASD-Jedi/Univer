#define MYHOOKDLL_API __declspec(dllexport)

#include <Windows.h>


extern "C" {
	MYHOOKDLL_API int SetHook(HWND, UINT, UINT);
	MYHOOKDLL_API int UnSetHook();
}