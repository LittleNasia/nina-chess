#pragma once
#include "move.h"
#include "score.h"
#include "utils.h"

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

class TranspositionTable
{
public:
	forceinline TranspositionTable(const size_t sizeInMb);

	forceinline constexpr void Insert(const TranspositionTableEntry& entry, const bool forceOverwrite);
	forceinline constexpr const TranspositionTableEntry& Get(const uint64_t key) const;

private:
	std::vector<TranspositionTableEntry> m_Entries;
};

forceinline TranspositionTable::TranspositionTable(const size_t sizeInMb)
{
	const size_t sizeInBytes = sizeInMb * 1024 * 1024;
	const size_t entryCount = sizeInBytes / sizeof(TranspositionTableEntry);
	m_Entries.resize(entryCount);
}

forceinline constexpr void TranspositionTable::Insert(const TranspositionTableEntry& entry, const bool forceOverwrite)
{
	const size_t index = FastModulo(entry.Key, m_Entries.size());

	const auto& currrentEntry = m_Entries[index];
	if ((currrentEntry.Depth <= entry.Depth) || forceOverwrite)
	{
		m_Entries[index] = entry;
	}
}

forceinline constexpr const TranspositionTableEntry& TranspositionTable::Get(const uint64_t key) const
{
	const size_t index = FastModulo(key, m_Entries.size());

	return m_Entries[index];
}
