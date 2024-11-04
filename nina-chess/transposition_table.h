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
	uint64_t Key = 0;
	Score Score = Score::DRAW;
	int64_t Depth = 0;
	Move BestMove;
	TTFlag Flag = TTFlag::EXACT;
};

class TranspositionTable : public Serializable
{
public:
	TranspositionTable(const size_t sizeInMb);

	void Insert(const TranspositionTableEntry& entry, const bool forceOverwrite);
	const TranspositionTableEntry& Get(const uint64_t key) const;

	void Serialize(std::ofstream& output);
	void Deserialize(std::ifstream& input);
private:
	std::vector<TranspositionTableEntry> m_Entries;
};