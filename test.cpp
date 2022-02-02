#include <iostream>
#include "PulseManager.h"

constexpr auto LoopCount = 10000 * 100;
void benchPulseManager() {
	auto& pm = PulseManager::Instance();
	auto loops = LoopCount;

	using std::chrono::high_resolution_clock;
	using std::chrono::duration_cast;
	using std::chrono::duration;
	using std::chrono::milliseconds;
	auto t1 = high_resolution_clock::now();
	auto i = 0;
	while (--loops) {
		if (pm.IncreaseClock(123, ePulse::GuildComment, std::chrono::milliseconds(100)))
			i++;
	};
	auto t2 = high_resolution_clock::now();
	/* Getting number of milliseconds as an integer. */
	auto ms_int = duration_cast<milliseconds>(t2 - t1);
	/* Getting number of milliseconds as a double. */
	duration<double, std::milli> ms_double = t2 - t1;
	std::cout << "Bench PulseManager class: i(" << i;
	std::cout << ") time ";
	std::cout << ms_double.count() << "ms\n";
}

enum eTest {TEST1,TEST2,TEST3,TEST4,TEST_MAX};
PulseManager::DurationT TestArray[TEST_MAX]{};

PulseManager::DurationT GetTest(eTest key2) {
	if (key2 >= TEST_MAX)
		return PulseManager::TypeClock::duration::zero();

	return TestArray[key2];
}

void SetTest(eTest key2, PulseManager::DurationT value, bool appendCurrent = true) {
	if (appendCurrent)
		value += PulseManager::GetChrono();
	TestArray[key2] = value;
}

bool CheckTest(eTest key2) {
	if (GetTest(key2) > PulseManager::GetChrono())
		return false;
	return true;
}

bool IncreaseTest(eTest key2, PulseManager::DurationT nextLapse) {
	if (!CheckTest(key2))
		return false;
	SetTest(key2, nextLapse, true);
	return true;
}

void benchNormalArray() {
	auto& pm = PulseManager::Instance();
	auto loops = LoopCount;

	using std::chrono::high_resolution_clock;
	using std::chrono::duration_cast;
	using std::chrono::duration;
	using std::chrono::milliseconds;
	auto t1 = high_resolution_clock::now();
	auto i = 0;
	while (--loops) {
		if (IncreaseTest(TEST1, std::chrono::milliseconds(100)))
			i++;
	};
	auto t2 = high_resolution_clock::now();
	/* Getting number of milliseconds as an integer. */
	auto ms_int = duration_cast<milliseconds>(t2 - t1);
	/* Getting number of milliseconds as a double. */
	duration<double, std::milli> ms_double = t2 - t1;
	std::cout << "Bench NormalArray class: i(" << i;
	std::cout << ") time ";
	std::cout << ms_double.count() << "ms\n";
}

int main() {
	auto & pm = PulseManager::Instance();
	benchPulseManager();
	benchNormalArray();

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
