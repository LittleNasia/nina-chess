#pragma once
#include "utils.h"

#include "color.h"

using Castling = uint32_t;

template<Color color, Castling isCastling>
forceinline constexpr Castling GetCastling()
{
	static_assert(isCastling <= 0b1111);
	ValidateColor<color>();
	if constexpr (color == WHITE)
	{
		return isCastling & 0b11;
	}
	else
	{
		return (isCastling & 0b1100) >> 2;
	}
}

// bitmask of the path of king during castling
template<Color color>
forceinline constexpr Bitboard KingsideCastlingKingPath()
{
	ValidateColor<color>();
	if constexpr (color == WHITE)
	{
		return 0xe;
	}
	else
	{
		return 0xe00000000000000;
	}
}

// bitmask of the path of king during castling
template<Color color>
forceinline constexpr Bitboard KingsideCastlingRookPath()
{
	ValidateColor<color>();
	if constexpr (color == WHITE)
	{
		return 0x6;
	}
	else
	{
		return 0x600000000000000;
	}
}

// bitmask of the path of king during castling
template<Color color>
forceinline constexpr Bitboard QueensideCastlingKingPath()
{
	ValidateColor<color>();
	if constexpr (color == WHITE)
	{
		return 0x38;
	}
	else
	{
		return 0x3800000000000000;
	}
}

// bitmask of the path of king during castling
template<Color color>
forceinline constexpr Bitboard QueensideCastlingRookPath()
{
	ValidateColor<color>();
	if constexpr (color == WHITE)
	{
		return 0x78;
	}
	else
	{
		return 0x7800000000000000;
	}
}

template<Color color>
forceinline constexpr Bitboard KingsideCastlingRookBitmask()
{
	ValidateColor<color>();
	if constexpr (color == WHITE)
	{
		return 0x1;
	}
	else
	{
		return 0x100000000000000;
	}
}

template<Color color>
forceinline constexpr Bitboard QueensideCastlingRookBitmask()
{
	ValidateColor<color>();
	if constexpr (color == WHITE)
	{
		return 0x80;
	}
	else
	{
		return 0x8000000000000000;
	}
}

template<Color color>
forceinline constexpr Bitboard KingsideCastlingKingDestination()
{
	ValidateColor<color>();
	if constexpr (color == WHITE)
	{
		return 0x2;
	}
	else
	{
		return 0x200000000000000;
	}
}

template<Color color>
forceinline constexpr Bitboard QueensideCastlingKingDestination()
{
	ValidateColor<color>();
	if constexpr (color == WHITE)
	{
		return 0x20;
	}
	else
	{
		return 0x2000000000000000;
	}
}

template<Color color>
forceinline constexpr Bitboard QueensideCastlingRookDestination()
{
	ValidateColor<color>();
	if constexpr (color == WHITE)
	{
		return 0x10;
	}
	else
	{
		return 0x1000000000000000;
	}
}

template<Color color>
forceinline constexpr Bitboard KingsideCastlingRookDestination()
{
	ValidateColor<color>();
	if constexpr (color == WHITE)
	{
		return 0x4;
	}
	else
	{
		return 0x400000000000000;
	}
}

forceinline constexpr Castling UpdateCastlingRights(const Bitboard white_rooks, const Bitboard black_rooks)
{
	Castling castling_perms = 0b1111;
	if (!(KingsideCastlingRookBitmask<WHITE>() & white_rooks))
	{
		castling_perms &= ~0b0001;
	}
	if (!(QueensideCastlingRookBitmask<WHITE>() & white_rooks))
	{
		castling_perms &= ~0b0010;
	}
	if (!(KingsideCastlingRookBitmask<BLACK>() & black_rooks))
	{
		castling_perms &= ~0b0100;
	}
	if (!(QueensideCastlingRookBitmask<BLACK>() & black_rooks))
	{
		castling_perms &= ~0b1000;
	}
	return castling_perms;
}

template<Color color>
forceinline constexpr Castling CastlingPermissionsBitmask()
{
	ValidateColor<color>();
	if constexpr (color == WHITE)
	{
		return 0b11;
	}
	else
	{
		return 0b1100;
	}
}

template<Color color>
forceinline constexpr Castling KingsideCastlingPermissionsBitmask()
{
	ValidateColor<color>();
	if constexpr (color == WHITE)
	{
		return 0b1;
	}
	else
	{
		return 0b100;
	}
}

template<Color color>
forceinline constexpr Castling QueensideCastlingPermissionsBitmask()
{
	ValidateColor<color>();
	if constexpr (color == WHITE)
	{
		return 0b10;
	}
	else
	{
		return 0b1000;
	}
}

