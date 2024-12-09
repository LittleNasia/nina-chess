#pragma once
#include "utils.h"

#include "piece_type.h"

enum class MoveType
{
	NORMAL,
	CAPTURE,
	DOUBLE_PAWN_ADVANCE,
	EN_PASSANT,
	KINGSIDE_CASTLING,
	QUEENSIDE_CASTLING,
	PROMOTION_TO_QUEEN,
	PROMOTION_TO_ROOK,
	PROMOTION_TO_BISHOP,
	PROMOTION_TO_KNIGHT,
	PROMOTION_TO_QUEEN_AND_CAPTURE,
	PROMOTION_TO_ROOK_AND_CAPTURE,
	PROMOTION_TO_BISHOP_AND_CAPTURE,
	PROMOTION_TO_KNIGHT_AND_CAPTURE
};

template<MoveType moveType>
consteval bool IsMoveTypePromotion()
{
	return moveType == MoveType::PROMOTION_TO_QUEEN ||
		moveType == MoveType::PROMOTION_TO_ROOK ||
		moveType == MoveType::PROMOTION_TO_BISHOP ||
		moveType == MoveType::PROMOTION_TO_KNIGHT ||
		moveType == MoveType::PROMOTION_TO_QUEEN_AND_CAPTURE ||
		moveType == MoveType::PROMOTION_TO_ROOK_AND_CAPTURE ||
		moveType == MoveType::PROMOTION_TO_BISHOP_AND_CAPTURE ||
		moveType == MoveType::PROMOTION_TO_KNIGHT_AND_CAPTURE;
}

template<MoveType moveType>
consteval bool IsMoveTypeCapture()
{
	return moveType == MoveType::CAPTURE ||
		moveType == MoveType::PROMOTION_TO_QUEEN_AND_CAPTURE ||
		moveType == MoveType::PROMOTION_TO_ROOK_AND_CAPTURE ||
		moveType == MoveType::PROMOTION_TO_BISHOP_AND_CAPTURE ||
		moveType == MoveType::PROMOTION_TO_KNIGHT_AND_CAPTURE;

}

template<MoveType moveType>
consteval PieceType GetPromotionPieceFromMoveType()
{
	static_assert(IsMoveTypePromotion<moveType>(), "Move type is not a promotion move type");
	switch (moveType)
	{
	case MoveType::PROMOTION_TO_QUEEN:
	case MoveType::PROMOTION_TO_QUEEN_AND_CAPTURE:
		return QUEEN;
	case MoveType::PROMOTION_TO_ROOK:
	case MoveType::PROMOTION_TO_ROOK_AND_CAPTURE:
		return ROOK;
	case MoveType::PROMOTION_TO_BISHOP:
	case MoveType::PROMOTION_TO_BISHOP_AND_CAPTURE:
		return BISHOP;
	case MoveType::PROMOTION_TO_KNIGHT:
	case MoveType::PROMOTION_TO_KNIGHT_AND_CAPTURE:
		return KNIGHT;
	}
	return PIECE_TYPE_NONE;
}
