#pragma once
#include "utils.h"

#include "move_list.h"
#include "transposition_table.h"

struct SearchInfo
{
	int depth = 0;
	int remaining_depth = 0;
	MoveList* move_lists;
	TranspositionTable& tt;

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
	forceinline constexpr MoveList& GetMoveList() const { return move_lists[depth]; }
};