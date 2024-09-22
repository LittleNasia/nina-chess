#pragma once
#include "utils.h"

#include <atomic>

#include "search_nodes_cancellation_policy.h"
#include "search_time_cancellation_policy.h"

class SearchCancellationPolicy
{
public:
	SearchCancellationPolicy(const TimePoint& start_time, const SearchConstraints& search_constraints) :
		node_cancellation_policy(search_constraints),
		time_cancellation_policy(start_time, search_constraints)
	{}

	forceinline bool CheckForAbort(uint64_t nodes);
	forceinline bool IsAborted() const { return aborted_flag.test(); }
	forceinline constexpr size_t GetNodeLimit() const { return node_cancellation_policy.GetNodeLimit();}
	forceinline constexpr int64_t GetTimeLimit() const { return time_cancellation_policy.GetTimeLimit(); }

private:
	SearchNodesCancellationPolicy node_cancellation_policy;
	SearchTimeCancellationPolicy time_cancellation_policy;

	std::atomic_flag aborted_flag = ATOMIC_FLAG_INIT;
};


forceinline bool SearchCancellationPolicy::CheckForAbort(uint64_t nodes)
{
	if (time_cancellation_policy.ShouldAbort() || node_cancellation_policy.ShouldAbort(nodes))
	{
		aborted_flag.test_and_set();
		return true;
	}
	return false;
}