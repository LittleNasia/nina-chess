#include "board.h"

#include <stdexcept>

Board::Board(Position& position, Evaluator* evaluator):
	evaluator(evaluator),
	position(&position)
{
}

Board Board::MakeMove(const Move& move, SearchStack& search_stack) const
{
	Position& new_position = search_stack.GetNextPosition();
	position::MakeMove(*position, new_position, move);
	search_stack.SetNextPositionHash();
	return Board(new_position, evaluator);
}

Score Board::Evaluate(const MoveList& move_list, const SearchStack& search_stack) const
{
	DEBUG_IF(evaluator == nullptr)
	{
		throw std::runtime_error("Evaluator is not set");
	}
	return evaluator->Evaluate(*position, move_list, search_stack);
}

bool Board::IsThreefoldRepetition(SearchStack& search_stack) const
{
	const Position& current_position = search_stack.GetCurrentPosition();
	const int ply_to_search_to = search_stack.depth < int(current_position.fifty_move_rule) ? search_stack.depth : int(current_position.fifty_move_rule);

	for (int ply = search_stack.depth - 2; ply >= ply_to_search_to; ply -= 2)
	{
		if (current_position.hash == search_stack.GetHashAtPly(ply))
		{
			return true;
		}
	}
	return false;
}
