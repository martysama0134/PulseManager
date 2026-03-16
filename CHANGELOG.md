# Changelog

## v2.0 (2026-03-16)

Complete rewrite as a modern C++20 template library.

### Breaking Changes
- **Namespace**: all types live under `pulse::` (`pulse::PulseManager<SubKeyT>`)
- **Template parameters**: `SubKeyT` and `MainKeyT` are user-defined — the `ePulse` enum is no longer part of the header
- **Removed macros**: `PULSEMANAGER_CLOCK_TO_SEC` / `PULSEMANAGER_CLOCK_TO_SEC2` replaced by `RemainingTime()` / `RemainingSeconds()` methods
- **Removed modules**: `SECOND_SUPPORT` and `M2_SUPPORT` preprocessor blocks deleted
- **Clock change**: `high_resolution_clock` replaced by `steady_clock` (guaranteed monotonic)

### Added
- C++20 `concept HashableKey` compile-time constraint on key types
- `RemainingTime(key, action)` — remaining cooldown as `Duration`
- `RemainingSeconds(key, action)` — remaining cooldown as `double`
- `Sweep()` — garbage-collect all expired entries
- `Size()`, `Empty()`, `EntryCount()` — diagnostics
- `ClearAll()` — wipe all state
- `[[nodiscard]]` on all query/check methods
- `const` / `noexcept` correctness throughout
- 28 automated tests (clock, count, clear, sweep, diagnostics, edge cases, template variants)
- Benchmarks (10M iterations for `IncreaseClock` and `IncreaseCount`)
- GitHub Actions CI (MSVC, GCC, Clang)
- CMake support (`find_package` / `add_subdirectory`)
- Full README with API reference, migration guide, memory management docs

### Changed
- Internal storage uses `TimePoint` (not raw `Duration`) for deadlines
- `SetCount` split into clear internal semantics (no more confusing `decreaseValue` bool)
- Header guard changed from `__COMMON__PULSEMANAGER_H__` to `PULSEMANAGER_H`
- `.vcxproj` updated to C++20 (`stdcpp20`) across all configurations
- `.gitignore` updated to exclude `x64/` build output

## v1.1 (2023-11-15)

### Added
- `IncreaseCount(key, action, window, maxCount)` — burst-limiting with time window + attempt counter
- `GetCount()`, `GetPair()` — const accessors for count and pair data
- `PairValueT` storage: entries now hold `std::pair<DurationT, CountT>`

### Changed
- `MainKeyT` changed from `int` to `uint32_t` for type safety

## v1.0 (2022-02-02)

Initial release.

### Added
- `PulseManager` singleton class with thread-local storage
- Clock-based rate limiting: `CheckClock`, `IncreaseClock`, `SetClock`, `GetClock`, `DiffClock`
- `ClearClock` — remove cooldown entries (single or per-entity)
- `PULSEMANAGER_CLOCK_TO_SEC` / `PULSEMANAGER_CLOCK_TO_SEC2` convenience macros
- Second-based module (`SECOND_SUPPORT`) — `CheckSecond`, `GetSecond`, `SetSecond`
- M2 pulse-based module (`M2_SUPPORT`) — `CheckPulse`, `GetPulse`, `SetPulse`, `Sec2Pulse`, `Pulse2Sec`
- `ePulse` enum: `GuildComment`, `CommandRequest`, `ItemDrop`, `BoxOpening`, `BuySell`, `SafeboxMove`, `ChannelStatus`, `RideMount`, `SharedRequest`
- Benchmarks: `benchPulseManager`, `benchNormalArray`, `benchPulseString`
- Interactive prompt tests: `promptClockTest`
