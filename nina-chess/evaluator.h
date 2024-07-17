#pragma once
#include "utils.h"

#include "position.h"

enum class Score: int32_t
{
	NEGATIVE_INF = -1000000,
	LOSS = -10000,
	DRAW = 0,
	WIN = 10000,
	POSITIVE_INF = 1000000
};

class Evaluator
{
public:
	Evaluator();

	Score Evaluate(const Position& position);
};