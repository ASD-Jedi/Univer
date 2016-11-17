#include <Windows.h>
#include <vfw.h>
#include <tchar.h>
#include <stdio.h>
#pragma comment (lib, "Vfw32.lib")

typedef struct CameraState {
	BOOL video;
	BOOL shown;
	BOOL screen;
}CameraState;

class WebCamera {
private:
	int next;
	CameraState state;
	HWND hwndWebCam;
	HWND app;
	TCHAR* pathToSave = TEXT("frames/");
	void postWork() {
		if (!state.shown) {
			SendMessage(hwndWebCam, WM_CAP_DRIVER_DISCONNECT, true, 0);
		}
		else {
			SendMessage(hwndWebCam, WM_CAP_DRIVER_CONNECT, 0, 0);
			SendMessage(hwndWebCam, WM_CAP_SET_SCALE, true, 0);
			SendMessage(hwndWebCam, WM_CAP_SET_PREVIEWRATE, 66, 0);
			SendMessage(hwndWebCam, WM_CAP_SET_PREVIEW, true, 0);
		}
	}
public:
	WebCamera(HWND app) {
		state = { FALSE, FALSE, FALSE };
		hwndWebCam = capCreateCaptureWindow(TEXT("camera window"), 0, 0, 100, 300, 300, NULL, 0);
		this->app = app;
		FILE* fileWithNext = fopen("next.txt", "r");
		if (fileWithNext == NULL) {
			next = 0;
		}
		else {
			fscanf(fileWithNext, "%d", &next);
			fclose(fileWithNext);
		}
	}
	~WebCamera() {
		if (state.video) {
			stopRecord();
		}
		if (state.shown) {
			ShowWindow(hwndWebCam, SW_HIDE);
			SendMessage(hwndWebCam, WM_CAP_DRIVER_DISCONNECT, 0, 0);
		}
		FILE* fileWithNext = fopen("next.txt", "w");
		fprintf(fileWithNext, "%d", next);
		fclose(fileWithNext);
	}
	void showCamera() {
		if (state.shown) {
			return;
		}
		SendMessage(hwndWebCam, WM_CAP_DRIVER_CONNECT, 0, 0);
		ShowWindow( hwndWebCam, SW_SHOW);
		SendMessage(hwndWebCam, WM_CAP_SET_SCALE, true, 0);
		SendMessage(hwndWebCam, WM_CAP_SET_PREVIEWRATE, 66, 0);
		SendMessage(hwndWebCam, WM_CAP_SET_PREVIEW, true, 0);
		state.shown = TRUE;
	}
	void saveFrame() {
		if(!state.shown)
			SendMessage(hwndWebCam, WM_CAP_DRIVER_CONNECT, 0, 0);
		WCHAR t[128] = L"frames/FrameHide";
		WCHAR nextStr[128];
		_itow(next++, nextStr, 10);
		wcscat(t, nextStr);
		wcscat(t, L".bmp");
		SendMessage(hwndWebCam, WM_CAP_FILE_SAVEDIB, 0, (WPARAM)t);
		postWork();
	}
	void hideCamera() {
		if (!state.shown) {
			return;
		}
		ShowWindow(hwndWebCam, SW_HIDE);
		SendMessage(hwndWebCam, WM_CAP_DRIVER_DISCONNECT, true, 0);
		state.shown = FALSE;
	}
	void startRecord() {
		if (state.video) {
			return;
		}
		SendMessage(hwndWebCam, WM_CAP_DRIVER_CONNECT, 0, 0);
		capCaptureSequence(hwndWebCam);
		state.video = TRUE;
	}
	void stopRecord() {
		if (!state.video) {
			return;
		}
		capFileSaveAs(hwndWebCam, TEXT("videos/video.avi"));
		SendMessage(hwndWebCam, WM_CAP_DRIVER_CONNECT, 0, 0);
		postWork();
		state.video = FALSE;
	}
};