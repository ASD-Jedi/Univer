#pragma once
#include <windows.h>
#include <winioctl.h>
#include <tchar.h>
#include <stdio.h>
#include "device.hpp"

class UsbRemover {
private:
	HANDLE hVolume;
	HANDLE OpenVolume(TCHAR cDriveLetter);
	BOOL LockVolume(HANDLE hVolume);
	BOOL DismountVolume(HANDLE hVolume);
	BOOL PreventRemovalOfVolume(HANDLE hVolume, BOOL fPrevent);
	BOOL AutoEjectVolume(HANDLE hVolume);
	BOOL CloseVolume(HANDLE hVolume);
	TCHAR message[MAX_PATH];
public:	
	BOOL EjectVolume(UsbDeviceDescription desc);
	BOOL EjectVolume(TCHAR letter);

	TCHAR* GetLastMessage();
};


TCHAR* UsbRemover::GetLastMessage()
{
	return message;
}

HANDLE UsbRemover::OpenVolume(TCHAR cDriveLetter)
{
	HANDLE hVolume;
	UINT uDriveType;
	TCHAR szVolumeName[8];
	TCHAR szRootName[5];
	DWORD dwAccessFlags;
	_stprintf_s(szRootName, TEXT("%c:\\"), cDriveLetter);
	_stprintf_s(szVolumeName, TEXT("\\\\.\\%c:"), cDriveLetter);

	uDriveType = GetDriveType(szRootName);
	switch (uDriveType) {
	case DRIVE_REMOVABLE:
		dwAccessFlags = GENERIC_READ | GENERIC_WRITE;
		break;
	default:
		_stprintf_s(message, TEXT("Cannot eject.  Drive type is incorrect.\n"));
		return INVALID_HANDLE_VALUE;
	}

	hVolume = CreateFile(szVolumeName,
		dwAccessFlags,
		FILE_SHARE_READ | FILE_SHARE_WRITE,
		NULL,
		OPEN_EXISTING,
		0,
		NULL);
	if (hVolume == INVALID_HANDLE_VALUE)
		return INVALID_HANDLE_VALUE;

	return hVolume;
}

BOOL UsbRemover::CloseVolume(HANDLE hVolume)
{
	return CloseHandle(hVolume);
}

#define LOCK_TIMEOUT        10000       // 10 Seconds
#define LOCK_RETRIES        20

BOOL UsbRemover::LockVolume(HANDLE hVolume)
{
	DWORD dwBytesReturned;
	DWORD dwSleepAmount;
	int nTryCount;

	dwSleepAmount = LOCK_TIMEOUT / LOCK_RETRIES;

	// Do this in a loop until a timeout period has expired
	for (nTryCount = 0; nTryCount < LOCK_RETRIES; nTryCount++) {
		if (DeviceIoControl(hVolume,
			FSCTL_LOCK_VOLUME,
			NULL, 0,
			NULL, 0,
			&dwBytesReturned,
			NULL))
			return TRUE;

		Sleep(dwSleepAmount);
	}

	return FALSE;
}

BOOL UsbRemover::DismountVolume(HANDLE hVolume)
{
	DWORD dwBytesReturned;

	return DeviceIoControl(hVolume,
		FSCTL_DISMOUNT_VOLUME,
		NULL, 0,
		NULL, 0,
		&dwBytesReturned,
		NULL);
}

BOOL UsbRemover::PreventRemovalOfVolume(HANDLE hVolume, BOOL fPreventRemoval)
{
	DWORD dwBytesReturned;
	PREVENT_MEDIA_REMOVAL PMRBuffer;

	PMRBuffer.PreventMediaRemoval = fPreventRemoval;

	return DeviceIoControl(hVolume,
		IOCTL_STORAGE_MEDIA_REMOVAL,
		&PMRBuffer, sizeof(PREVENT_MEDIA_REMOVAL),
		NULL, 0,
		&dwBytesReturned,
		NULL);
}

BOOL UsbRemover::AutoEjectVolume(HANDLE hVolume)
{
	DWORD dwBytesReturned;

	return DeviceIoControl(hVolume,
		IOCTL_STORAGE_EJECT_MEDIA,
		NULL, 0,
		NULL, 0,
		&dwBytesReturned,
		NULL);
}

BOOL UsbRemover::EjectVolume(TCHAR cDriveLetter) {
	HANDLE hVolume;
	BOOL fRemoveSafely = FALSE;
	BOOL fAutoEject = FALSE;

	// Open the volume.
	hVolume = OpenVolume(cDriveLetter);
	if (hVolume == INVALID_HANDLE_VALUE)
		return FALSE;

	// Lock and dismount the volume.
	if (LockVolume(hVolume) && DismountVolume(hVolume)) {
		fRemoveSafely = TRUE;

		// Set prevent removal to false and eject the volume.
		if (PreventRemovalOfVolume(hVolume, FALSE) &&
			AutoEjectVolume(hVolume))
			fAutoEject = TRUE;
	}

	// Close the volume so other processes can use the drive.
	if (!CloseVolume(hVolume))
		return FALSE;

	if (fAutoEject)
		_stprintf_s(message, TEXT("Media in Drive %c has been ejected safely.\n"),
			cDriveLetter);
	else {
		if (fRemoveSafely)
			_stprintf_s(message, TEXT("Media in Drive %c can be safely removed.\n"),
				cDriveLetter);
	}

	return TRUE;
}

BOOL UsbRemover::EjectVolume(UsbDeviceDescription usb)
{
	TCHAR cDriveLetter = usb.volumeLetter[0];
	return this->EjectVolume(cDriveLetter);
}