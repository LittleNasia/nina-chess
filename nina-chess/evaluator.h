#pragma once
#include "utils.h"

#include "position.h"

class Evaluator
{
public:
	Evaluator();

	Score Evaluate(const Position& position);
};