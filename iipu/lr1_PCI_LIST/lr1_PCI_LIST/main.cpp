#include <windows.h>
#include <devguid.h>    // for GUID_DEVCLASS_CDROM etc
#include <setupapi.h>
#include <cfgmgr32.h>   // for MAX_DEVICE_ID_LEN, CM_Get_Parent and CM_Get_Device_ID
#define INITGUID
#include <tchar.h>
#include <stdio.h>
#include <string>
#include "pcihdr.h"
//#include "c:\WinDDK\7600.16385.1\inc\api\devpkey.h"

// include DEVPKEY_Device_BusReportedDeviceDesc from WinDDK\7600.16385.1\inc\api\devpropdef.h
#ifdef DEFINE_DEVPROPKEY
#undef DEFINE_DEVPROPKEY
#endif
#ifdef INITGUID
#define DEFINE_DEVPROPKEY(name, l, w1, w2, b1, b2, b3, b4, b5, b6, b7, b8, pid) EXTERN_C const DEVPROPKEY DECLSPEC_SELECTANY name = { { l, w1, w2, { b1, b2,  b3,  b4,  b5,  b6,  b7,  b8 } }, pid }
#else
#define DEFINE_DEVPROPKEY(name, l, w1, w2, b1, b2, b3, b4, b5, b6, b7, b8, pid) EXTERN_C const DEVPROPKEY name
#endif // INITGUID


#define ARRAY_SIZE(arr)     (sizeof(arr)/sizeof(arr[0]))

#pragma comment (lib, "setupapi.lib")


// List all USB devices with some additional information
void ListDevices(CONST GUID *pClassGuid, LPCTSTR pszEnumerator)
{
	unsigned i, j;
	DWORD dwSize, dwPropertyRegDataType;
	CONFIGRET status;
	HDEVINFO hDevInfo;
	SP_DEVINFO_DATA DeviceInfoData;
	const static LPCTSTR arPrefix[3] = { TEXT("VID_"), TEXT("PID_"), TEXT("MI_") };
	TCHAR szDeviceInstanceID[MAX_DEVICE_ID_LEN];
	TCHAR szDesc[1024], szHardwareIDs[4096];
	
	hDevInfo = SetupDiGetClassDevs(pClassGuid, pszEnumerator, NULL,
		pClassGuid != NULL ? DIGCF_PRESENT : DIGCF_ALLCLASSES | DIGCF_PRESENT);
	if (hDevInfo == INVALID_HANDLE_VALUE)
		return;

	for (i = 0; ; i++) {
		DeviceInfoData.cbSize = sizeof(DeviceInfoData);
		if (!SetupDiEnumDeviceInfo(hDevInfo, i, &DeviceInfoData))
			break;

		if (SetupDiGetDeviceRegistryProperty(hDevInfo, &DeviceInfoData, SPDRP_DEVICEDESC,
			&dwPropertyRegDataType, (BYTE*)szDesc,
			sizeof(szDesc),   // The size, in bytes
			&dwSize))
			//_tprintf(TEXT("%d.%s\n"),i + 1, szDesc);
			_tprintf(TEXT("PCI Devce %d\n"),i + 1, szDesc);


		std::wstring desc = szDesc;
		int k = 0;
		std::wstring vendorDecode = TEXT("");
		std::wstring deviceDecode = TEXT("");
		while (desc[k] != ' ') {
			vendorDecode += desc[k++];
		}
		++k;
		while (desc[k] != '\0') {
			deviceDecode += desc[k++];
		}

		status = CM_Get_Device_ID(DeviceInfoData.DevInst, szDeviceInstanceID, MAX_PATH, 0);
		if (status != CR_SUCCESS)
			continue;

		// Display device instance ID
		//_tprintf(TEXT("DeviceID\n\t%s\n\n"), szDeviceInstanceID);
		std::wstring devId = szDeviceInstanceID;
		j = 8;
		_tprintf(TEXT("Vendor id: "));
		while (devId[j] != '&') {
			_tprintf(TEXT("%c"), devId[j++]);
		}
		_tprintf(TEXT("Device id: "));
		while (devId[j] != '&') {
			_tprintf(TEXT("%c"), devId[j++]);
		}
		_tprintf(TEXT("\n"));
	}
}

int _tmain()
{
	// List all connected USB devices
	_tprintf(TEXT("---------------\n"));
	_tprintf(TEXT("- PCI devices -\n"));
	_tprintf(TEXT("---------------\n"));
	ListDevices(NULL, TEXT("USB"));
	return 0;
}