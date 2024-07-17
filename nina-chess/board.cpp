#include "board.h"

Board::Board(Evaluator* evaluator):
	evaluator(evaluator),
	position()
{
}

Board Board::MakeMove(const Move move) const
{
	return Board(evaluator, position::MakeMove(position, move));
}

Score Board::Evaluate() const
{
	return evaluator->Evaluate(position);
}

Board::Board(Evaluator* evaluator, const Position&& position):
	evaluator(evaluator),
	position(position)
{
}
