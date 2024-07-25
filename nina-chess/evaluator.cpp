#include "evaluator.h"

Evaluator::Evaluator() :
	depth{ 0 }
{
	Position startpos;
	MoveList startpos_move_list;
	generate_moves(startpos, startpos_move_list);

	Update<WHITE>(startpos, startpos_move_list);
}

void Evaluator::Reset(SearchStack& search_stack)
{
	depth = 0;

	for (int depth = 0; depth < search_stack.depth; depth++)
	{
		const auto& curr_position = search_stack.GetPositionAt(depth);
		const auto& curr_move_list = search_stack.GetMoveListAt(depth);

		if (curr_position.side_to_move == WHITE)
			IncrementalUpdate<WHITE>(curr_position, curr_move_list);
		else
			IncrementalUpdate<BLACK>(curr_position, curr_move_list);
	}
}
