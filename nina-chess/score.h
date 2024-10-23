#pragma once
#include "utils.h"

enum class Score : int32_t
{
	NEGATIVE_INF = -10001,
	LOSS = -1000,
	DRAW = 0,
	WIN = 1000,
	POSITIVE_INF = 10001,
	UNKNOWN = 10002
};

forceinline constexpr Score operator-(const Score score)
{
	return static_cast<Score>(-static_cast<int32_t>(score));
}

forceinline constexpr Score operator+(const Score left, const Score right)
{
	return static_cast<Score>(static_cast<int32_t>(left) + static_cast<int32_t>(right));
}

forceinline std::ostream& operator<<(std::ostream& os, const Score score)
{
	os << static_cast<int32_t>(score);
	return os;
}

forceinline constexpr Score GetDrawValueWithSmallVariance(int64_t random_seed)
{
	return static_cast<Score>(random_seed & 7);
}

forceinline constexpr void ValidateScore(Score score)
{
	DEBUG_ASSERT(score > Score::NEGATIVE_INF && score < Score::POSITIVE_INF);
}