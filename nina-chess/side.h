#pragma once
#include "utils.h"

struct Side
{
	forceinline constexpr Side() :
		pawns(0ULL), knights(0ULL), bishops(0ULL),
		rooks(0ULL), queens(0ULL), king(0ULL),
		pieces(0ULL)
	{

	}
	forceinline constexpr Side(const Bitboard pawns, const Bitboard knights, const Bitboard bishops,
		const Bitboard rooks, const Bitboard queens, const Bitboard king) :
		pawns(pawns), knights(knights), bishops(bishops),
		rooks(rooks), queens(queens), king(king),
		pieces(pawns | knights | bishops | rooks | queens | king)
	{

	}
	Bitboard pawns = 0;
	Bitboard knights = 0;
	Bitboard bishops = 0;
	Bitboard rooks = 0;
	Bitboard queens = 0;
	Bitboard king = 0;

	constexpr Bitboard& get_piece_bb(const PieceType piece_type)
	{
		return (&pawns)[piece_type];
	}

	forceinline constexpr void remove_pieces(const Bitboard piece)
	{
		pawns &= ~piece;
		knights &= ~piece;
		bishops &= ~piece;
		rooks &= ~piece;
		queens &= ~piece;
	}

	const Bitboard pieces = 0;
};