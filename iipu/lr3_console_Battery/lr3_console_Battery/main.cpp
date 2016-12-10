#include "Battery.hpp"
using namespace std;

void printState();
void loopEnd(int& time);

int main() {

	const int times = 15;
	const int shutdownTime = 10;
	int time = 0;
	ScreenShutdowner shutdown(shutdownTime);

	while (time < times) {
		printState();
		loopEnd(time);
	}
	
	return 0;
}

void printState() {
	vector<string> res = PowerState::getState();
	for (vector<string>::iterator it = res.begin(); it != res.end(); ++it) {
		cout << *it << endl;
	}
}

void loopEnd(int& time) {
	Sleep(1000);
	++time;
	system("cls");
}