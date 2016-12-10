#pragma once
#include <iostream>
#include <Windows.h>
#include <string>
#include <vector>
#include <powrprof.h>

#pragma comment(lib, "PowrProf.lib")

class PowerState {
public:
	static std::vector<std::string> getState() {
		std::vector<std::string> res;
		SYSTEM_POWER_STATUS status = { 0 };
		if (!GetSystemPowerStatus(&status)) {
			std::string errorMsg = "Error in PowerState::GetSystemPowerState()" + GetLastError();
			res.push_back(errorMsg);
			return res;
		}
		if (status.ACLineStatus == 0) {
			res.push_back("Mode: Battery");
		}
		else {
			res.push_back("Mode: AC");
		}
		DWORD percent = status.BatteryLifePercent;
		res.push_back(("Charge level: " + std::to_string(percent) + "%"));
		if (status.ACLineStatus == 0) {
			res.push_back("Time left: " + std::to_string(status.BatteryLifeTime) + " seconds");
		}
		return res;
	}
	static int isBattery() {
		SYSTEM_POWER_STATUS status = { 0 };
		if (!GetSystemPowerStatus(&status)) {
			std::string errorMsg = "Error in PowerState::GetSystemPowerState()" + GetLastError();
			std::cout << errorMsg;
		}
		if (status.ACLineStatus == 0) {
			return 1;
		}
		else {
			return 0;
		}
	}
};

class ScreenShutdowner {
private:
	int time;
	int a;
	int oldTime;
	SYSTEM_POWER_POLICY powerPolicy;
public:
	ScreenShutdowner(int timer) {
		powerPolicy = { 0 };
		CallNtPowerInformation(SystemPowerPolicyCurrent, NULL, 0, &powerPolicy, sizeof(powerPolicy));
		oldTime = powerPolicy.VideoTimeout;
		powerPolicy.VideoTimeout = timer;
		if (PowerState::isBattery()) {
			CallNtPowerInformation(SystemPowerPolicyDc, &powerPolicy, sizeof(powerPolicy), &powerPolicy, sizeof(powerPolicy));
		}
		else {
			CallNtPowerInformation(SystemPowerPolicyAc, &powerPolicy, sizeof(powerPolicy), &powerPolicy, sizeof(powerPolicy));
		}
		this->time = timer;
		a = 0;
	}
	
	~ScreenShutdowner() {
		powerPolicy.VideoTimeout = oldTime;
		if (PowerState::isBattery()) {
			CallNtPowerInformation(SystemPowerPolicyDc, &powerPolicy, sizeof(powerPolicy), &powerPolicy, sizeof(powerPolicy));
		}
		else {
			CallNtPowerInformation(SystemPowerPolicyAc, &powerPolicy, sizeof(powerPolicy), &powerPolicy, sizeof(powerPolicy));
		}
	}
};
