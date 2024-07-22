#pragma once
#include "utils.h"

#include "evaluator.h"
#include "move_list.h"
#include "position.h"
#include "search_stack.h"

class Board
{
public:
	Board(Position& position, Evaluator* evaluator);

	template<Color side_to_move>
	forceinline Board MakeMove(const Move& move, SearchStack& search_stack) const;
				Board MakeMove(const Move& move, SearchStack& search_stack) const;

	Score Evaluate(const MoveList& move_list, const SearchStack& search_stack) const;

	const Position& GetPosition()			const { return *position; }
	const Evaluator& GetEvaluator()			const { return *evaluator; }
	bool IsDrawn(SearchStack& search_stack) const { return IsThreefoldRepetition(search_stack) || position->IsDrawn(); }

private:
	bool IsThreefoldRepetition(SearchStack& search_stack) const;

	Evaluator* evaluator;
	Position* position;
};

#include "board.inl"