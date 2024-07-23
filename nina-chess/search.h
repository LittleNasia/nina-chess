#pragma once
#include "utils.h"

#include "move.h"
#include "search_stack.h"
#include "transposition_table.h"

struct SearchResult
{
	Move pv[max_depth];
	size_t pv_length = 0ULL;
	Score score = Score::DRAW;
	size_t nodes = 0;
};

struct AlphaBeta
{
	Score alpha;
	const Score beta;
	forceinline constexpr AlphaBeta Invert() const { return { -beta, -alpha }; }
};

SearchResult start_search(const Position& position, const int depth, TranspositionTable& tt, Evaluator& evaluator);