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
	Score alpha;
	const Score beta;
	forceinline constexpr AlphaBeta Invert() const { return { -beta, -alpha }; }
};

std::vector<SearchResult> start_search(UciIncrementalUpdater& uci_incremental_updater, SharedSearchContext& search_context);