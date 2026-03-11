#pragma once
#include "Chess/board_features.h"
#include "Chess/piece_type.h"
#include "Core/Engine/utils.h"
#include "MoveGen/move_list.h"
#include <stdexcept>

class ChessBitboardFeatureIterator
{
public:
	inline static constexpr size_t WHITE_PIECES_START = 0;
	inline static constexpr size_t NUM_SIDE_FEATURES = static_cast<size_t>(PIECE_TYPE_NONE);
	inline static constexpr size_t BLACK_PIECES_START = WHITE_PIECES_START + NUM_SIDE_FEATURES;
	inline static constexpr size_t EN_PASSANT_INDEX = BLACK_PIECES_START + NUM_SIDE_FEATURES;
	inline static constexpr size_t CASTLING_INDEX = EN_PASSANT_INDEX + 1;
	inline static constexpr size_t PIECE_MOVES_START = CASTLING_INDEX + 1;
	inline static constexpr size_t MOVELIST_MISC_START = PIECE_MOVES_START;
	inline static constexpr size_t NUM_PIECE_MOVE_FEATURES = static_cast<size_t>(PIECE_TYPE_NONE);
	inline static constexpr size_t PINMASK_INDEX = PIECE_MOVES_START + NUM_PIECE_MOVE_FEATURES;
	inline static constexpr size_t CHECKMASK_INDEX = PINMASK_INDEX + 1;
	inline static constexpr size_t PINNERS_INDEX = CHECKMASK_INDEX + 1;
	inline static constexpr size_t CHECKERS_INDEX = PINNERS_INDEX + 1;
	inline static constexpr size_t ATTACKED_SQUARES_INDEX = CHECKERS_INDEX + 1;
	inline static constexpr size_t NUM_MOVELIST_MISC_FEATURES = ATTACKED_SQUARES_INDEX - MOVELIST_MISC_START + 1;

	forceinline constexpr ChessBitboardFeatureIterator(const BoardFeatures& boardFeatures, const MoveListMiscellaneous& moveListMiscellaneous);

	template<size_t index>
	forceinline constexpr Bitboard Get(const float unused) const;

	forceinline constexpr Bitboard Get(const size_t index) const;

	static consteval size_t NumBitboardFeatures();

private:
	const BoardFeatures& m_BoardFeatures;
	const MoveListMiscellaneous& m_MoveListMiscellaneous;
};


forceinline constexpr ChessBitboardFeatureIterator::ChessBitboardFeatureIterator(const BoardFeatures& boardFeatures, const MoveListMiscellaneous& moveListMiscellaneous) :
	m_BoardFeatures(boardFeatures),
	m_MoveListMiscellaneous(moveListMiscellaneous)
{}

consteval size_t ChessBitboardFeatureIterator::NumBitboardFeatures()
{
	return ATTACKED_SQUARES_INDEX + 1;
}

forceinline constexpr Bitboard ChessBitboardFeatureIterator::Get(const size_t index) const
{
	DEBUG_ASSERT(index < NumBitboardFeatures());
	// bleh
	switch (index)
	{
	case 0: case 1: case 2: case 3: case 4: case 5:
	{
		const auto pieceType = static_cast<PieceType>(index - WHITE_PIECES_START);
		return m_BoardFeatures.WhitePieces->GetPieceBitboard(pieceType);
	}
	case 6: case 7: case 8: case 9: case 10: case 11:
	{
		const auto pieceType = static_cast<PieceType>(index - BLACK_PIECES_START);
		return m_BoardFeatures.BlackPieces->GetPieceBitboard(pieceType);
	}
	case EN_PASSANT_INDEX:
		return m_BoardFeatures.EnPassantSquare;
	case CASTLING_INDEX:
		return m_BoardFeatures.Castling.CurrentCastlingPermissions;
	case 14: case 15: case 16: case 17: case 18: case 19:
	{
		const int pieceTypeIndex = static_cast<int>(index - PIECE_MOVES_START);
		DEBUG_IF(pieceTypeIndex >= PIECE_TYPE_NONE)
		{
			throw std::runtime_error("invalid index for piece moves bitboard");
		}
		return m_MoveListMiscellaneous.PieceMoves[pieceTypeIndex];
	}
	case PINMASK_INDEX:
		return m_MoveListMiscellaneous.Pinmask;
	case CHECKMASK_INDEX:
		return m_MoveListMiscellaneous.Checkmask;
	case PINNERS_INDEX:
		return m_MoveListMiscellaneous.Pinners;
	case CHECKERS_INDEX:
		return m_MoveListMiscellaneous.Checkers;
	case ATTACKED_SQUARES_INDEX:
		return m_MoveListMiscellaneous.AttackedSquares;
#ifdef _DEBUG
	default:
		return 0ULL;
#endif
	}
}

// msvc compiler cannot compile this function for some reason if it's duck typed and it takes no parameters
// error "unexpected token ')' following 'simple_declaration'"
// adding the unused parameter and shutting up the warning about it being unused is a workaround
// please save me from this hell
#pragma warning( push )
#pragma warning( disable:4100)
template<size_t index>
forceinline constexpr Bitboard ChessBitboardFeatureIterator::Get(const float unused) const
{
	static_assert(index < NumBitboardFeatures());
	if constexpr (index < BLACK_PIECES_START)
	{
		const auto pieceType = static_cast<PieceType>(index - WHITE_PIECES_START);
		return m_BoardFeatures.WhitePieces->GetPieceBitboard(pieceType);
	}
	else if constexpr (index < EN_PASSANT_INDEX)
	{
		const auto pieceType = static_cast<PieceType>(index - BLACK_PIECES_START);
		return m_BoardFeatures.BlackPieces->GetPieceBitboard(pieceType);
	}
	else if constexpr (index == EN_PASSANT_INDEX)
	{
		return m_BoardFeatures.EnPassantSquare;
	}
	else if constexpr (index == CASTLING_INDEX)
	{
		return m_BoardFeatures.Castling.CurrentCastlingPermissions;
	}
	else if constexpr (index < PINMASK_INDEX)
	{
		return m_MoveListMiscellaneous.PieceMoves[index - PIECE_MOVES_START];
	}
	else if constexpr (index == PINMASK_INDEX)
	{
		return m_MoveListMiscellaneous.Pinmask;
	}
	else if constexpr (index == CHECKMASK_INDEX)
	{
		return m_MoveListMiscellaneous.Checkmask;
	}
	else if constexpr (index == PINNERS_INDEX)
	{
		return m_MoveListMiscellaneous.Pinners;
	}
	else if constexpr (index == CHECKERS_INDEX)
	{
		return m_MoveListMiscellaneous.Checkers;
	}
	else if constexpr (index == ATTACKED_SQUARES_INDEX)
	{
		return m_MoveListMiscellaneous.AttackedSquares;
	}
	else
	{
		return 0ULL;
	}
}
#pragma warning( pop )
