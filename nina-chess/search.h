#pragma once
#include "utils.h"

#include "board.h"
#include "move.h"
#include "search_info.h"
#include "transposition_table.h"

struct SearchResult
{
	Move pv[max_depth];
	size_t pv_length = 0ULL;
	Score score = Score::DRAW;
};

template<Color side_to_move>
Score search(const Board& board, const size_t depth, Score alpha, Score beta, TranspositionTable& tt);

SearchResult start_search(const Board& board, const size_t depth, TranspositionTable& tt);

#include "search.inl"