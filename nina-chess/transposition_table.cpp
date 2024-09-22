#include "transposition_table.h"

TranspositionTable::TranspositionTable(const size_t size_in_mb)
{
	const size_t size_in_bytes = size_in_mb * 1024 * 1024;
	const size_t entry_count = size_in_bytes / sizeof(TranspositionTableEntry);
	entries.resize(entry_count);
}

void TranspositionTable::Insert(const TranspositionTableEntry& entry, bool force_overwrite)
{
	const size_t index = fast_modulo(entry.key, entries.size());
	
	const auto& curr_entry = entries[index];
	if ((curr_entry.depth <= entry.depth) || force_overwrite)
	{
		entries[index] = entry;
	}
}

const TranspositionTableEntry& TranspositionTable::Get(const uint64_t key) const
{
	const size_t index = fast_modulo(key, entries.size());

	return entries[index];
}

void TranspositionTable::Serialize(std::ofstream& output)
{
	size_t entry_count = entries.size();
	output.write((char*)&entry_count, sizeof(entry_count));
	for (const auto& entry : entries)
	{
		output.write((char*)&entry, sizeof(entry));
	}
}

void TranspositionTable::Deserialize(std::ifstream& input)
{
	size_t entry_count;
	input.read((char*)&entry_count, sizeof(entry_count));
	entries.resize(entry_count);
	for (auto& entry : entries)
	{
		input.read((char*)&entry, sizeof(entry));
	}
}
