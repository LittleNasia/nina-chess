#pragma once
#include "utils.h"

enum Color : uint8_t
{
	WHITE,
	BLACK,
	COLOR_NONE
};


template<Color color>
forceinline constexpr Color get_opposite_color()
{
	if constexpr (color == WHITE)
	{
		return BLACK;
	}
	if constexpr (color == BLACK)
	{
		return WHITE;
	}
}

forceinline constexpr Color get_opposite_color(const Color color)
{
	if (color == WHITE)
	{
		return BLACK;
	}
	else if (color == BLACK)
	{
		return WHITE;
	}
}