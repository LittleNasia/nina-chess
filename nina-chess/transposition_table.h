#pragma once
#include "utils.h"

#include <vector>

#include "move.h"
#include "score.h"
#include "serializable.h"

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
	int64_t depth = 0;
	Move best_move;
	TTFlag flag = TTFlag::EXACT;
};

class TranspositionTable : public Serializable
{
public:
	TranspositionTable(const size_t size_in_mb);

	void Insert(const TranspositionTableEntry& entry, bool force_overwrite);
	const TranspositionTableEntry& Get(const uint64_t key) const;

	void Serialize(std::ofstream& output);
	void Deserialize(std::ifstream& input);
private:
	std::vector<TranspositionTableEntry> entries;
};