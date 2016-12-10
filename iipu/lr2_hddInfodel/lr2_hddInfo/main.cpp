#include <iostream>
#include <Windows.h>
#include <winioctl.h>
#include "identify_device_data.hpp"
using namespace std;

void printTransferMode() {
	HANDLE device = CreateFile(TEXT("\\\\.\\PhysicalDrive0"), GENERIC_READ | GENERIC_WRITE, FILE_SHARE_WRITE | FILE_SHARE_READ, NULL,
		OPEN_EXISTING, 0, NULL);
	if (device == INVALID_HANDLE_VALUE) {
		cout << "Fail to CreateFile(\"\\\\.\\PhysicalDrive0\"): Error - " << GetLastError();
	}
	STORAGE_PROPERTY_QUERY query = {};
	query.PropertyId = StorageAdapterProperty;
	query.QueryType = PropertyStandardQuery;
	STORAGE_ADAPTER_DESCRIPTOR descriptor = {};
	DWORD read;

	bool res = DeviceIoControl(device, IOCTL_STORAGE_QUERY_PROPERTY, &query, sizeof(STORAGE_PROPERTY_QUERY),
		&descriptor, sizeof(STORAGE_DESCRIPTOR_HEADER), &read, NULL);
	if (!res) {
		cout << "GetTransferMode() : Error - " << GetLastError() << endl;
		return;
	}
	else {
		cout << "HDD TransferMode: ";
		if (descriptor.AdapterUsesPio)
			cout << "PIO " << descriptor.AdapterUsesPio << endl;
		else
			cout << "DMA" << endl;
	}
	CloseHandle(device);
}

BOOL GetDriveGeometry(LPWSTR wszPath, DISK_GEOMETRY *pdg)
{
	HANDLE hDevice = INVALID_HANDLE_VALUE;
	BOOL bResult = FALSE;
	DWORD junk = 0;
	hDevice = CreateFileW(wszPath, 0, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, 0, NULL);
	if (hDevice == INVALID_HANDLE_VALUE)
	{
		cout << "GetDriveGeometry() : Error - " << GetLastError() << endl;
		return (FALSE);
	}
	bResult = DeviceIoControl(hDevice, IOCTL_DISK_GET_DRIVE_GEOMETRY, NULL, 0, pdg, sizeof(*pdg), &junk, NULL);
	CloseHandle(hDevice);
	return (bResult);
}

void printDiskSize() {
	DISK_GEOMETRY pdg = { 0 };
	BOOL bResult = FALSE;  
	ULONGLONG DiskSize = 0;

	bResult = GetDriveGeometry(TEXT("\\\\.\\PhysicalDrive0"), &pdg);
	if (!bResult) {
		cout << "GetDriveGeometry(): Error - " << GetLastError();
	}
	else {
		DiskSize = pdg.Cylinders.QuadPart * (ULONG)pdg.TracksPerCylinder *
			(ULONG)pdg.SectorsPerTrack * (ULONG)pdg.BytesPerSector;
		wprintf(L"Disk size       = %I64d (Bytes)\n"
			L"                = %.2f (Gb)\n",
			DiskSize, (double)DiskSize / (1024 * 1024 * 1024));
	}
}

void printFreeSpace() {
	ULARGE_INTEGER freeBytes;
	GetDiskFreeSpaceEx(TEXT("C:"), NULL, NULL, &freeBytes);
	wprintf(L"Disk free       = %I64d (Bytes)\n"
		L"                = %.2f (Gb)\n",
		freeBytes, (double)freeBytes.QuadPart/ (1024 * 1024 * 1024));
}

int main() {
	
	IDENTIFY_DEVICE_DATA * data = getIdentifyDeviceData("\\\\.\\PhysicalDrive0");
	printDiskSize();
	printFreeSpace();
	printTransferMode();
	cout << endl << endl;
	return 0;
}

