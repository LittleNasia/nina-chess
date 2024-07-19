#pragma once
#include "utils.h"

#include "evaluator.h"
#include "move_list.h"
#include "position.h"

class Board
{
public:
	Board(Evaluator* evaluator, uint64_t* hash_history);
	Board(const Position position, Evaluator* evaluator);

	Board MakeMove(const Move move) const;
	template<Color side_to_move>
	forceinline Board MakeMove(const Move move) const;

	Score Evaluate(const MoveList& move_list) const;

	const Position& GetPosition() const { return position; }
	const Evaluator* GetEvaluator() const { return evaluator; }
private:
	Board(Evaluator* evaluator, const Position&& position);

	Evaluator* evaluator;
	Position position;
};

#include "board.inl"