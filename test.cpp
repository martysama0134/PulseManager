#include <iostream>
#include "PulseManager.h"

int main() {
	auto & pm = PulseManager::Instance();
	while (true) {
		system("pause");
		if (pm.CheckSecond(123, "kek", 3))
			std::cout << "SECOND HIT " << pm.GetSecond(123, "kek") << std::endl;
		else
			puts("SECOND NOT HIT");

		//if (pm.CheckClock(123, "kek", std::chrono::milliseconds(100)))
		if (pm.CheckClock(123, "kek", std::chrono::milliseconds(3000)))
			std::cout << "CLOCK HIT " << pm.GetClock(123, "kek").count() << std::endl;
		else
			std::cout << "CLOCK NOT HIT " << pm.DiffClock(123, "kek").count() << std::endl;
	}
	return 0;
}
