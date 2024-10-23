#pragma once
#include "utils.h"

enum PieceType : uint32_t
{
	PAWN,
	KNIGHT,
	BISHOP,
	ROOK,
	QUEEN,
	KING,
	PIECE_TYPE_NONE
};

forceinline constexpr PieceType operator++(PieceType& piece, int)
{
	DEBUG_ASSERT(piece < PIECE_TYPE_NONE);
	return piece = static_cast<PieceType>(piece + 1);
}

inline constexpr PieceType promotion_pieces[4] =
{
	KNIGHT,
	BISHOP,
	ROOK,
	QUEEN
};