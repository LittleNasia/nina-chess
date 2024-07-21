#pragma once
#include "utils.h"

#include "board.h"
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

template<Color side_to_move>
Score search(const Board& board, AlphaBeta alpha_beta, SearchStack& search_info);

SearchResult start_search(const Board& board, const int depth, TranspositionTable& tt);

#include "search.inl"