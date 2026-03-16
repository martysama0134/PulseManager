// PulseManager — Header-only C++20 rate-limiter
// https://github.com/martysama0134/PulseManager
// SPDX-License-Identifier: MIT

#ifndef PULSEMANAGER_H
#define PULSEMANAGER_H
#pragma once

#if __cplusplus < 202002L && (!defined(_MSVC_LANG) || _MSVC_LANG < 202002L)
	#error "PulseManager requires C++20 or later."
#endif

#include <chrono>
#include <concepts>
#include <cstddef>
#include <cstdint>
#include <unordered_map>

namespace pulse {

// Key types must be usable as unordered_map keys.
template <typename T>
concept HashableKey = std::equality_comparable<T> && requires(T v) {
	{ std::hash<T>{}(v) } -> std::convertible_to<std::size_t>;
};

/// Thread-local singleton rate-limiter.
///
/// Tracks per-entity cooldowns and burst counters using a two-level
/// unordered_map keyed by (MainKey, SubKey).  Each entry stores a
/// deadline time-point and an optional attempt counter.
///
/// Thread safety: Instance() returns a thread_local singleton — each
/// thread owns independent state with no locking required.
template <HashableKey SubKeyT, HashableKey MainKeyT = uint32_t>
class PulseManager
{
public:
	using Clock     = std::chrono::steady_clock;
	using TimePoint = Clock::time_point;
	using Duration  = Clock::duration;
	using Count     = int64_t;

	/// Returns the thread-local singleton.
	static PulseManager& Instance() noexcept
	{
		thread_local PulseManager instance;
		return instance;
	}

	// ── Clock-based rate limiting ──────────────────────────────

	/// Returns true if the cooldown for (key, action) has expired or was never set.
	[[nodiscard]] bool CheckClock(MainKeyT key, SubKeyT action) const
	{
		return Deadline(key, action) <= Clock::now();
	}

	/// Sets a new cooldown if the previous one expired.
	/// Returns true when the action is allowed, false if still in cooldown.
	[[nodiscard]] bool IncreaseClock(MainKeyT key, SubKeyT action, Duration cooldown)
	{
		if (!CheckClock(key, action))
			return false;
		entries_[key][action].deadline = Clock::now() + cooldown;
		return true;
	}

	/// Remaining cooldown duration (negative or zero when expired).
	[[nodiscard]] Duration RemainingTime(MainKeyT key, SubKeyT action) const
	{
		return Deadline(key, action) - Clock::now();
	}

	/// Remaining cooldown as floating-point seconds.
	[[nodiscard]] double RemainingSeconds(MainKeyT key, SubKeyT action) const
	{
		using Sec = std::chrono::duration<double>;
		return std::chrono::duration_cast<Sec>(RemainingTime(key, action)).count();
	}

	// ── Count-based burst limiting ─────────────────────────────

	/// Allows up to maxCount actions within a time window.
	/// Returns true if the action is allowed, false when the burst limit is hit.
	/// Resets both the window and counter once the window expires.
	[[nodiscard]] bool IncreaseCount(MainKeyT key, SubKeyT action,
									 Duration window, Count maxCount)
	{
		if (!CheckClock(key, action))
		{
			auto& e = entries_[key][action];
			if (--e.count <= 0)
				return false;
			return true;
		}
		auto& e = entries_[key][action];
		e.deadline = Clock::now() + window;
		e.count    = maxCount;
		return true;
	}

	/// Returns the remaining burst count for (key, action).
	[[nodiscard]] Count GetCount(MainKeyT key, SubKeyT action) const
	{
		return FindEntry(key, action).count;
	}

	// ── Cleanup ────────────────────────────────────────────────

	/// Removes all entries for a given entity.
	void Clear(MainKeyT key) { entries_.erase(key); }

	/// Removes a single (key, action) entry.
	void Clear(MainKeyT key, SubKeyT action)
	{
		if (auto it = entries_.find(key); it != entries_.end())
		{
			it->second.erase(action);
			if (it->second.empty())
				entries_.erase(it);
		}
	}

	/// Removes all state.
	void ClearAll() noexcept { entries_.clear(); }

	/// Removes all expired entries. Returns the number removed.
	std::size_t Sweep()
	{
		const auto now = Clock::now();
		std::size_t removed = 0;
		for (auto outer = entries_.begin(); outer != entries_.end();)
		{
			for (auto inner = outer->second.begin(); inner != outer->second.end();)
			{
				if (inner->second.deadline <= now)
				{
					inner = outer->second.erase(inner);
					++removed;
				}
				else
					++inner;
			}
			outer = outer->second.empty() ? entries_.erase(outer) : std::next(outer);
		}
		return removed;
	}

	// ── Diagnostics ────────────────────────────────────────────

	/// Number of tracked entities (unique MainKeys).
	[[nodiscard]] std::size_t Size() const noexcept { return entries_.size(); }

	/// True if no entries are tracked.
	[[nodiscard]] bool Empty() const noexcept { return entries_.empty(); }

	/// Total number of (key, action) entries across all entities.
	[[nodiscard]] std::size_t EntryCount() const noexcept
	{
		std::size_t n = 0;
		for (const auto& [_, inner] : entries_)
			n += inner.size();
		return n;
	}

private:
	struct Entry
	{
		TimePoint deadline{};
		Count count{0};
	};

	using InnerMap = std::unordered_map<SubKeyT, Entry>;
	using OuterMap = std::unordered_map<MainKeyT, InnerMap>;
	OuterMap entries_;

	[[nodiscard]] TimePoint Deadline(MainKeyT key, SubKeyT action) const
	{
		if (auto o = entries_.find(key); o != entries_.end())
			if (auto i = o->second.find(action); i != o->second.end())
				return i->second.deadline;
		return TimePoint{};
	}

	[[nodiscard]] const Entry& FindEntry(MainKeyT key, SubKeyT action) const
	{
		static const Entry empty;
		if (auto o = entries_.find(key); o != entries_.end())
			if (auto i = o->second.find(action); i != o->second.end())
				return i->second;
		return empty;
	}
};

} // namespace pulse

#endif // PULSEMANAGER_H
