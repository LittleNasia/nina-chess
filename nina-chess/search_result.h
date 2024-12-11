#pragma once
#include "move.h"
#include "score.h"
#include "search_core.h"
#include "utils.h"

struct SearchResult
{
	Move Pv[MAX_DEPTH];
	size_t PvLength = 0ULL;
	Score Score = Score::DRAW;
	size_t Nodes = 0;
	int64_t Depth{};

	void PrintUciInfo(size_t durationInMs = 0) const;
};

void SearchResult::PrintUciInfo(size_t durationInMs) const
{
	const double duration = static_cast<double>(durationInMs) / 1000.0;
	std::cout << "info depth " << Depth << " score cp " << int(Score) << " nodes " << Nodes;
	if (durationInMs != 0)
	{
		std::cout << " nps " << static_cast<size_t>(static_cast<double>(Nodes) / duration);
		std::cout << " time " << durationInMs;
	}
	std::cout << " pv ";

	for (uint32_t pv_id = 0; pv_id < PvLength; pv_id++)
	{
		std::cout << Pv[pv_id].ToUciMove() << " ";
	}
	std::cout << std::endl;
}
