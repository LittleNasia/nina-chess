#pragma once
#include "piece_type.h"
#include "utils.h"

struct Side
{
	forceinline constexpr Side();
	forceinline constexpr Side(const Bitboard pawns, const Bitboard knights, const Bitboard bishops,
		const Bitboard rooks, const Bitboard queens, const Bitboard king);

	Bitboard Pawns = 0;
	Bitboard Knights = 0;
	Bitboard Bishops = 0;
	Bitboard Rooks = 0;
	Bitboard Queens = 0;
	Bitboard King = 0;
	Bitboard Pieces = 0;

	template<PieceType pieceType>
	forceinline constexpr Bitboard& GetPieceBitboard();

	forceinline constexpr Bitboard& GetPieceBitboard(const PieceType pieceType)		  { return (&Pawns)[pieceType]; }
	forceinline constexpr Bitboard  GetPieceBitboard(const PieceType pieceType) const { return (&Pawns)[pieceType]; }
	forceinline constexpr void RemovePieces(const Bitboard piece);
};


forceinline constexpr Side::Side() :
	Pawns(0ULL), Knights(0ULL), Bishops(0ULL),
	Rooks(0ULL), Queens(0ULL), King(0ULL),
	Pieces(0ULL)
{
}

forceinline constexpr Side::Side(const Bitboard pawns, const Bitboard knights, const Bitboard bishops,
	const Bitboard rooks, const Bitboard queens, const Bitboard king) :
	Pawns(pawns), Knights(knights), Bishops(bishops),
	Rooks(rooks), Queens(queens), King(king),
	Pieces(pawns | knights | bishops | rooks | queens | king)
{
}

template<PieceType pieceType>
forceinline constexpr Bitboard& Side::GetPieceBitboard()
{
	switch (pieceType)
	{
	case PAWN:   return Pawns;
	case KNIGHT: return Knights;
	case BISHOP: return Bishops;
	case ROOK:   return Rooks;
	case QUEEN:  return Queens;
	case KING:   return King;
#ifdef _DEBUG
	default:     return Pawns;
#endif
	}
}

forceinline constexpr void Side::RemovePieces(const Bitboard piece)
{
	Pawns &= ~piece;
	Knights &= ~piece;
	Bishops &= ~piece;
	Rooks &= ~piece;
	Queens &= ~piece;
	Pieces &= ~piece;
}
