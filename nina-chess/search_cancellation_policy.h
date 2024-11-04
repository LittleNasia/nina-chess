#pragma once
#include "utils.h"

#include <atomic>

#include "search_nodes_cancellation_policy.h"
#include "search_time_cancellation_policy.h"

class SearchCancellationPolicy
{
public:
	SearchCancellationPolicy(const TimePoint& startTime, const SearchConstraints& searchConstraints) :
		m_NodeCancellationPolicy(searchConstraints),
		m_TimeCancellationPolicy(startTime, searchConstraints)
	{}

	forceinline bool CheckForAbort(uint64_t nodes);
	forceinline bool IsAborted() const { return m_SearchAbortedFlag.test(); }
	forceinline constexpr size_t GetNodeLimit() const { return m_NodeCancellationPolicy.GetNodeLimit();}
	forceinline constexpr int64_t GetTimeLimit() const { return m_TimeCancellationPolicy.GetTimeLimit(); }

private:
	SearchNodesCancellationPolicy m_NodeCancellationPolicy;
	SearchTimeCancellationPolicy m_TimeCancellationPolicy;

	std::atomic_flag m_SearchAbortedFlag = ATOMIC_FLAG_INIT;
};


forceinline bool SearchCancellationPolicy::CheckForAbort(uint64_t nodes)
{
	if (m_TimeCancellationPolicy.ShouldAbort() || m_NodeCancellationPolicy.ShouldAbort(nodes))
	{
		m_SearchAbortedFlag.test_and_set();
		return true;
	}
	return false;
}