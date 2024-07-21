#include "move_gen.h"

template<Color side_to_move>
forceinline Board Board::MakeMove(const Move& move, SearchStack& search_stack) const
{
	Position& new_position = search_stack.GetNextPosition();
	position::MakeMove<side_to_move>(*position, new_position, move);
	return Board(new_position, evaluator);
}