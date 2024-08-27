#include "transposition_table.h"

forceinline constexpr uint32_t fast_modulo(const size_t input, const size_t ceil)
{
	return ((input >> 32) * (ceil)) >> 32;
}

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
