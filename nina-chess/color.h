#pragma once
#include "utils.h"

enum Color : uint8_t
{
	WHITE,
	BLACK,
	COLOR_NONE
};

template<Color color>
consteval void ValidateColor()
{
	static_assert(color < COLOR_NONE);
}

forceinline constexpr void ValidateColor(const Color color)
{
	DEBUG_ASSERT(color < COLOR_NONE);
}

template<Color color>
forceinline constexpr Color GetOppositeColor()
{
	ValidateColor<color>();
	if constexpr (color == WHITE)
	{
		return BLACK;
	}
	if constexpr (color == BLACK)
	{
		return WHITE;
	}
}

forceinline constexpr Color GetOppositeColor(const Color color)
{
	ValidateColor(color);
	if (color == WHITE)
	{
		return BLACK;
	}
	else if (color == BLACK)
	{
		return WHITE;
	}
}