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

template<PieceType piece_type>
consteval void validate_piece_type()
{
	static_assert(piece_type < PIECE_TYPE_NONE);
}

forceinline constexpr void validate_piece_type(const PieceType piece_type)
{
	DEBUG_ASSERT(piece_type < PIECE_TYPE_NONE);
}

forceinline constexpr PieceType operator++(PieceType& piece_type, int)
{
	validate_piece_type(piece_type);
	return piece_type = static_cast<PieceType>(piece_type + 1);
}

inline constexpr PieceType promotion_pieces[4] =
{
	KNIGHT,
	BISHOP,
	ROOK,
	QUEEN
};