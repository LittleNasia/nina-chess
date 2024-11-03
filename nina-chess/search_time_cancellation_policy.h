#pragma once
#include "utils.h"

#include <chrono>

#include "search_constraints.h"

using TimePoint = std::chrono::time_point<std::chrono::high_resolution_clock>;

class SearchTimeCancellationPolicy
{
public:
	SearchTimeCancellationPolicy(const TimePoint& start_time, const SearchConstraints& constraints) :
		start_time{ start_time },
		max_search_duration{ calculateMaxSearchDuration(constraints.time, constraints.movetime) }
	{}

	forceinline bool ShouldAbort() const;
	forceinline constexpr int64_t GetTimeLimit() const { return max_search_duration; }

private:
	forceinline int64_t getSearchDurationInMs() const;
	forceinline constexpr int64_t calculateMaxSearchDuration(const int64_t total_time, const int64_t movetime) const;
	
	TimePoint start_time;
	int64_t max_search_duration;
};

forceinline bool SearchTimeCancellationPolicy::ShouldAbort() const
{
	int64_t search_duration = getSearchDurationInMs();
	return search_duration >= max_search_duration;
}

inline int64_t SearchTimeCancellationPolicy::getSearchDurationInMs() const
{
	const auto current_time = std::chrono::high_resolution_clock::now();
	return std::chrono::duration_cast<std::chrono::milliseconds>(current_time - start_time).count();
}

forceinline constexpr int64_t SearchTimeCancellationPolicy::calculateMaxSearchDuration(const int64_t total_time, const int64_t movetime) const
{
	constexpr double time_per_move_factor = 0.05f;
	constexpr double compensation_factor = 0.98f;

	// 5% of total time allocated to each move, otherwise infinity sekonds
	const int64_t allocation_from_total_time = total_time == -1
		? std::numeric_limits<int64_t>::max()
		: int64_t(static_cast<double>(total_time) * time_per_move_factor);

	const int64_t movetime_duration = movetime == -1
		? std::numeric_limits<int64_t>::max()
		: movetime;

	// little window to compensate for the time it takes to abort the search
	return int64_t(static_cast<double>(std::min(allocation_from_total_time, movetime_duration)) * compensation_factor);
}

