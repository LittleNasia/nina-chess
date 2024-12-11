#pragma once
#include "score.h"
#include "utils.h"

struct AlphaBeta
{
	Score Alpha;
	const Score Beta;
	forceinline constexpr AlphaBeta Invert() const { return { -Beta, -Alpha }; }
};
