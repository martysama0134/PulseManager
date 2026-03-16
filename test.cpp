// PulseManager test suite & benchmarks
// Compile: cl /std:c++20 /EHsc /O2 test.cpp
//     g++: g++ -std=c++20 -O2 -o test test.cpp

#include "PulseManager.h"

#include <cassert>
#include <chrono>
#include <cstdio>
#include <string>
#include <thread>

using namespace std::chrono_literals;

// ── Helpers ────────────────────────────────────────────────────────

static int g_passed = 0;
static int g_failed = 0;

#define RUN(fn)                                           \
	do {                                                  \
		std::printf("  %-50s", #fn);                      \
		try { fn(); ++g_passed; std::puts(" OK"); }       \
		catch (...) { ++g_failed; std::puts(" FAIL"); }   \
	} while (0)

#define EXPECT(cond) \
	do { if (!(cond)) { std::printf("\n    ASSERT(%s) @ line %d", #cond, __LINE__); throw 0; } } while (0)

// ── Test types ─────────────────────────────────────────────────────

enum class Action { Chat, Trade, Drop, Mount, Command };
using PM = pulse::PulseManager<Action>;

// ── Clock tests ────────────────────────────────────────────────────

void test_first_call_allowed()
{
	auto& pm = PM::Instance();
	pm.ClearAll();
	EXPECT(pm.IncreaseClock(1, Action::Chat, 50ms));
}

void test_second_call_blocked()
{
	auto& pm = PM::Instance();
	pm.ClearAll();
	EXPECT(pm.IncreaseClock(1, Action::Chat, 50ms));
	EXPECT(!pm.IncreaseClock(1, Action::Chat, 50ms));
}

void test_allowed_after_cooldown()
{
	auto& pm = PM::Instance();
	pm.ClearAll();
	EXPECT(pm.IncreaseClock(1, Action::Chat, 20ms));
	std::this_thread::sleep_for(30ms);
	EXPECT(pm.IncreaseClock(1, Action::Chat, 20ms));
}

void test_check_clock_readonly()
{
	auto& pm = PM::Instance();
	pm.ClearAll();
	EXPECT(pm.CheckClock(1, Action::Chat));   // never set → expired
	EXPECT(pm.IncreaseClock(1, Action::Chat, 50ms));
	EXPECT(!pm.CheckClock(1, Action::Chat));  // in cooldown
	EXPECT(!pm.CheckClock(1, Action::Chat));  // still read-only, no side effect
}

void test_independent_actions()
{
	auto& pm = PM::Instance();
	pm.ClearAll();
	EXPECT(pm.IncreaseClock(1, Action::Chat, 100ms));
	EXPECT(pm.IncreaseClock(1, Action::Trade, 100ms));  // different action
	EXPECT(!pm.IncreaseClock(1, Action::Chat, 100ms));   // Chat still blocked
}

void test_independent_entities()
{
	auto& pm = PM::Instance();
	pm.ClearAll();
	EXPECT(pm.IncreaseClock(1, Action::Chat, 100ms));
	EXPECT(pm.IncreaseClock(2, Action::Chat, 100ms));   // different entity
	EXPECT(!pm.IncreaseClock(1, Action::Chat, 100ms));   // entity 1 still blocked
}

void test_remaining_time()
{
	auto& pm = PM::Instance();
	pm.ClearAll();
	(void)pm.IncreaseClock(1, Action::Chat, 100ms);
	EXPECT(pm.RemainingSeconds(1, Action::Chat) > 0.0);
	EXPECT(pm.RemainingTime(1, Action::Chat) > PM::Duration::zero());
}

void test_remaining_time_expired()
{
	auto& pm = PM::Instance();
	pm.ClearAll();
	// Never set — should be zero or negative
	EXPECT(pm.RemainingSeconds(1, Action::Chat) <= 0.0);
}

// ── Count tests ────────────────────────────────────────────────────

void test_count_allows_burst()
{
	auto& pm = PM::Instance();
	pm.ClearAll();
	// Allow 3 calls within 200ms
	EXPECT(pm.IncreaseCount(1, Action::Command, 200ms, 3));  // 1st: resets window
	EXPECT(pm.IncreaseCount(1, Action::Command, 200ms, 3));  // 2nd: count 3→2
	EXPECT(pm.IncreaseCount(1, Action::Command, 200ms, 3));  // 3rd: count 2→1
}

void test_count_blocks_after_limit()
{
	auto& pm = PM::Instance();
	pm.ClearAll();
	EXPECT(pm.IncreaseCount(1, Action::Command, 200ms, 3));  // 1: sets count=3
	EXPECT(pm.IncreaseCount(1, Action::Command, 200ms, 3));  // 2: count=2
	EXPECT(pm.IncreaseCount(1, Action::Command, 200ms, 3));  // 3: count=1
	EXPECT(!pm.IncreaseCount(1, Action::Command, 200ms, 3)); // 4: count=0 → blocked
}

void test_count_resets_after_window()
{
	auto& pm = PM::Instance();
	pm.ClearAll();
	EXPECT(pm.IncreaseCount(1, Action::Command, 20ms, 2));
	EXPECT(pm.IncreaseCount(1, Action::Command, 20ms, 2));
	EXPECT(!pm.IncreaseCount(1, Action::Command, 20ms, 2)); // blocked
	std::this_thread::sleep_for(30ms);
	EXPECT(pm.IncreaseCount(1, Action::Command, 20ms, 2));  // window expired → allowed
}

void test_get_count()
{
	auto& pm = PM::Instance();
	pm.ClearAll();
	(void)pm.IncreaseCount(1, Action::Command, 200ms, 5);
	EXPECT(pm.GetCount(1, Action::Command) == 5);
	(void)pm.IncreaseCount(1, Action::Command, 200ms, 5);
	EXPECT(pm.GetCount(1, Action::Command) == 4);
}

// ── Clear tests ────────────────────────────────────────────────────

void test_clear_single()
{
	auto& pm = PM::Instance();
	pm.ClearAll();
	(void)pm.IncreaseClock(1, Action::Chat, 500ms);
	(void)pm.IncreaseClock(1, Action::Trade, 500ms);
	pm.Clear(1, Action::Chat);
	EXPECT(pm.IncreaseClock(1, Action::Chat, 500ms));    // cleared
	EXPECT(!pm.IncreaseClock(1, Action::Trade, 500ms));   // untouched
}

void test_clear_entity()
{
	auto& pm = PM::Instance();
	pm.ClearAll();
	(void)pm.IncreaseClock(1, Action::Chat, 500ms);
	(void)pm.IncreaseClock(1, Action::Trade, 500ms);
	pm.Clear(1);
	EXPECT(pm.IncreaseClock(1, Action::Chat, 500ms));
	EXPECT(pm.IncreaseClock(1, Action::Trade, 500ms));
}

void test_clear_all()
{
	auto& pm = PM::Instance();
	pm.ClearAll();
	(void)pm.IncreaseClock(1, Action::Chat, 500ms);
	(void)pm.IncreaseClock(2, Action::Trade, 500ms);
	pm.ClearAll();
	EXPECT(pm.Empty());
	EXPECT(pm.IncreaseClock(1, Action::Chat, 500ms));
	EXPECT(pm.IncreaseClock(2, Action::Trade, 500ms));
}

void test_clear_nonexistent()
{
	auto& pm = PM::Instance();
	pm.ClearAll();
	pm.Clear(999);                    // no-op
	pm.Clear(999, Action::Chat);      // no-op
	EXPECT(pm.Empty());
}

// ── Sweep tests ────────────────────────────────────────────────────

void test_sweep_removes_expired()
{
	auto& pm = PM::Instance();
	pm.ClearAll();
	(void)pm.IncreaseClock(1, Action::Chat, 10ms);
	(void)pm.IncreaseClock(2, Action::Trade, 500ms);
	std::this_thread::sleep_for(20ms);
	auto removed = pm.Sweep();
	EXPECT(removed == 1);
	EXPECT(pm.Size() == 1);  // entity 2 remains
}

void test_sweep_cleans_empty_entities()
{
	auto& pm = PM::Instance();
	pm.ClearAll();
	(void)pm.IncreaseClock(1, Action::Chat, 10ms);
	std::this_thread::sleep_for(20ms);
	pm.Sweep();
	EXPECT(pm.Empty());  // entity 1 fully removed
}

// ── Diagnostics tests ──────────────────────────────────────────────

void test_size_and_entry_count()
{
	auto& pm = PM::Instance();
	pm.ClearAll();
	EXPECT(pm.Size() == 0);
	EXPECT(pm.EntryCount() == 0);
	EXPECT(pm.Empty());

	(void)pm.IncreaseClock(1, Action::Chat, 500ms);
	(void)pm.IncreaseClock(1, Action::Trade, 500ms);
	(void)pm.IncreaseClock(2, Action::Drop, 500ms);
	EXPECT(pm.Size() == 2);
	EXPECT(pm.EntryCount() == 3);
	EXPECT(!pm.Empty());
}

// ── Template with different key types ──────────────────────────────

void test_int_keys()
{
	using IntPM = pulse::PulseManager<int, int>;
	auto& pm = IntPM::Instance();
	pm.ClearAll();
	EXPECT(pm.IncreaseClock(42, 7, 50ms));
	EXPECT(!pm.IncreaseClock(42, 7, 50ms));
}

void test_string_main_key()
{
	using StrPM = pulse::PulseManager<int, std::string>;
	auto& pm = StrPM::Instance();
	pm.ClearAll();
	EXPECT(pm.IncreaseClock("player_abc", 1, 50ms));
	EXPECT(!pm.IncreaseClock("player_abc", 1, 50ms));
}

void test_uint64_keys()
{
	using U64PM = pulse::PulseManager<uint16_t, uint64_t>;
	auto& pm = U64PM::Instance();
	pm.ClearAll();
	EXPECT(pm.IncreaseClock(UINT64_MAX, 0, 50ms));
	EXPECT(!pm.IncreaseClock(UINT64_MAX, 0, 50ms));
}

// ── Benchmarks ─────────────────────────────────────────────────────

static constexpr int BenchIterations = 10'000'000;

void bench_increase_clock()
{
	auto& pm = PM::Instance();
	pm.ClearAll();
	int hits = 0;

	auto t1 = PM::Clock::now();
	for (int i = 0; i < BenchIterations; ++i)
		if (pm.IncreaseClock(123, Action::Chat, 100ms))
			++hits;
	auto t2 = PM::Clock::now();

	auto ms = std::chrono::duration<double, std::milli>(t2 - t1).count();
	std::printf("  IncreaseClock  %d hits / %d iters  %.1f ms\n", hits, BenchIterations, ms);
}

void bench_increase_count()
{
	auto& pm = PM::Instance();
	pm.ClearAll();
	int hits = 0;

	auto t1 = PM::Clock::now();
	for (int i = 0; i < BenchIterations; ++i)
		if (pm.IncreaseCount(123, Action::Command, 100ms, 5))
			++hits;
	auto t2 = PM::Clock::now();

	auto ms = std::chrono::duration<double, std::milli>(t2 - t1).count();
	std::printf("  IncreaseCount  %d hits / %d iters  %.1f ms\n", hits, BenchIterations, ms);
}

// ── Main ───────────────────────────────────────────────────────────

int main()
{
	std::puts("=== PulseManager Tests ===\n");

	std::puts("[clock]");
	RUN(test_first_call_allowed);
	RUN(test_second_call_blocked);
	RUN(test_allowed_after_cooldown);
	RUN(test_check_clock_readonly);
	RUN(test_independent_actions);
	RUN(test_independent_entities);
	RUN(test_remaining_time);
	RUN(test_remaining_time_expired);

	std::puts("\n[count]");
	RUN(test_count_allows_burst);
	RUN(test_count_blocks_after_limit);
	RUN(test_count_resets_after_window);
	RUN(test_get_count);

	std::puts("\n[clear]");
	RUN(test_clear_single);
	RUN(test_clear_entity);
	RUN(test_clear_all);
	RUN(test_clear_nonexistent);

	std::puts("\n[sweep]");
	RUN(test_sweep_removes_expired);
	RUN(test_sweep_cleans_empty_entities);

	std::puts("\n[diagnostics]");
	RUN(test_size_and_entry_count);

	std::puts("\n[template variants]");
	RUN(test_int_keys);
	RUN(test_string_main_key);
	RUN(test_uint64_keys);

	std::printf("\n--- %d passed, %d failed ---\n", g_passed, g_failed);

	std::puts("\n=== Benchmarks (10M iterations) ===\n");
	bench_increase_clock();
	bench_increase_count();

	return g_failed ? 1 : 0;
}
