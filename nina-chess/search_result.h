#pragma once
#include "utils.h"

#include <iostream>

#include "move.h"
#include "score.h"
#include "search_core.h"

struct SearchResult
{
	Move Pv[MAX_DEPTH];
	size_t PvLength = 0ULL;
	Score Score = Score::DRAW;
	size_t Nodes = 0;
	int64_t Depth{};

	void PrintUciInfo(size_t duration_in_ms = 0) const
	{
		const double duration = static_cast<double>(duration_in_ms) / 1000.0;
		std::cout << "info depth " << Depth << " score cp " << int(Score) << " nodes " << Nodes;
		if (duration_in_ms != 0)
		{
			std::cout << " nps " << static_cast<size_t>(static_cast<double>(Nodes) / duration);
			std::cout << " time " << duration_in_ms;
		}
		std::cout << " pv ";

		for (uint32_t pv_id = 0; pv_id < PvLength; pv_id++)
		{
			std::cout << Pv[pv_id].ToUciMove() << " ";
		}
		std::cout << std::endl;
	}
};