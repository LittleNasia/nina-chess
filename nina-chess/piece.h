#pragma once
#include "utils.h"

#include "piece_type.h"

enum Piece : uint32_t
{
	WHITE_PAWN,
	WHITE_KNIGHT,
	WHITE_BISHOP,
	WHITE_ROOK,
	WHITE_QUEEN,
	WHITE_KING,

	BLACK_PAWN,
	BLACK_KNIGHT,
	BLACK_BISHOP,
	BLACK_ROOK,
	BLACK_QUEEN,
	BLACK_KING,

	PIECE_NONE
};

template<Piece piece>
consteval void ValidatePiece()
{
	static_assert(piece < PIECE_NONE);
}

constexpr char PIECE_NAMES[] =
{
	'P','N','B','R','Q','K',
	'p','n','b','r','q','k',
};

template<PieceType pieceType, Color color>
forceinline constexpr Piece GetPieceFromPieceType()
{
	ValidateColor<color>();
	ValidatePieceType<pieceType>();
	if constexpr (pieceType == KING && color == WHITE)
	{
		return WHITE_KING;
	}
	else if constexpr (pieceType == KNIGHT && color == WHITE)
	{
		return WHITE_KNIGHT;
	}
	else if constexpr (pieceType == BISHOP && color == WHITE)
	{
		return WHITE_BISHOP;
	}
	else if constexpr (pieceType == PAWN && color == WHITE)
	{
		return WHITE_PAWN;
	}
	else if constexpr (pieceType == ROOK && color == WHITE)
	{
		return WHITE_ROOK;
	}
	else if constexpr (pieceType == QUEEN && color == WHITE)
	{
		return WHITE_QUEEN;
	}
	else if constexpr (pieceType == KING && color == BLACK)
	{
		return BLACK_KING;
	}
	else if constexpr (pieceType == PAWN && color == BLACK)
	{
		return BLACK_PAWN;
	}
	else if constexpr (pieceType == KNIGHT && color == BLACK)
	{
		return BLACK_KNIGHT;
	}
	else if constexpr (pieceType == BISHOP && color == BLACK)
	{
		return BLACK_BISHOP;
	}
	else if constexpr (pieceType == ROOK && color == BLACK)
	{
		return BLACK_ROOK;
	}
	else
	{
		return BLACK_QUEEN;
	}
}