#pragma once
#include "utils.h"

#include "evaluator.h"
#include "position.h"

class Board
{
public:
	Board(Evaluator* evaluator);

	Board MakeMove(const Move move) const;
	Score Evaluate() const;
private:
	Board(Evaluator* evaluator, const Position&& position);

	Evaluator* evaluator;
	Position position;
};
