#pragma once
#include <Windows.h>
#include <windot11.h>
#include <wlanapi.h>
#include <tchar.h>
#pragma comment(lib, "Wlanapi.lib")
#include <string>
#include <vector>

#ifdef UNICODE
typedef std::wstring TSTRING;
#elif
typedef std::string TSTRING;
#endif

typedef struct InfoWifi {
	TSTRING name;
	TSTRING macAddress;
	TSTRING auth;
	TSTRING quality;
} WifiInfo;

class WifiScanner {
private:
	static TSTRING lastError;
	static bool realScanning;
	std::vector<WifiInfo> lastScanRes;
	std::vector<WifiInfo> currentScan;
	static const TSTRING auths[9];
	static bool scanning;
	bool isWifiAvailable = false;
	HANDLE clientHandle;
	PWLAN_INTERFACE_INFO_LIST ppInterfaceList;
	PWLAN_AVAILABLE_NETWORK_LIST networkList;
	static void WlanNotification(WLAN_NOTIFICATION_DATA *wlanNotifData, VOID *p) {
		if (wlanNotifData->NotificationCode == wlan_notification_acm_scan_complete) {
			 realScanning = false;
		}
		else if (wlanNotifData->NotificationCode ==
			wlan_notification_acm_scan_fail) {
			TCHAR buffer[1024];
			_stprintf_s(buffer, 1024, TEXT("Error: %x\n"), wlanNotifData->pData);
			lastError = buffer;
			realScanning = false;
		}
	}
	TSTRING getAuth(int type) {
		if (type < 8) {
			return auths[type];
		}
		else {
			return auths[8];
		}
	}
	bool isSecurityEnabled(int type) {
		return (type != 1);
	}
	void endScanning() {
		for (int i = 0; i < networkList->dwNumberOfItems; i++) {
			WifiInfo tmp;
			tmp.auth = getAuth((int)networkList->Network[i].dot11DefaultAuthAlgorithm);
#ifdef UNICODE
			char* name = (char *)networkList->Network[i].dot11Ssid.ucSSID;
			int size = strlen(name) + 1;
			wchar_t* nameW = new wchar_t[size];
			mbstowcs(nameW, name, size);
			tmp.name = nameW;
#else
			tmp.name = (TCHAR *)networkList->Network[i].dot11Ssid.ucSSID;
#endif
			TCHAR qualityBuffer[1024];
			int quality = networkList->Network[i].wlanSignalQuality;
			_stprintf_s(qualityBuffer, 1024, TEXT("%d"), quality);
			tmp.quality = qualityBuffer;
			PWLAN_BSS_LIST bssid;
			bool securityEnabled = isSecurityEnabled(
				(int)networkList->Network[i].dot11DefaultAuthAlgorithm);
			WlanGetNetworkBssList(
				clientHandle, &(ppInterfaceList->InterfaceInfo->InterfaceGuid),
				&(networkList->Network[i].dot11Ssid),
				networkList->Network[i].dot11BssType, securityEnabled, NULL, &bssid);
			unsigned char * macAddress = (unsigned char*)bssid->wlanBssEntries[0].dot11Bssid;
			for (int j = 0; j < 6; j++) {
				char buffer[16];
				sprintf_s(buffer, 16, "%02X", macAddress[j]);
#ifdef UNICODE
				int size = strlen(buffer) + 1;
				wchar_t* macPart = new wchar_t[size];
				mbstowcs(macPart, buffer, size);
				tmp.macAddress += macPart;
#else
				tmp.macAddress += buffer;
#endif
			}
			WlanFreeMemory(bssid);
			currentScan.push_back(tmp);
		}
		lastScanRes = currentScan;
		scanning = false;
	}
public:
	WifiScanner() {
		DWORD version = 1;
		DWORD negotiatedVersion;
		WlanOpenHandle(version, NULL, &negotiatedVersion, &clientHandle);
		WlanEnumInterfaces(clientHandle, NULL, &ppInterfaceList);
		if (ppInterfaceList->dwNumberOfItems == 0) {
			isWifiAvailable = false;
		}
		else {
			isWifiAvailable = true;
		}
	}
	bool startScanning() {
		WlanScan(clientHandle, &(ppInterfaceList->InterfaceInfo->InterfaceGuid),
			NULL, NULL, NULL);
		WlanRegisterNotification(clientHandle, WLAN_NOTIFICATION_SOURCE_ACM, TRUE,
			(WLAN_NOTIFICATION_CALLBACK)WlanNotification, NULL,
			NULL, NULL);
		scanning = true;
		realScanning = true;
		while (realScanning) Sleep(10);
		WlanRegisterNotification(clientHandle, WLAN_NOTIFICATION_SOURCE_NONE, TRUE,
			NULL, NULL, NULL, NULL);
		WlanGetAvailableNetworkList(
			clientHandle, &(ppInterfaceList->InterfaceInfo->InterfaceGuid),
			WLAN_AVAILABLE_NETWORK_INCLUDE_ALL_MANUAL_HIDDEN_PROFILES, NULL,
			&networkList);
		endScanning();
		return true;
	}
	bool isScanning() {
		return scanning;
	}
	
	std::vector<WifiInfo> getLastScanResults() {
		return lastScanRes;
	}
	~WifiScanner() {
		WlanFreeMemory(ppInterfaceList);
		WlanCloseHandle(clientHandle, NULL);
	}
};

TSTRING WifiScanner::lastError = TEXT("");
bool WifiScanner::scanning = false;
bool WifiScanner::realScanning = false;

const TSTRING WifiScanner::auths[9] = { 
	TEXT("EMPTY"),
	TEXT("OPEN"),
	TEXT("WEP"),
	TEXT("WPA"),
	TEXT("WPA PSK"),
	TEXT("WPA NONE"),
	TEXT("WPA2"),
	TEXT("WPA2 PSK"),
	TEXT("IHV")
};