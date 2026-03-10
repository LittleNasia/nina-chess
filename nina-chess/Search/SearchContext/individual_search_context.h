#pragma once
#include "Search/SearchContext/shared_search_context.h"
#include "Core/Engine/utils.h"
#include <cstdint>

class IndividualSearchContext
{
public:
	forceinline IndividualSearchContext(SharedSearchContext& sharedSearchContext, const int64_t depthToSearchTo);
	size_t Nodes{ 0ULL };

	forceinline constexpr operator SharedSearchContext&() { return m_SharedSearchContext; }
	forceinline void MakeMove();
	forceinline void UndoMove();

	forceinline int64_t GetSearchDepth() const { return m_SearchDepth; }
	forceinline int64_t GetRemainingDepth() const { return m_RemainingDepth; }

private:
	SharedSearchContext& m_SharedSearchContext;
	int64_t m_RemainingDepth{ 0ULL };
	int64_t m_SearchDepth{ 0ULL };
};

forceinline IndividualSearchContext::IndividualSearchContext(SharedSearchContext& sharedSearchContext, const int64_t depthToSearchTo) :
	m_SharedSearchContext(sharedSearchContext),
	m_RemainingDepth(depthToSearchTo),
	m_SearchDepth(0)
	{}


forceinline void IndividualSearchContext::MakeMove()
{
	m_SearchDepth++;
	m_RemainingDepth--;
}

forceinline void IndividualSearchContext::UndoMove()
{
	m_SearchDepth--;
	m_RemainingDepth++;
}
