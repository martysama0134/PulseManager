#include <iostream>
#include "PulseManager.h"

int main() {
	auto & pm = PulseManager::Instance();
	while (true) {
		system("pause");

#ifdef __PULSEMANAGER__SECOND_SUPPORT__
		if (pm.CheckSecond(123, "kek", 3))
			std::cout << "SECOND HIT " << pm.GetSecond(123, "kek") << std::endl;
		else
			puts("SECOND NOT HIT");
#endif

#ifdef __PULSEMANAGER__CLOCK_SUPPORT__
		//if (pm.CheckClock(123, "kek", std::chrono::milliseconds(100)))
		if (pm.IncreaseClock(123, ePulse::GuildComment, std::chrono::milliseconds(3000)))
			std::cout << "CLOCK HIT " << pm.GetClock(123, ePulse::GuildComment).count() << std::endl;
		else
			std::cout << "CLOCK NOT HIT " << PULSEMANAGER_CLOCK_TO_SEC2(123, ePulse::GuildComment) << std::endl;
			//std::cout << "CLOCK NOT HIT " << PULSEMANAGER_CLOCK_TO_SEC(pm.DiffClock(123, "kek")) << std::endl;
			//std::cout << "CLOCK NOT HIT " << pm.DiffClock(123, "kek").count() << std::endl;
			//std::cout << "CLOCK NOT HIT " << std::chrono::duration_cast<std::chrono::milliseconds>(pm.DiffClock(123, "kek")).count() << std::endl;
#endif
	}
	return 0;
}
