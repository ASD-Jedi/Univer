#pragma once
#include <Windows.h>
#include <ntddscsi.h> // for ATA_PASS_THROUGH_EX
#include <iostream>
#include "identify_device_data_struct.h"
#include <string>

// Taken from smartmontools
// Copies n bytes (or n-1 if n is odd) from in to out, but swaps adjacents
// bytes.
static void swapbytes(char * out, const char * in, size_t n)
{
	for (size_t i = 0; i < n; i += 2) {
		out[i] = in[i + 1];
		out[i + 1] = in[i];
	}
}

// Taken from smartmontools
// Copies in to out, but removes leading and trailing whitespace.
static void trim(char * out, const char * in)
{
	// Find the first non-space character (maybe none).
	int first = -1;
	int i;
	for (i = 0; in[i]; i++)
		if (!isspace((int)in[i])) {
			first = i;
			break;
		}

	if (first == -1) {
		// There are no non-space characters.
		out[0] = '\0';
		return;
	}

	// Find the last non-space character.
	for (i = strlen(in) - 1; i >= first && isspace((int)in[i]); i--)
		;
	int last = i;

	strncpy(out, in + first, last - first + 1);
	out[last - first + 1] = '\0';
}

// Taken from smartmontools
// Convenience function for formatting strings from ata_identify_device
void ata_format_id_string(char * out, const unsigned char * in, int n)
{
	bool must_swap = true;
#ifdef __NetBSD__
	/* NetBSD kernel delivers IDENTIFY data in host byte order (but all else is LE) */
	// TODO: Handle NetBSD case in os_netbsd.cpp
	if (isbigendian())
		must_swap = !must_swap;
#endif
	char tmp[65];
	n = n > 64 ? 64 : n;
	if (!must_swap)
		strncpy(tmp, (const char *)in, n);
	else
		swapbytes(tmp, (const char *)in, n);
	tmp[n] = '\0';
	trim(out, tmp);
}

std::string atas[7] = {
	"ATA1",
	"ATA2",
	"ATA3",
	"ATA4",
	"ATA5",
	"ATA6",
	"ATA7"
};

void printAtas(int maxVersion) {
	for (int i = maxVersion - 1; i >= 0; --i) {
		std::cout << "\t" << atas[i] << std::endl;
	}
}

//from ata-4 all support equal to 1, they need only to check lower atas standards
//rate and time bounds are taken from https://en.wikipedia.org/wiki/UDMA 
//technical supports are taken from http://perscom.ru/index.php/ataide/762-ata41
void printAtaStandards(int rate, int time, int smartSupport, int udmaSupport, int lbaSuport) {
	std::cout << "Supported ATA Standards: \n";
	if (rate < 16 || time < 120) {
		if (udmaSupport) {
			printAtas(4);
		} else if (smartSupport) {
			printAtas(3);
		} else if (lbaSuport) {
			printAtas(2);
		} else {
			printAtas(1);
		}
	} else if (rate >= 16 && rate < 44 && time > 45) {
		printAtas(4);
	}
	else if (rate >= 44 && rate < 100 && time > 20) {
		printAtas(5);
	} else if (rate >= 100 && rate < 133 && time > 12) {
		printAtas(6);
	} else {
		printAtas(7);
	}
}

IDENTIFY_DEVICE_DATA* getIdentifyDeviceData(char* strForCreateFile)
{
	HANDLE handle = CreateFileA(
		strForCreateFile,
		GENERIC_READ | GENERIC_WRITE, //IOCTL_ATA_PASS_THROUGH requires read-write
		FILE_SHARE_READ,
		0,            //no security attributes
		OPEN_EXISTING,
		0,              //flags and attributes
		0             //no template file
	);

	if (handle == INVALID_HANDLE_VALUE) {
		std::cout << "Invalid handle - CreateFile("<<strForCreateFile<<"): Error - "<< GetLastError() << "\n";
	}

	// IDENTIFY command requires a 512 byte buffer for data:
	const unsigned int IDENTIFY_buffer_size = 512;
	const BYTE IDENTIFY_command_ID = 0xEC;
	unsigned char Buffer[IDENTIFY_buffer_size + sizeof(ATA_PASS_THROUGH_EX)] = { 0 };
	ATA_PASS_THROUGH_EX & PTE = *(ATA_PASS_THROUGH_EX *)Buffer;
	PTE.Length = sizeof(PTE);
	PTE.TimeOutValue = 10;
	PTE.DataTransferLength = 512;
	PTE.DataBufferOffset = sizeof(ATA_PASS_THROUGH_EX);

	// Set up the IDE registers as specified in ATA spec.
	IDEREGS * ir = (IDEREGS *)PTE.CurrentTaskFile;
	ir->bCommandReg = IDENTIFY_command_ID;
	ir->bSectorCountReg = 1;

	PTE.AtaFlags = ATA_FLAGS_DATA_IN | ATA_FLAGS_DRDY_REQUIRED;

	DWORD BR = 0;
	BOOL b = DeviceIoControl(handle, IOCTL_ATA_PASS_THROUGH, &PTE, sizeof(Buffer), &PTE, sizeof(Buffer), &BR, 0);
	if (b == 0) {
		std::cout << "Invalid call in getIdentifyData\n";
	}

	IDENTIFY_DEVICE_DATA * data = (IDENTIFY_DEVICE_DATA *)(Buffer + sizeof(ATA_PASS_THROUGH_EX));
	char model[40 + 1];
	ata_format_id_string(model, data->ModelNumber, sizeof(model) - 1);

	char serial[20 + 1];
	ata_format_id_string(serial, data->SerialNumber, sizeof(serial) - 1);

	char firmware[8 + 1];
	ata_format_id_string(firmware, data->FirmwareRevision, sizeof(firmware) - 1);

	int lbaSupport = data->Capabilities.LbaSupported;
	int smartSupport = data->CommandSetSupport.SmartCommands;
	int ultraDmaSupport = data->UltraDMASupport;
	int rate = (int)data->MaximumBlockTransfer;
	int time = data->MinimumMWXferCycleTime;

	std::cout << "ModelNumber:      " << model << "\n";
	std::cout << "SerialNumber:     " << serial << "\n";
	std::cout << "FirmwareRevision: " << firmware << "\n";
	printAtaStandards(rate, time, smartSupport, ultraDmaSupport, lbaSupport);
	CloseHandle(handle);
	return data;
}