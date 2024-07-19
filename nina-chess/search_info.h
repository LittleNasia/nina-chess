#pragma once
#include "utils.h"

#include "move_list.h"

struct SearchInfo
{
	int depth = 0;
	MoveList* move_lists;

	static SearchInfo Next(const SearchInfo& search_info)
	{
		return { search_info.depth + 1, search_info.move_lists };
	}

	MoveList& GetMoveList() const { return move_lists[depth]; }
};