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


constexpr char piece_names[] =
{
	'P','N','B','R','Q','K',
	'p','n','b','r','q','k',
};

template<PieceType piece, Color color>
forceinline constexpr Piece get_piece_from_type()
{
	if constexpr (piece == KING && color == WHITE)
	{
		return WHITE_KING;
	}
	else if constexpr (piece == KNIGHT && color == WHITE)
	{
		return WHITE_KNIGHT;
	}
	else if constexpr (piece == BISHOP && color == WHITE)
	{
		return WHITE_BISHOP;
	}
	else if constexpr (piece == PAWN && color == WHITE)
	{
		return WHITE_PAWN;
	}
	else if constexpr (piece == ROOK && color == WHITE)
	{
		return WHITE_ROOK;
	}
	else if constexpr (piece == QUEEN && color == WHITE)
	{
		return WHITE_QUEEN;
	}
	else if constexpr (piece == KING && color == BLACK)
	{
		return BLACK_KING;
	}
	else if constexpr (piece == PAWN && color == BLACK)
	{
		return BLACK_PAWN;
	}
	else if constexpr (piece == KNIGHT && color == BLACK)
	{
		return BLACK_KNIGHT;
	}
	else if constexpr (piece == BISHOP && color == BLACK)
	{
		return BLACK_BISHOP;
	}
	else if constexpr (piece == ROOK && color == BLACK)
	{
		return BLACK_ROOK;
	}
	else
	{
		return BLACK_QUEEN;
	}
}