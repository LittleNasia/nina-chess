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
consteval void validate_piece()
{
	static_assert(piece < PIECE_NONE);
}

constexpr char piece_names[] =
{
	'P','N','B','R','Q','K',
	'p','n','b','r','q','k',
};

template<PieceType piece_type, Color color>
forceinline constexpr Piece get_piece_from_type()
{
	validate_color<color>();
	validate_piece_type<piece_type>();
	if constexpr (piece_type == KING && color == WHITE)
	{
		return WHITE_KING;
	}
	else if constexpr (piece_type == KNIGHT && color == WHITE)
	{
		return WHITE_KNIGHT;
	}
	else if constexpr (piece_type == BISHOP && color == WHITE)
	{
		return WHITE_BISHOP;
	}
	else if constexpr (piece_type == PAWN && color == WHITE)
	{
		return WHITE_PAWN;
	}
	else if constexpr (piece_type == ROOK && color == WHITE)
	{
		return WHITE_ROOK;
	}
	else if constexpr (piece_type == QUEEN && color == WHITE)
	{
		return WHITE_QUEEN;
	}
	else if constexpr (piece_type == KING && color == BLACK)
	{
		return BLACK_KING;
	}
	else if constexpr (piece_type == PAWN && color == BLACK)
	{
		return BLACK_PAWN;
	}
	else if constexpr (piece_type == KNIGHT && color == BLACK)
	{
		return BLACK_KNIGHT;
	}
	else if constexpr (piece_type == BISHOP && color == BLACK)
	{
		return BLACK_BISHOP;
	}
	else if constexpr (piece_type == ROOK && color == BLACK)
	{
		return BLACK_ROOK;
	}
	else
	{
		return BLACK_QUEEN;
	}
}