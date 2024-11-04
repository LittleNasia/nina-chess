#pragma once
#include "utils.h"

#include <iostream>

#include "evaluator.h"
#include "move.h"
#include "position_stack.h"
#include "search_result.h"
#include "shared_search_context.h"
#include "transposition_table.h"
#include "uci_incremental_updater.h"

struct AlphaBeta
{
	Score Alpha;
	const Score Beta;
	forceinline constexpr AlphaBeta Invert() const { return { -Beta, -Alpha }; }
};

std::vector<SearchResult> StartSearch(UciIncrementalUpdater& uciIncrementalUpdater, SharedSearchContext& searchContext);