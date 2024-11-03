#pragma once
#include "utils.h"

#include "search_cancellation_policy.h"
#include "transposition_table.h"

class SharedSearchContext
{
public:
	SharedSearchContext(const SearchConstraints search_constraints, const TimePoint& search_start_timepoint,
		TranspositionTable* transposition_table) :
		transposition_table(transposition_table),
		cancellation_policy(search_start_timepoint, search_constraints),
		search_depth( calculateSearchDepth(search_constraints) )
	{
	}

	forceinline constexpr SearchCancellationPolicy& GetCancellationPolicy() { return cancellation_policy; }
	forceinline constexpr TranspositionTable& GetTranspositionTable() { return *transposition_table; }
	forceinline constexpr const TranspositionTable& GetTranspositionTable() const { return *transposition_table; }
	forceinline constexpr int64_t GetSearchDepth() const { return search_depth; }

private:
	forceinline constexpr int64_t calculateSearchDepth(const SearchConstraints& search_constraints);

	TranspositionTable* transposition_table;
	SearchCancellationPolicy cancellation_policy;
	int64_t search_depth;
};


inline constexpr int64_t SharedSearchContext::calculateSearchDepth(const SearchConstraints& search_constraints)
{
	int64_t searchDepth = search_constraints.depth == -1
		? std::numeric_limits<int64_t>::max()
		: search_constraints.depth;

	return searchDepth;
}