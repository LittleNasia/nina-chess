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
	forceinline TranspositionTable(const size_t sizeInMb);

	forceinline void Insert(const TranspositionTableEntry& entry, const bool forceOverwrite);
	forceinline const TranspositionTableEntry& Get(const uint64_t key) const;

	forceinline void Serialize(std::ofstream& output);
	forceinline void Deserialize(std::ifstream& input);
private:
	std::vector<TranspositionTableEntry> m_Entries;
};

forceinline TranspositionTable::TranspositionTable(const size_t sizeInMb)
{
	const size_t sizeInBytes = sizeInMb * 1024 * 1024;
	const size_t entryCount = sizeInBytes / sizeof(TranspositionTableEntry);
	m_Entries.resize(entryCount);
}

forceinline void TranspositionTable::Insert(const TranspositionTableEntry& entry, const bool forceOverwrite)
{
	const size_t index = FastModulo(entry.Key, m_Entries.size());

	const auto& currrentEntry = m_Entries[index];
	if ((currrentEntry.Depth <= entry.Depth) || forceOverwrite)
	{
		m_Entries[index] = entry;
	}
}

forceinline const TranspositionTableEntry& TranspositionTable::Get(const uint64_t key) const
{
	const size_t index = FastModulo(key, m_Entries.size());

	return m_Entries[index];
}

forceinline void TranspositionTable::Serialize(std::ofstream& output)
{
	size_t entryCount = m_Entries.size();
	output.write((char*)&entryCount, sizeof(entryCount));
	for (const auto& entry : m_Entries)
	{
		output.write((char*)&entry, sizeof(entry));
	}
}

forceinline void TranspositionTable::Deserialize(std::ifstream& input)
{
	size_t entryCount;
	input.read((char*)&entryCount, sizeof(entryCount));
	m_Entries.resize(entryCount);
	for (auto& entry : m_Entries)
	{
		input.read((char*)&entry, sizeof(entry));
	}
}