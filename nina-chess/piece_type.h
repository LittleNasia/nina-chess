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

template<PieceType pieceType>
consteval void ValidatePieceType()
{
	static_assert(pieceType < PIECE_TYPE_NONE);
}

forceinline constexpr void ValidatePieceType(const PieceType pieceType)
{
	DEBUG_ASSERT(pieceType < PIECE_TYPE_NONE);
}

forceinline constexpr PieceType operator++(PieceType& pieceType, int)
{
	ValidatePieceType(pieceType);
	return pieceType = static_cast<PieceType>(pieceType + 1);
}

inline constexpr PieceType PROMOTION_PIECE_TYPES[4] =
{
	KNIGHT,
	BISHOP,
	ROOK,
	QUEEN
};