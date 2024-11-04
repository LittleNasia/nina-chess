#pragma once
#include "utils.h"

#include "bit_manip.h"
#include "bitmasks.h"
#include "castling.h"

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

	inline static constexpr uint32_t CASTLING_OFFSET = PROMOTION_PIECE_OFFSET + 4;
	inline static constexpr uint32_t CASTLING_BITMASK = 0b1 << CASTLING_OFFSET;

	inline static constexpr uint32_t EN_PASSANT_OFFSET = CASTLING_OFFSET + 1;
	inline static constexpr uint32_t EN_PASSANT_BITMASK = 0b1 << EN_PASSANT_OFFSET;

	forceinline constexpr std::string GetUciPromotionPiece() const
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

	forceinline std::string ToUciMove() const { return std::string(NAME_OF_SQUARE[BitIndex(FromBitmask())]) + NAME_OF_SQUARE[BitIndex(ToBitmask())] + GetUciPromotionPiece(); }

	forceinline constexpr Bitboard  FromBitmask()			const { return 1ULL << FromIndex(); }
	forceinline constexpr Bitboard  ToBitmask()				const { return 1ULL << ToIndex(); }
	forceinline constexpr uint32_t  FromIndex()				const { return (m_encoded_move & INDEX_FROM_BITMASK); }
	forceinline constexpr uint32_t  ToIndex()				const { return (m_encoded_move & INDEX_TO_BITMASK) >> INDEX_TO_OFFSET; }
	forceinline constexpr PieceType MovingPieceType()		const { return static_cast<PieceType>((m_encoded_move & PIECE_BITMASK) >> PIECE_OFFSET); }
	forceinline constexpr PieceType PromotionPieceType()	const { return static_cast<PieceType>((m_encoded_move & PROMOTION_PIECE_BITMASK) >> PROMOTION_PIECE_OFFSET); }
	forceinline constexpr bool      IsCastling()			const { return (m_encoded_move & CASTLING_BITMASK) >> CASTLING_OFFSET; }
	forceinline constexpr bool      IsEnPassant()					const { return (m_encoded_move & EN_PASSANT_BITMASK) >> EN_PASSANT_OFFSET; }

	forceinline constexpr bool operator==(const Move& other) const { return m_encoded_move == other.m_encoded_move; }

	forceinline constexpr Move() : Move(0, 0, PIECE_TYPE_NONE, PIECE_TYPE_NONE)
	{}

	forceinline constexpr Move(const uint32_t from, const uint32_t to, const PieceType piece) :
		m_encoded_move{
		(PIECE_TYPE_NONE << PROMOTION_PIECE_OFFSET) |
		(from << INDEX_FROM_OFFSET) |
		(to << INDEX_TO_OFFSET) |
		(piece << PIECE_OFFSET)
		}
	{}

	forceinline constexpr Move(const uint32_t from, const uint32_t to, const PieceType piece, const PieceType promotion_piece) :
		m_encoded_move{
		(from << INDEX_FROM_OFFSET) |
		(to << INDEX_TO_OFFSET) |
		(piece << PIECE_OFFSET) |
		(promotion_piece << PROMOTION_PIECE_OFFSET)
		}
	{}

	forceinline constexpr Move(const uint32_t from, const uint32_t to, const PieceType piece, const PieceType promotion_piece, const bool castling) :
		m_encoded_move{
		(from << INDEX_FROM_OFFSET) |
		(to << INDEX_TO_OFFSET) |
		(piece << PIECE_OFFSET) |
		(promotion_piece << PROMOTION_PIECE_OFFSET) |
		(castling << CASTLING_OFFSET)
		}
	{}

	forceinline constexpr Move(const uint32_t from, const uint32_t to, const PieceType piece, const PieceType promotion_piece, const bool castling, const bool EP) :
		m_encoded_move{
		(from << INDEX_FROM_OFFSET) |
		(to << INDEX_TO_OFFSET) |
		(piece << PIECE_OFFSET) |
		(promotion_piece << PROMOTION_PIECE_OFFSET) |
		(castling << CASTLING_OFFSET) |
		(EP << EN_PASSANT_OFFSET)
		}
	{}

	forceinline constexpr bool IsKingsideCastling() const
	{
		constexpr Bitboard kingStartposBitmask = KingStartposBitmask<WHITE>() | KingStartposBitmask<BLACK>();
		constexpr Bitboard kingsideRookBitmask = KingsideCastlingRookBitmask<WHITE>() | KingsideCastlingRookBitmask<BLACK>();

		return (FromBitmask() & kingStartposBitmask) && (ToBitmask() & kingsideRookBitmask);
	}

	forceinline constexpr bool IsQueensideCastling() const
	{
		constexpr Bitboard kingStartposBitmask = KingStartposBitmask<WHITE>() | KingStartposBitmask<BLACK>();
		constexpr Bitboard queensideRookBitmask = QueensideCastlingRookBitmask<WHITE>() | QueensideCastlingRookBitmask<BLACK>();

		return (FromBitmask() & kingStartposBitmask) && (ToBitmask() & queensideRookBitmask);
	}

private:
	uint32_t m_encoded_move;
};