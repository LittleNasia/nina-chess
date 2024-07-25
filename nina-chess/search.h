#pragma once
#include "utils.h"

#include <iostream>

#include "move.h"
#include "search_stack.h"
#include "transposition_table.h"

struct SearchResult
{
	Move pv[max_depth];
	size_t pv_length = 0ULL;
	Score score = Score::DRAW;
	size_t nodes = 0;
	int depth;

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

struct SearchConstraints
{
	int depth = -1;
	int time = -1;
	int movetime = -1;
	int nodes = -1;
};

std::vector<SearchResult> start_search(std::vector<Position> position_stack, TranspositionTable& tt, Evaluator& evaluator, const SearchConstraints& search_constraints);