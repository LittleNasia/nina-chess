#pragma once
#include "search_cancellation_policy.h"
#include "transposition_table.h"
#include "search_core.h"
#include "utils.h"

class SharedSearchContext
{
public:
	SharedSearchContext(const SearchConstraints searchConstraints, const TimePoint& searchStartTimepoint,
		TranspositionTable* transposition_table);

	forceinline constexpr SearchCancellationPolicy& GetCancellationPolicy() { return m_CancellationPolicy; }
	forceinline constexpr TranspositionTable& GetTranspositionTable() { return *m_TranspositionTable; }
	forceinline constexpr const TranspositionTable& GetTranspositionTable() const { return *m_TranspositionTable; }
	forceinline constexpr int64_t GetSearchDepth() const { return m_SearchDepth; }

private:
	forceinline constexpr int64_t calculateSearchDepth(const SearchConstraints& searchConstraints);

	TranspositionTable* m_TranspositionTable;
	SearchCancellationPolicy m_CancellationPolicy;
	int64_t m_SearchDepth;
};


SharedSearchContext::SharedSearchContext(const SearchConstraints searchConstraints, const TimePoint& searchStartTimepoint,
	TranspositionTable* transposition_table) :
	m_TranspositionTable(transposition_table),
	m_CancellationPolicy(searchStartTimepoint, searchConstraints),
	m_SearchDepth(calculateSearchDepth(searchConstraints))
{
}

forceinline constexpr int64_t SharedSearchContext::calculateSearchDepth(const SearchConstraints& searchConstraints)
{
	int64_t searchDepth = searchConstraints.Depth == -1
		? std::numeric_limits<int64_t>::max()
		: searchConstraints.Depth;
	searchDepth = std::min(searchDepth, MAX_DEPTH);
	

	return searchDepth;
}
