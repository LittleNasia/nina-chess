#pragma once
#include "shared_search_context.h"
#include "utils.h"

class IndividualSearchContext
{
public:
	IndividualSearchContext(SharedSearchContext& sharedSearchContext) : m_SharedSearchContext(sharedSearchContext) {}

	size_t Nodes{ 0ULL };

	forceinline constexpr operator SharedSearchContext&() { return m_SharedSearchContext; }

private:
	SharedSearchContext& m_SharedSearchContext;
};
