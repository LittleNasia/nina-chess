#include "board.h"

#include <stdexcept>

Board::Board(Evaluator* evaluator):
	evaluator(evaluator),
	position()
{
}

Board Board::MakeMove(const Move move) const
{
	return Board(evaluator, position::MakeMove(position, move));
}

Score Board::Evaluate(const MoveList& move_list) const
{
	DEBUG_IF(evaluator == nullptr)
	{
		throw std::runtime_error("Evaluator is not set");
	}
	return evaluator->Evaluate(position);
}

Board::Board(Evaluator* evaluator, const Position&& position):
	evaluator(evaluator),
	position(position)
{
}
