#pragma once
#include "search_nodes_cancellation_policy.h"
#include "search_time_cancellation_policy.h"
#include "utils.h"

class SearchCancellationPolicy
{
public:
	SearchCancellationPolicy(const TimePoint& startTime, const SearchConstraints& searchConstraints);

	forceinline bool CheckForAbort(uint64_t nodes);
	forceinline constexpr size_t GetNodeLimit() const { return m_NodeCancellationPolicy.GetNodeLimit(); }
	forceinline constexpr int64_t GetTimeLimit() const { return m_TimeCancellationPolicy.GetTimeLimit(); }
	forceinline bool IsAborted() const { return m_SearchAbortedFlag.test(); }
	
private:
	SearchNodesCancellationPolicy m_NodeCancellationPolicy;
	SearchTimeCancellationPolicy m_TimeCancellationPolicy;

	std::atomic_flag m_SearchAbortedFlag = ATOMIC_FLAG_INIT;
};


SearchCancellationPolicy::SearchCancellationPolicy(const TimePoint& startTime, const SearchConstraints& searchConstraints) :
	m_NodeCancellationPolicy(searchConstraints),
	m_TimeCancellationPolicy(startTime, searchConstraints)
{}

forceinline bool SearchCancellationPolicy::CheckForAbort(uint64_t nodes)
{
	if (m_TimeCancellationPolicy.ShouldAbort() || m_NodeCancellationPolicy.ShouldAbort(nodes))
	{
		m_SearchAbortedFlag.test_and_set();
		return true;
	}
	return false;
}
