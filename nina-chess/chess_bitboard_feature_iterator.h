#pragma once
#include "utils.h"

#include "board_features.h"
#include "move_list.h"

class ChessBitboardFeatureIterator
{
public:
	forceinline constexpr ChessBitboardFeatureIterator(const BoardFeatures& boardFeatures, const MoveListMiscellaneous& moveListMiscellaneous) : 
		m_BoardFeatures(boardFeatures),
		m_MoveListMiscellaneous(moveListMiscellaneous)
	{}

	static consteval size_t NumBitboardFeatures()
	{
		return 
			static_cast<size_t>(PIECE_TYPE_NONE) * 2 + // white and black pieces
			2 +   // EP_square and castling
			6 +  // piece_moves
			4 + // pinmask, checkmask, pinners, checkers
			1; // attacked_squares
	}

// msvc compiler cannot compile this function for some reason if it's duck typed and it takes no parameters
// error "unexpected token ')' following 'simple_declaration'"
// adding the unused parameter and shutting up the warning about it being unused is a workaround
// please save me from this hell
#pragma warning( push )
#pragma warning( disable:4100)  
	template<size_t index>
	forceinline constexpr Bitboard Get(const float unused) const
	{
		static_assert(index < NumBitboardFeatures());
		if constexpr (index < 6)
		{
			const auto pieceType = static_cast<PieceType>(index);
			return m_BoardFeatures.WhitePieces->GetPieceBitboard(pieceType);
		}
		else if constexpr (index < 12)
		{
			const auto pieceType = static_cast<PieceType>(index - 6);
			return m_BoardFeatures.BlackPieces->GetPieceBitboard(pieceType);
		}
		else if constexpr (index == 12)
		{
			return m_BoardFeatures.EnPassantSquare;
		}
		else if constexpr (index == 13)
		{
			return m_BoardFeatures.Castling.CastlingPermissionsBitmask;
		}
		else if constexpr (index < 20)
		{
			return m_MoveListMiscellaneous.PieceMoves[index - 14];
		}
		else if constexpr (index == 20)
		{
			return m_MoveListMiscellaneous.Pinmask;
		}
		else if constexpr (index == 21)
		{
			return m_MoveListMiscellaneous.Checkmask;
		}
		else if constexpr (index == 22)
		{
			return m_MoveListMiscellaneous.Pinners;
		}
		else if constexpr (index == 23)
		{
			return m_MoveListMiscellaneous.Checkers;
		}
		else if constexpr (index == 24)
		{
			return m_MoveListMiscellaneous.AttackedSquares;
		}
		else
		{
			return 0ULL;
		}
	}
#pragma warning( pop )

	forceinline constexpr Bitboard Get(const size_t index) const
	{
		DEBUG_ASSERT(index < NumBitboardFeatures());
		// bleh
		switch (index)
		{
		case 0:
		case 1:
		case 2:
		case 3:
		case 4:
		case 5:
		{
			const auto pieceType = static_cast<PieceType>(index);
			return m_BoardFeatures.WhitePieces->GetPieceBitboard(pieceType);
		}
		case 6:
		case 7:
		case 8:
		case 9:
		case 10:
		case 11:
		{
			const auto pieceType = static_cast<PieceType>(index - 6);
			return m_BoardFeatures.BlackPieces->GetPieceBitboard(pieceType);
		}
		case 12:
			return m_BoardFeatures.EnPassantSquare;
		case 13:
			return m_BoardFeatures.Castling.CastlingPermissionsBitmask;
		case 14:
		case 15:
		case 16:
		case 17:
		case 18:
		case 19:
			return m_MoveListMiscellaneous.PieceMoves[index - 14];
		case 20:
			return m_MoveListMiscellaneous.Pinmask;
		case 21:
			return m_MoveListMiscellaneous.Checkmask;
		case 22:
			return m_MoveListMiscellaneous.Pinners;
		case 23:
			return m_MoveListMiscellaneous.Checkers;
		case 24:
			return m_MoveListMiscellaneous.AttackedSquares;
#ifdef _DEBUG
		default:
			return 0ULL;
#endif
		}
	}

private:
	const BoardFeatures& m_BoardFeatures;
	const MoveListMiscellaneous& m_MoveListMiscellaneous;
};