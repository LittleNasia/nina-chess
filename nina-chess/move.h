#pragma once
#include "bit_manip.h"
#include "chess_constants.h"
#include "move_type.h"
#include "utils.h"

struct Move
{
public:
	inline static constexpr uint32_t INDEX_FROM_OFFSET = 0;
	inline static constexpr uint32_t INDEX_FROM_BITMASK = 0b111111 << INDEX_FROM_OFFSET;

	inline static constexpr uint32_t INDEX_TO_OFFSET = INDEX_FROM_OFFSET + 6;
	inline static constexpr uint32_t INDEX_TO_BITMASK = 0b111111 << INDEX_TO_OFFSET;

	inline static constexpr uint32_t PIECE_OFFSET = INDEX_TO_OFFSET + 6;
	inline static constexpr uint32_t PIECE_BITMASK = 0b1111 << PIECE_OFFSET;

	inline static constexpr uint32_t PROMOTION_PIECE_OFFSET = PIECE_OFFSET + 4;
	inline static constexpr uint32_t PROMOTION_PIECE_BITMASK = 0b1111 << PROMOTION_PIECE_OFFSET;

	inline static constexpr uint32_t MOVE_TYPE_OFFSET = PROMOTION_PIECE_OFFSET + 4;
	inline static constexpr uint32_t MOVE_TYPE_BITMASK = 0b1111 << MOVE_TYPE_OFFSET;

	forceinline constexpr Move();
	forceinline constexpr Move(const uint32_t from, const uint32_t to, const PieceType piece, const MoveType moveType, const PieceType promotionPieceType = PIECE_TYPE_NONE);

	forceinline			  std::string ToUciMove()					const { return std::string(NAME_OF_SQUARE[BitIndex(FromBitmask())]) + NAME_OF_SQUARE[BitIndex(ToBitmask())] + GetUciPromotionPiece(); }
	forceinline constexpr Bitboard    FromBitmask()					const { return 1ULL << FromIndex(); }
	forceinline constexpr Bitboard    ToBitmask()					const { return 1ULL << ToIndex(); }
	forceinline constexpr uint32_t    FromIndex()					const { return (m_EncodedMove & INDEX_FROM_BITMASK); }
	forceinline constexpr uint32_t    ToIndex()						const { return (m_EncodedMove & INDEX_TO_BITMASK) >> INDEX_TO_OFFSET; }
	forceinline constexpr PieceType   MovingPieceType()				const { return static_cast<PieceType>((m_EncodedMove & PIECE_BITMASK) >> PIECE_OFFSET); }
	forceinline constexpr PieceType   PromotionPieceType()			const { return static_cast<PieceType>((m_EncodedMove & PROMOTION_PIECE_BITMASK) >> PROMOTION_PIECE_OFFSET); }
	forceinline constexpr MoveType    GetMoveType()					const { return static_cast<MoveType>((m_EncodedMove & MOVE_TYPE_BITMASK) >> MOVE_TYPE_OFFSET); }
	forceinline constexpr std::string GetUciPromotionPiece()		const;
	forceinline constexpr bool		  IsKingsideCastling()			const { return GetMoveType() == MoveType::KINGSIDE_CASTLING; }
	forceinline constexpr bool		  IsQueensideCastling()			const { return GetMoveType() == MoveType::QUEENSIDE_CASTLING; }
	forceinline constexpr bool		  operator==(const Move& other) const { return m_EncodedMove == other.m_EncodedMove; }

private:
	uint32_t m_EncodedMove;
};

forceinline constexpr Move::Move() : Move(0, 0, PIECE_TYPE_NONE, MoveType::NORMAL, PIECE_TYPE_NONE)
{}

forceinline constexpr Move::Move(const uint32_t from, const uint32_t to, const PieceType piece, const MoveType moveType, const PieceType promotionPieceType) :
	m_EncodedMove{
	(from << INDEX_FROM_OFFSET) |
	(to << INDEX_TO_OFFSET) |
	(piece << PIECE_OFFSET) |
	(static_cast<uint32_t>(moveType) << MOVE_TYPE_OFFSET) |
	(promotionPieceType << PROMOTION_PIECE_OFFSET)
	}
{}

constexpr std::string Move::GetUciPromotionPiece() const
{
	switch (PromotionPieceType())
	{
	case QUEEN:		return "q";
	case ROOK:		return "r";
	case BISHOP:	return "b";
	case KNIGHT:	return "n";
	default:		return "";
	}
}
