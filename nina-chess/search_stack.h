#pragma once
#include "utils.h"

#include "move_list.h"
#include "position.h"
#include "transposition_table.h"

struct SearchStack
{
	int depth = 0;
	int remaining_depth = 0;
	size_t nodes = 0;

	MoveList* move_list_stack;
	Position* position_stack;
	uint64_t* hash_stack;

	TranspositionTable* tt;

	forceinline constexpr TranspositionTable& GetTranspositionTable() const { return *tt; }

	forceinline constexpr void IncrementDepth()
	{
		++depth;
		--remaining_depth;
	}
	forceinline constexpr void DecrementDepth()
	{
		--depth;
		++remaining_depth;
	}

	forceinline constexpr const Position& GetCurrentPosition() const { return position_stack[depth]; }

	forceinline constexpr Position& GetNextPosition() { return position_stack[depth + 1]; }

	forceinline constexpr MoveList& GetMoveList() { return move_list_stack[depth]; }

	forceinline constexpr void SetCurrentPositionHash() { hash_stack[depth] = GetCurrentPosition().hash; }
};