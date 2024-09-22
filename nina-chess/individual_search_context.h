#pragma once
#include "utils.h"

#include "search_context.h"

class IndividualSearchContext
{
public:
	IndividualSearchContext(SharedSearchContext& shared_search_context) :
		shared_search_context(shared_search_context)
	{
	}

	size_t nodes{ 0ULL };

	forceinline constexpr operator SharedSearchContext& () { return shared_search_context; }

private:
	SharedSearchContext& shared_search_context;
};