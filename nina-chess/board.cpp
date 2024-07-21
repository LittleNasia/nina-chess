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
