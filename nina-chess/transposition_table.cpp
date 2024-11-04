#include "transposition_table.h"

TranspositionTable::TranspositionTable(const size_t sizeInMb)
{
	const size_t sizeInBytes = sizeInMb * 1024 * 1024;
	const size_t entryCount = sizeInBytes / sizeof(TranspositionTableEntry);
	m_Entries.resize(entryCount);
}

void TranspositionTable::Insert(const TranspositionTableEntry& entry, const bool forceOverwrite)
{
	const size_t index = FastModulo(entry.Key, m_Entries.size());
	
	const auto& currrentEntry = m_Entries[index];
	if ((currrentEntry.Depth <= entry.Depth) || forceOverwrite)
	{
		m_Entries[index] = entry;
	}
}

const TranspositionTableEntry& TranspositionTable::Get(const uint64_t key) const
{
	const size_t index = FastModulo(key, m_Entries.size());

	return m_Entries[index];
}

void TranspositionTable::Serialize(std::ofstream& output)
{
	size_t entryCount = m_Entries.size();
	output.write((char*)&entryCount, sizeof(entryCount));
	for (const auto& entry : m_Entries)
	{
		output.write((char*)&entry, sizeof(entry));
	}
}

void TranspositionTable::Deserialize(std::ifstream& input)
{
	size_t entryCount;
	input.read((char*)&entryCount, sizeof(entryCount));
	m_Entries.resize(entryCount);
	for (auto& entry : m_Entries)
	{
		input.read((char*)&entry, sizeof(entry));
	}
}
