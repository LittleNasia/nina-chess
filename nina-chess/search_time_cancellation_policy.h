#pragma once
#include "search_constraints.h"
#include "utils.h"

using TimePoint = std::chrono::time_point<std::chrono::high_resolution_clock>;

class SearchTimeCancellationPolicy
{
public:
	SearchTimeCancellationPolicy(const TimePoint& startTime, const SearchConstraints& searchConstraints);

	forceinline bool ShouldAbort() const;
	forceinline constexpr int64_t GetTimeLimit() const { return m_MaxSearchDuration; }

private:
	forceinline int64_t getSearchDurationInMs() const;
	forceinline constexpr int64_t calculateMaxSearchDuration(const int64_t totalTime, const int64_t movetime) const;
	
	TimePoint m_StartTime;
	int64_t m_MaxSearchDuration;
};


SearchTimeCancellationPolicy::SearchTimeCancellationPolicy(const TimePoint& startTime, const SearchConstraints& searchConstraints) :
	m_StartTime{ startTime },
	m_MaxSearchDuration{ calculateMaxSearchDuration(searchConstraints.Time, searchConstraints.Movetime) }
{
}

forceinline bool SearchTimeCancellationPolicy::ShouldAbort() const
{
	int64_t searchDuration = getSearchDurationInMs();
	return searchDuration >= m_MaxSearchDuration;
}

forceinline int64_t SearchTimeCancellationPolicy::getSearchDurationInMs() const
{
	const auto currentTime = std::chrono::high_resolution_clock::now();
	return std::chrono::duration_cast<std::chrono::milliseconds>(currentTime - m_StartTime).count();
}

forceinline constexpr int64_t SearchTimeCancellationPolicy::calculateMaxSearchDuration(const int64_t totalTime, const int64_t movetime) const
{
	constexpr double timePerMoveFactor = 0.05f;
	constexpr double compensationFactor = 0.98f;

	// 5% of total time allocated to each move, otherwise infinity sekonds
	const int64_t allocationFromTotalTime = totalTime == -1
		? std::numeric_limits<int64_t>::max()
		: int64_t(static_cast<double>(totalTime) * timePerMoveFactor);

	const int64_t movetimeDuration = movetime == -1
		? std::numeric_limits<int64_t>::max()
		: movetime;

	// little window to compensate for the time it takes to abort the search
	return int64_t(static_cast<double>(std::min(allocationFromTotalTime, movetimeDuration)) * compensationFactor);
}

