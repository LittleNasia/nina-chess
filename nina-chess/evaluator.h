#pragma once
#include "utils.h"

#include "move_list.h"
#include "position.h"
#include "search_stack.h"

class Evaluator
{
public:
	Evaluator();

	Score Evaluate(const Position& position, const MoveList& move_list, const SearchStack& search_stack);
};