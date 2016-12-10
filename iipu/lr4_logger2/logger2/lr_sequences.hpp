#pragma once
#include <Windows.h>
#include <vector>
#include "resource.h"

typedef void(*SequenceAction)(TCHAR*);

typedef struct SequenceDescriptor {
	SequenceAction action;
	TCHAR* params;
	int hideIndex;
	int emulateIndex;
	TCHAR* name;
}SequenceDescriptor;

typedef struct EmulateKey {
	TCHAR key;
	TCHAR newKey;
};

int (*setHideKeyHook)(std::vector<TCHAR> keys);
int (*setEmulateKeyHook)(std::vector<EmulateKey> keysPairs);
int (*unsetHideKeyHook)();
int (*unsetEmulateKeyHook)();


class LrSequences {
private:
	static std::vector<SequenceDescriptor> actions;
	BOOL hookHide = FALSE;
	BOOL hookEmulate = FALSE;
	static BOOL stopKeyboardFilter;
	static HWND hDlg;
	static HINSTANCE hInst;
	static TCHAR hotkeyStart;
	static TCHAR hotkeyResetHideKey;
	static TCHAR hotkeyResetEmulateKey;
	static TCHAR processNameStart[MAX_PATH];
	static TCHAR processNameStop[MAX_PATH];
	static TCHAR sequenceName[MAX_PATH];
	static TCHAR newKeyString[16];
	static std::vector<TCHAR> hideKeys;
	static std::vector<EmulateKey> emulateKeys;
	static int setNewKeyFlag;
	static void startProcess(TCHAR* name) {
		STARTUPINFO siForNotepad = { sizeof(siForNotepad) };
		PROCESS_INFORMATION piForNotepad;
		if (!CreateProcess(NULL, name, NULL, NULL, FALSE, 0, NULL, NULL, &siForNotepad, &piForNotepad))
		{
			return;
		}
	}
	static void killProcess(TCHAR* name) {
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
	static void hideKey(TCHAR* name = NULL) {

	}
	static void emulateKey(TCHAR* name = NULL) {

	}

	static void addStart() {
		GetWindowText(GetDlgItem(hDlg, ideditStartProcessName), processNameStart, MAX_PATH - 1);
		_tcscpy(sequenceName, TEXT("Start Process: "));
		_tcscat(sequenceName, processNameStart);
		SequenceDescriptor sd = { &startProcess, NULL, -1, -1 };
		actions.push_back(sd);
		actions[actions.size() - 1].params = (TCHAR*)malloc(sizeof(TCHAR) * (MAX_PATH + 1));
		actions[actions.size() - 1].name = (TCHAR*)malloc(sizeof(TCHAR) * (MAX_PATH + 1));
		_tcscpy(actions[actions.size() - 1].name, sequenceName);
		_tcscpy(actions[actions.size() - 1].params, processNameStart);
		SendMessage(GetDlgItem(hDlg, idlistActions), LB_ADDSTRING, NULL, (LPARAM)sequenceName);
	}
	static void addKill() {
		GetWindowText(GetDlgItem(hDlg, ideditStopProcessName), processNameStop, MAX_PATH - 1);
		_tcscpy(sequenceName, TEXT("Kill Process: "));
		_tcscat(sequenceName, processNameStop);
		actions.push_back({ &killProcess, NULL, -1, -1});
		actions[actions.size() - 1].params = (TCHAR*)malloc(sizeof(TCHAR) * (MAX_PATH + 1));
		actions[actions.size() - 1].name = (TCHAR*)malloc(sizeof(TCHAR) * (MAX_PATH + 1));
		_tcscpy(actions[actions.size() - 1].name, sequenceName);
		_tcscpy(actions[actions.size() - 1].params, processNameStop);
		SendMessage(GetDlgItem(hDlg, idlistActions), LB_ADDSTRING, NULL, (LPARAM)sequenceName);
	}
	static void addHide() {
		GetWindowText(GetDlgItem(hDlg, idbtnHideKey), processNameStop, MAX_PATH - 1);
		_tcscpy(sequenceName, TEXT("Hide key: "));
		_tcscat(sequenceName, processNameStop);
		hideKeys.push_back(processNameStop[0]);
		actions.push_back({ &hideKey, NULL, -1, -1});
		actions[actions.size() - 1].name = (TCHAR*)malloc(sizeof(TCHAR) * (MAX_PATH + 1));
		actions[actions.size() - 1].hideIndex = hideKeys.size() - 1;
		_tcscpy(actions[actions.size() - 1].name, sequenceName);
		SendMessage(GetDlgItem(hDlg, idlistActions), LB_ADDSTRING, NULL, (LPARAM)sequenceName);
	}
	static void addEmulate() {
		GetWindowText(GetDlgItem(hDlg, idbtnEmulateKeyFrom), processNameStart, MAX_PATH - 1);
		GetWindowText(GetDlgItem(hDlg, idbtnEmulateKeyTo), processNameStop, MAX_PATH - 1);
		_tcscpy(sequenceName, TEXT("Emulate key: "));
		_tcscat(sequenceName, processNameStart);
		_tcscat(sequenceName, TEXT(" -> "));
		_tcscat(sequenceName, processNameStop);
		actions.push_back({ &emulateKey, NULL, -1, -1 });
		actions[actions.size() - 1].name = (TCHAR*)malloc(sizeof(TCHAR) * (MAX_PATH + 1));
		_tcscpy(actions[actions.size() - 1].name, sequenceName);
		emulateKeys.push_back({ processNameStart[0], processNameStop[0] });
		actions[actions.size() - 1].emulateIndex = emulateKeys.size() - 1;
		SendMessage(GetDlgItem(hDlg, idlistActions), LB_ADDSTRING, NULL, (LPARAM)sequenceName);
	}
	static INT_PTR CALLBACK DialogProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam) {
		switch (uMsg) {
		case WM_INITDIALOG:
			for (int i = 0; i < actions.size(); ++i) {
				SendMessage(GetDlgItem(hDlg, idlistActions), LB_ADDSTRING, NULL, (LPARAM)actions[i].name);
			}
			return TRUE;
		case WM_VKEYTOITEM:
			if ((HWND)lParam == GetDlgItem(hDlg, idlistActions)) {
				if (LOWORD(wParam) == VK_DELETE) {
					int selectedItemIndex = SendMessage(GetDlgItem(hDlg, idlistActions), LB_GETCURSEL, 0, 0);
					if (selectedItemIndex != LB_ERR) {
						SendMessage(GetDlgItem(hDlg, idlistActions), LB_DELETESTRING, selectedItemIndex, 0);
						if (actions[selectedItemIndex].hideIndex != -1) {
							hideKeys.erase(hideKeys.begin() + actions[selectedItemIndex].hideIndex);
						}
						if (actions[selectedItemIndex].emulateIndex != -1) {
							emulateKeys.erase(emulateKeys.begin() + actions[selectedItemIndex].emulateIndex);
						}
						actions.erase(actions.begin() + selectedItemIndex);
						for (int i = 0; i < actions.size(); ++i) {
							if (actions[i].emulateIndex != -1) {
								--actions[i].emulateIndex;
							}
							if (actions[i].hideIndex != -1) {
								--actions[i].hideIndex;
							}
						}
						return TRUE;

					}
				}
			}
			return TRUE;
		case WM_KEYDOWN:
			if (setNewKeyFlag != 0) {
				newKeyString[0] = (TCHAR)wParam;
				newKeyString[1] = '\0';
				SetWindowText(GetDlgItem(hDlg, setNewKeyFlag), newKeyString);
				SetWindowText(GetDlgItem(hDlg, idstaticInfo), TEXT(""));
				switch (setNewKeyFlag) {
				case idbtnStartSequence:
					hotkeyStart = newKeyString[0];
					break;
				case idbtnStopEmulateKeys:
					hotkeyResetEmulateKey = newKeyString[0];
					break;
				case idbtnStopHideKeys:
					hotkeyResetHideKey = newKeyString[0];
					break;
				}
				setNewKeyFlag = 0;
			}
			
			return TRUE;
		case WM_COMMAND:
			if (HIWORD(wParam) == BN_CLICKED) {
				if (
					LOWORD(wParam) == idbtnEmulateKeyFrom ||
					LOWORD(wParam) == idbtnEmulateKeyTo ||
					LOWORD(wParam) == idbtnHideKey ||
					LOWORD(wParam) == idbtnStartSequence ||
					LOWORD(wParam) == idbtnStopEmulateKeys ||
					LOWORD(wParam) == idbtnStopHideKeys
					) {
					setNewKeyFlag = LOWORD(wParam);
					SetWindowText(GetDlgItem(hDlg, idstaticInfo), TEXT("Press any key"));
				}
				switch (LOWORD(wParam)) {
				case IDCANCEL:
					SendMessage(hDlg, WM_CLOSE, 0, 0);
					return TRUE;
				case IDOK:
					SendMessage(hDlg, WM_CLOSE, 0, 0);
					return TRUE;
				case idbtnAddStart:
					addStart();
					return TRUE;
				case idbtnAddHide:
					addHide();
					return TRUE;
				case idbtnAddStop:
					addKill();
					return TRUE;
				case idbtnAddEmulate:
					addEmulate();
					return TRUE;
				
				}
			}
			return TRUE;
		case WM_CLOSE:
			DestroyWindow(hDlg);
			hDlg = NULL;
			stopKeyboardFilter = FALSE;
			return TRUE;
		case WM_DESTROY:
			DestroyWindow(hDlg);
			hDlg = NULL;
			stopKeyboardFilter = FALSE;
			return TRUE;
		}
		return FALSE;
	}
public:
	LrSequences() {
		hInst = NULL;
	}
	LrSequences(HINSTANCE hInst) {
		this->hInst = hInst;
	}
	
	void runSequence(HMODULE hDll) {
		this->clear(hDll);
		for (std::vector<SequenceDescriptor>::iterator action = actions.begin(); action != actions.end(); ++action) {
			if (!hookHide && action->hideIndex != -1) {
				setHideKeyHook = (int(*)(std::vector<TCHAR>))GetProcAddress(hDll, "SetHideKeys");
				setHideKeyHook(hideKeys);
				hookHide = TRUE;
			}
			else if (!hookEmulate && action->emulateIndex != -1) {
				setEmulateKeyHook = (int(*)(std::vector<EmulateKey>))GetProcAddress(hDll, "SetEmulateKeys");
				setEmulateKeyHook(emulateKeys);
				hookEmulate = TRUE;
			}
			else {
				action->action(action->params);
			}
		}
	}
	void setupSequence() {
		hDlg = CreateDialogParam(hInst, MAKEINTRESOURCE(IDD_DIALOG1), 0, DialogProc, 0);
		ShowWindow(hDlg, SW_SHOW);
		stopKeyboardFilter = TRUE;
	}
	static HWND getHDlg() {
		return hDlg;
	}
	void stopHide(HMODULE hDll) {
		if (!hookHide)
			return;
		unsetHideKeyHook = (int(*)())GetProcAddress(hDll, "UnSetHideKeys");
		unsetHideKeyHook();
		hookHide = FALSE;
	}
	void stopEmulate(HMODULE hDll) {
		if (!hookEmulate)
			return;
		unsetEmulateKeyHook = (int(*)())GetProcAddress(hDll, "UnSetEmulateKeys");
		unsetEmulateKeyHook();
		hookEmulate = FALSE;
	}
	void clear(HMODULE hDll) {
		if (hookHide) {
			stopHide(hDll);
		}
		if (hookEmulate) {
			stopEmulate(hDll);
		}
	}
	TCHAR getStartKey() {
		return hotkeyStart;
	}
	TCHAR getStopHide() {
		return hotkeyResetHideKey;
	}
	TCHAR getStopEmulate() {
		return hotkeyResetEmulateKey;
	}
	BOOL getStopKeyboardFilter() {
		return stopKeyboardFilter;
	}
};
BOOL LrSequences::stopKeyboardFilter = FALSE;
TCHAR LrSequences::hotkeyStart = 'Q';
TCHAR LrSequences::hotkeyResetEmulateKey = 'E';
TCHAR LrSequences::hotkeyResetHideKey = 'W';
HINSTANCE LrSequences::hInst = NULL;
int LrSequences::setNewKeyFlag = 0;
std::vector<SequenceDescriptor> LrSequences::actions;
HWND LrSequences::hDlg = NULL;
TCHAR LrSequences::processNameStart[MAX_PATH];
TCHAR LrSequences::processNameStop[MAX_PATH];
TCHAR LrSequences::sequenceName[MAX_PATH];
std::vector<TCHAR> LrSequences::hideKeys;
std::vector<EmulateKey> LrSequences::emulateKeys;
TCHAR LrSequences::newKeyString[16];
