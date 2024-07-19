#pragma once
#include "utils.h"

#include "board.h"
#include "move.h"

inline constexpr size_t max_depth = 256;

struct SearchResult
{
	Move pv[max_depth];
	size_t pv_length = 0ULL;
	Score score = Score::DRAW;
};

template<Color side_to_move>
Score search(Board& board, size_t depth, Score alpha, Score beta);

SearchResult start_search(Board& board, size_t depth);