#pragma once
#include "Eval/score.h"
#include "Core/Engine/utils.h"

struct AlphaBeta
{
	Score Alpha;
	const Score Beta;
	forceinline constexpr AlphaBeta Invert() const { return { -Beta, -Alpha }; }
};
