#pragma once
#include <windows.h>
#include <winioctl.h>
#include <tchar.h>
#include <stdio.h>
#include "device.hpp"

class UsbRemover {

public:
	BOOL IsUSBDevice(DWORD DevInst)
	{
		DWORD size;
		LPBYTE pData;
		BOOL usb = FALSE;
		if (CM_Get_Device_ID_Size(&size, DevInst, 0) == CR_SUCCESS)
		{
			if (size)
			{
				pData = (LPBYTE)GlobalAlloc(GPTR, size + 1);
				if (pData)
				{
					if (CM_Get_Device_ID(DevInst, (PTCHAR)pData, size + 1, 0) == CR_SUCCESS)
					{
						WCHAR wide;
						*(pData + 7) = 0x00;
						if (!_tcscmp((TCHAR*)pData, TEXT("USBSTOR")))
							usb = TRUE;
					}
					GlobalFree(pData);
				}
			}
		}
		return usb;
	}
	//форма info_data определяетяс типом GUID
	void eject(TCHAR letter) {
		TCHAR volume_access_path[] = TEXT("\\\\.\\X:");
		volume_access_path[4] = letter;
		HANDLE vol = CreateFileW(volume_access_path, 0,
			FILE_SHARE_READ | FILE_SHARE_WRITE,
			NULL, OPEN_EXISTING, 0, NULL);
		STORAGE_DEVICE_NUMBER sdn;
		DWORD bytes_ret = 0;
		long DeviceNumber = -1;

		//Это делается таким IOCTL-запросом к устройству
		if (DeviceIoControl(vol,
			IOCTL_STORAGE_GET_DEVICE_NUMBER,
			NULL, 0, &sdn, sizeof(sdn),
			&bytes_ret, NULL))
			DeviceNumber = sdn.DeviceNumber;

		//Хендл нам больше не нужен
		CloseHandle(vol);

		TCHAR devname[] = TEXT("?:");
		TCHAR devpath[] = TEXT("?:\\");
		devname[0] = letter;
		devpath[0] = letter;

		UINT driveType = GetDriveType(devpath);
		const GUID* guid;

		switch (driveType) {
		case DRIVE_REMOVABLE:
			guid = &GUID_DEVINTERFACE_DISK;
			break;
		}
		HDEVINFO devInfo = SetupDiGetClassDevs(guid, NULL, NULL, DIGCF_PRESENT | DIGCF_DEVICEINTERFACE);
		if (devInfo == INVALID_HANDLE_VALUE)
			return;
		DWORD index = 0;
		PSP_DEVICE_INTERFACE_DETAIL_DATA pDevInterfaceDetailData;
		SP_DEVICE_INTERFACE_DATA devInterfaceData;
		SP_DEVINFO_DATA devInfoData;
		DWORD size;
		devInterfaceData.cbSize = sizeof(SP_DEVICE_INTERFACE_DATA);
		BOOL found = false;
		int ret;
		while (true) {
			ret = SetupDiEnumDeviceInterfaces(devInfo, NULL, guid, index, &devInterfaceData);
			if (!ret)
				break;
			size = 0;
			SetupDiGetDeviceInterfaceDetail(devInfo, &devInterfaceData, NULL, 0, &size, NULL);
			pDevInterfaceDetailData = (PSP_DEVICE_INTERFACE_DETAIL_DATA)malloc(sizeof(PSP_DEVICE_INTERFACE_DETAIL_DATA)*(size+2));
			pDevInterfaceDetailData->cbSize = sizeof(SP_DEVICE_INTERFACE_DETAIL_DATA);
			devInterfaceData.cbSize = sizeof(SP_DEVICE_INTERFACE_DATA);
			devInfoData.cbSize = sizeof(SP_DEVINFO_DATA);
			SetupDiGetDeviceInterfaceDetail(devInfo, &devInterfaceData, pDevInterfaceDetailData, size, &size, &devInfoData);
			HANDLE drive = CreateFile(pDevInterfaceDetailData->DevicePath, 0, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, 0, NULL);
			if (drive != INVALID_HANDLE_VALUE) {
				STORAGE_DEVICE_NUMBER sdn;
				DWORD bytesReturned = 0;
				if (DeviceIoControl(drive, IOCTL_STORAGE_GET_DEVICE_NUMBER, NULL, 0, &sdn, sizeof(sdn), &bytesReturned, NULL)) {
					if (DeviceNumber == sdn.DeviceNumber) {
						CloseHandle(drive);
						found = true;
						break;
					}
				}
				CloseHandle(drive);
			}
			++index;
		}
		SetupDiDestroyDeviceInfoList(devInfo);
		DEVINST devParent = 0;
		if (CR_SUCCESS == CM_Get_Parent(&devParent, devInfoData.DevInst, 0)) {
			CM_Request_Device_Eject(devParent, NULL, NULL, 0, 0);
		}
	}
};