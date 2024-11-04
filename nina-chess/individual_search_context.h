#pragma once
#include "utils.h"

#include "shared_search_context.h"

class IndividualSearchContext
{
public:
	IndividualSearchContext(SharedSearchContext& sharedSearchContext) :
		m_SharedSearchContext(sharedSearchContext)
	{
	}

	size_t Nodes{ 0ULL };

	forceinline constexpr operator SharedSearchContext& () { return m_SharedSearchContext; }

private:
	SharedSearchContext& m_SharedSearchContext;
};