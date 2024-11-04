#pragma once
#include "utils.h"

struct Side
{
	forceinline constexpr Side() :
		Pawns(0ULL), Knights(0ULL), Bishops(0ULL),
		Rooks(0ULL), Queens(0ULL), King(0ULL),
		Pieces(0ULL)
	{
	}

	forceinline constexpr Side(const Bitboard pawns, const Bitboard knights, const Bitboard bishops,
		const Bitboard rooks, const Bitboard queens, const Bitboard king) :
		Pawns(pawns), Knights(knights), Bishops(bishops),
		Rooks(rooks), Queens(queens), King(king),
		Pieces(pawns | knights | bishops | rooks | queens | king)
	{
	}

	forceinline constexpr Bitboard& GetPieceBitboard(const PieceType pieceType)		  { return (&Pawns)[pieceType]; }
	forceinline constexpr Bitboard  GetPieceBitboard(const PieceType pieceType) const { return (&Pawns)[pieceType]; }

	forceinline constexpr void RemovePieces(const Bitboard piece)
	{
		Pawns &= ~piece;
		Knights &= ~piece;
		Bishops &= ~piece;
		Rooks &= ~piece;
		Queens &= ~piece;
		Pieces &= ~piece;
	}

	Bitboard Pawns = 0;
	Bitboard Knights = 0;
	Bitboard Bishops = 0;
	Bitboard Rooks = 0;
	Bitboard Queens = 0;
	Bitboard King = 0;
	Bitboard Pieces = 0;
};