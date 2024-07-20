#pragma once
#include "utils.h"

#include <vector>

#include "move.h"

enum class TTFlag
{
	EXACT,
	ALPHA,
	BETA
};

struct TranspositionTableEntry
{
	uint64_t key = 0;
	Score score = Score::DRAW;
	int depth = 0;
	Move best_move;
	TTFlag flag = TTFlag::EXACT;
};

class TranspositionTable
{
public:
	TranspositionTable(const size_t size_in_mb);

	void Insert(const TranspositionTableEntry& entry);
	const TranspositionTableEntry& Get(const uint64_t key) const;

private:
	std::vector<TranspositionTableEntry> entries;
};