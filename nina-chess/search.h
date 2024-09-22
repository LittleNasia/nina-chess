#pragma once
#include "utils.h"

#include <iostream>

#include "evaluator.h"
#include "move.h"
#include "position_stack.h"
#include "search_context.h"
#include "transposition_table.h"
#include "uci_incremental_updater.h"

struct SearchResult
{
	Move pv[max_depth];
	size_t pv_length = 0ULL;
	Score score = Score::DRAW;
	size_t nodes = 0;
	int64_t depth;

	void PrintUciInfo(size_t duration_in_ms = 0) const
	{
		double duration = duration_in_ms / 1000.0;
		std::cout << "info depth " << depth << " score cp " << int(score) << " nodes " << nodes;
		if(duration_in_ms != 0)
			std::cout << " nps " << int(nodes / duration);
		std::cout << " pv ";

		for (int pv_id = 0; pv_id < pv_length; pv_id++)
		{
			std::cout << pv[pv_id].ToUciMove() << " ";
		}
		std::cout << std::endl;
	}
};

struct AlphaBeta
{
	Score alpha;
	const Score beta;
	forceinline constexpr AlphaBeta Invert() const { return { -beta, -alpha }; }
};

std::vector<SearchResult> start_search(UciIncrementalUpdater& uci_incremental_updater, SharedSearchContext& search_context);