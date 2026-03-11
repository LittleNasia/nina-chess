#pragma once
#include "Core/Engine/utils.h"
#include "Eval/score.h"

struct AlphaBeta
{
	Score Alpha;
	const Score Beta;
	forceinline constexpr AlphaBeta Invert() const { return { -Beta, -Alpha }; }
};
