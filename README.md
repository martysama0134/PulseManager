# PulseManager

[![CI](https://github.com/martysama0134/PulseManager/actions/workflows/ci.yml/badge.svg)](https://github.com/martysama0134/PulseManager/actions/workflows/ci.yml)

A header-only C++20 rate-limiter for anti-flood and cooldown management.

PulseManager tracks per-entity cooldowns using a two-level hash map keyed by entity ID and action type. It supports both simple time-based cooldowns and burst-limiting (N actions per time window). Designed for game servers but usable anywhere you need lightweight, in-process rate limiting.

## Features

- **Single header** — drop `PulseManager.h` into your project
- **Zero dependencies** — standard library only
- **C++20 concepts** — key types are checked at compile time
- **Template parameters** — bring your own enum/key types
- **Thread-local singleton** — lock-free, no contention
- **Steady clock** — monotonic, immune to wall-clock adjustments
- **Sweep / GC** — evict expired entries on demand

## Requirements

- C++20 compiler (MSVC 2019 16.10+, GCC 10+, Clang 10+)

## Installation

Copy `PulseManager.h` into your project and `#include` it.

## Quick Start

```cpp
#include "PulseManager.h"
using namespace std::chrono_literals;

// 1. Define your action types
enum class ePulse { GuildComment, CommandRequest, ItemDrop, BuySell };

// 2. Create a type alias
using PM = pulse::PulseManager<ePulse>;

// 3. Rate-limit an action (1.5 s cooldown)
uint32_t playerID = 42;
if (!PM::Instance().IncreaseClock(playerID, ePulse::GuildComment, 1500ms))
{
    double remaining = PM::Instance().RemainingSeconds(playerID, ePulse::GuildComment);
    printf("Wait %.1f seconds\n", remaining);
    return;
}

// 4. Burst-limit (5 commands per 500 ms window)
if (!PM::Instance().IncreaseCount(playerID, ePulse::CommandRequest, 500ms, 5))
{
    // flood detected
    return;
}

// 5. Cleanup on disconnect
PM::Instance().Clear(playerID);
```

## API Reference

All methods live on `pulse::PulseManager<SubKeyT, MainKeyT>`.

### Clock-based rate limiting

| Method | Returns | Description |
|--------|---------|-------------|
| `CheckClock(key, action)` | `bool` | True if cooldown expired or never set |
| `IncreaseClock(key, action, cooldown)` | `bool` | Allows action and sets cooldown; false if still blocked |
| `RemainingTime(key, action)` | `Duration` | Time left on cooldown (negative = expired) |
| `RemainingSeconds(key, action)` | `double` | Same, as floating-point seconds |

### Count-based burst limiting

| Method | Returns | Description |
|--------|---------|-------------|
| `IncreaseCount(key, action, window, maxCount)` | `bool` | Allows up to N actions per window; resets on expiry |
| `GetCount(key, action)` | `Count` | Remaining burst allowance |

### Cleanup

| Method | Description |
|--------|-------------|
| `Clear(key)` | Remove all entries for an entity |
| `Clear(key, action)` | Remove a single entry |
| `ClearAll()` | Wipe all state |
| `Sweep()` | Evict all expired entries; returns count removed |

### Diagnostics

| Method | Returns | Description |
|--------|---------|-------------|
| `Size()` | `size_t` | Number of tracked entities |
| `Empty()` | `bool` | True if no entries exist |
| `EntryCount()` | `size_t` | Total (key, action) pairs across all entities |

## Custom Key Types

Any type that satisfies `std::equality_comparable` and `std::hash` works:

```cpp
// Integer keys
using PM = pulse::PulseManager<int, int>;

// String entity IDs
using PM = pulse::PulseManager<int, std::string>;

// Default MainKeyT is uint32_t
using PM = pulse::PulseManager<MyEnum>;
```

## Thread Safety

`Instance()` returns a `thread_local` singleton — each thread owns its own independent state. No mutex, no contention. If you need shared state across threads, use a single non-singleton instance with external synchronization.

## Memory Management

PulseManager uses `std::unordered_map` internally, which does not shrink on erase. To avoid memory growth in long-running servers:

- **Call `Clear(entityID)` when an entity disconnects.** This is the most important rule. Without it, cooldown entries for departed players accumulate indefinitely.
- **Call `Sweep()` periodically** (e.g. every 60 seconds) to evict all expired entries as a safety net.
- **Call `ClearAll()`** on shutdown or when you want to fully reclaim memory.

```cpp
// On player disconnect
PM::Instance().Clear(playerID);

// Periodic cleanup (e.g. in a server tick)
auto removed = PM::Instance().Sweep();
```

## Migrating from v1.x

v2.0 is a breaking change. Key differences:

| v1.x | v2.0 |
|------|------|
| `PulseManager::Instance()` | `pulse::PulseManager<ePulse>::Instance()` |
| `ePulse` enum defined in header | You define your own enum |
| `PULSEMANAGER_CLOCK_TO_SEC2(key, action)` | `pm.RemainingSeconds(key, action)` |
| `high_resolution_clock` | `steady_clock` (monotonic) |
| `#define __PULSEMANAGER__CLOCK_SUPPORT__` | Always enabled, no preprocessor flags |

Minimal migration — add this to your project:

```cpp
enum class ePulse { GuildComment, CommandRequest, ItemDrop, /* ... */ };
using PulseManager = pulse::PulseManager<ePulse>;
```

Existing call sites (`PulseManager::Instance().IncreaseClock(...)`) work unchanged after the alias.

## Building Tests

```bash
# MSVC
cl /std:c++20 /EHsc /O2 test.cpp

# GCC / Clang
g++ -std=c++20 -O2 -o test test.cpp
./test
```

## License

MIT — see [LICENSE](LICENSE).
