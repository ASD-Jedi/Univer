#pragma once
#include <Windows.h>
#include <tchar.h>
#include <setupapi.h>
#include <cfgmgr32.h>
#include <initguid.h>
#include <vector>
#include <string>
#pragma comment (lib,"SetupAPI.lib")
#pragma comment (lib,"Cfgmgr32.lib")

#ifdef UNICODE
typedef std::string TSTRING;
#else 
typedef std::wstring TSTRING;
#endif

//struct to hold info, that need for output
typedef struct UsbDeviceDescription {
	TCHAR* volumeName;
	TCHAR* volumeLetter;
	ULONGLONG freeMemory;
	ULONGLONG busyMemory;
	ULONGLONG allMemory;
	PSP_DEVICE_INTERFACE_DETAIL_DATA pDevDetail;
	SP_DEVINFO_DATA devInfoData;
	int index;
} UsbDeviceDescription;

class UsbDevice {
private:
	std::vector<UsbDeviceDescription> devices;
	std::vector<UsbDeviceDescription> oldDevices;

	static const int MAX_GUID_PATH = 50;
	static const GUID GUID_DEVINTERFACE_USB;
	static void enumLoopEnd(int* i, SP_DEVICE_INTERFACE_DATA* devInterfaceData) {
		ZeroMemory(devInterfaceData, sizeof(SP_DEVICE_INTERFACE_DATA));
		devInterfaceData->cbSize = sizeof(SP_DEVICE_INTERFACE_DATA);
		++(*i);
	}
	typedef struct temp {
		char* ñ;
		char* b;
	}Temp;
	//insert usb
	//define usb
	//defin wha type of usb
	//call the suitale enject
	void refresh() {
		devices.clear();
		GUID guid = GUID_DEVINTERFACE_VOLUME;
		SP_DEVICE_INTERFACE_DATA devInterfaceData;
		devInterfaceData.cbSize = sizeof(SP_DEVICE_INTERFACE_DATA);
		DWORD dwSizeOut;
		HDEVINFO hDevInfo = SetupDiGetClassDevs(&guid, NULL, NULL, DIGCF_DEVICEINTERFACE | DIGCF_PRESENT);
		for (int dwIndex = 0; SetupDiEnumDeviceInterfaces(hDevInfo, NULL, &guid, dwIndex, &devInterfaceData); enumLoopEnd(&dwIndex, &devInterfaceData)) {

			SetupDiGetDeviceInterfaceDetail(hDevInfo, &devInterfaceData, NULL, 0, &dwSizeOut, NULL);
			PSP_DEVICE_INTERFACE_DETAIL_DATA pDevDetail = (PSP_DEVICE_INTERFACE_DETAIL_DATA)malloc(sizeof(PSP_DEVICE_INTERFACE_DETAIL_DATA)*(dwSizeOut + 2));
			pDevDetail->cbSize = sizeof(SP_DEVICE_INTERFACE_DETAIL_DATA);
			SP_DEVINFO_DATA devInfoData;
			devInfoData.cbSize = sizeof(SP_DEVINFO_DATA);
			SetupDiGetDeviceInterfaceDetail(hDevInfo, &devInterfaceData, pDevDetail, dwSizeOut, &dwSizeOut, &devInfoData);
			DEVINST parent;
			CM_Get_Parent(&parent, devInfoData.DevInst, 0);
			int nLength = _tcslen(pDevDetail->DevicePath);
			pDevDetail->DevicePath[nLength] = '\\';
			pDevDetail->DevicePath[nLength + 1] = 0;
			TCHAR volumeName[MAX_PATH];

			//works only for volume guid
			if (_tcsstr(pDevDetail->DevicePath, TEXT("usbstor")) && GetVolumeNameForVolumeMountPoint(pDevDetail->DevicePath, volumeName, MAX_PATH)) {
				UsbDeviceDescription newUsb;
				newUsb.volumeName = (TCHAR*)malloc(sizeof(TCHAR) * MAX_PATH);
				newUsb.volumeLetter = (TCHAR*)malloc(sizeof(TCHAR) * MAX_PATH);
				newUsb.index = devices.size();
				newUsb.pDevDetail = pDevDetail;
				_tcscpy_s(newUsb.volumeName, MAX_PATH, volumeName);
				newUsb.devInfoData = devInfoData;
				TCHAR volumeLabelBuffer[MAX_PATH];
				DWORD dwRead;
				GetVolumePathNamesForVolumeName(volumeName, volumeLabelBuffer, MAX_PATH, &dwRead);
				_tcscpy_s(newUsb.volumeLetter, MAX_PATH, volumeLabelBuffer);
				TCHAR tmp[MAX_PATH];
				ULARGE_INTEGER freeBytes;
				ULARGE_INTEGER allMemory;
				GetDiskFreeSpaceEx(newUsb.volumeLetter, NULL, &allMemory, &freeBytes);
				newUsb.freeMemory = freeBytes.QuadPart;
				newUsb.allMemory  = allMemory.QuadPart;
				newUsb.busyMemory = allMemory.QuadPart - freeBytes.QuadPart;
				devices.push_back(newUsb);
			}
		}
	}
public:
	UsbDevice() {

	}

	std::vector<UsbDeviceDescription> getDevices() {
		refresh();
		return devices;
	}

	UsbDeviceDescription getDevice(int i) {
		return devices[i];
	}
	
	void reject(TSTRING tomLetter) {
		for (int i = 0; i < devices.size(); ++i) {
			
		}
	}
};

const GUID UsbDevice::GUID_DEVINTERFACE_USB = { 0xA5DCBF10L, 0x6530,
0x11D2,{ 0x90, 0x1F, 0x00, 0xC0, 0x4F, 0xB9, 0x51,
0xED } };