#pragma once
#include "utils.h"

struct Move
{
public:
	inline static constexpr uint32_t index_from_offset = 0;
	inline static constexpr uint32_t index_from_mask = 0b111111 << index_from_offset;

	inline static constexpr uint32_t index_to_offset = index_from_offset + 6;
	inline static constexpr uint32_t index_to_mask = 0b111111 << index_to_offset;

	inline static constexpr uint32_t piece_offset = index_to_offset + 6;
	inline static constexpr uint32_t piece_mask = 0b1111 << piece_offset;

	inline static constexpr uint32_t promotion_piece_offset = piece_offset + 4;
	inline static constexpr uint32_t promotion_piece_mask = 0b1111 << promotion_piece_offset;

	inline static constexpr uint32_t castling_offset = promotion_piece_offset + 4;
	inline static constexpr uint32_t castling_mask = 0b1 << castling_offset;

	inline static constexpr uint32_t EP_offset = castling_offset + 1;
	inline static constexpr uint32_t EP_mask = 0b1 << EP_offset;

	forceinline constexpr std::string GetUciPromotionPiece() const
	{
		switch (promotion_piece())
		{
		case PIECE_TYPE_NONE: return "";
		case QUEEN: return "q";
		case ROOK: return "r";
		case BISHOP: return "b";
		case KNIGHT: return "n";
		default: return "";
		}
	}

	forceinline std::string ToUciMove() const { return std::string(square_names[bit_index(from())]) + square_names[bit_index(to())] + GetUciPromotionPiece(); }


	forceinline constexpr Bitboard  from()			  const	{ return 1ULL << from_index(); }
	forceinline constexpr Bitboard  to()			  const	{ return 1ULL << to_index(); }
	forceinline constexpr uint32_t  from_index()	  const { return (encodedMove & index_from_mask); }
	forceinline constexpr uint32_t  to_index()		  const { return (encodedMove & index_to_mask) >> index_to_offset; }
	forceinline constexpr PieceType piece()			  const	{ return static_cast<PieceType>((encodedMove & piece_mask) >> piece_offset); }
	forceinline constexpr PieceType promotion_piece() const { return static_cast<PieceType>((encodedMove & promotion_piece_mask) >> promotion_piece_offset); }
	forceinline constexpr bool      is_castling()	  const	{ return (encodedMove & castling_mask) >> castling_offset; }
	forceinline constexpr bool      is_EP()			  const	{ return (encodedMove & EP_mask) >> EP_offset; }

	forceinline constexpr bool operator==(const Move& other) const { return encodedMove == other.encodedMove; }

	forceinline constexpr Move() : Move(0, 0, PIECE_TYPE_NONE, PIECE_TYPE_NONE)
	{}

	forceinline constexpr Move(const uint32_t from, const uint32_t to, const PieceType piece) :
		encodedMove{
		(PIECE_TYPE_NONE << promotion_piece_offset) |
		(from << index_from_offset) |
		(to << index_to_offset) |
		(piece << piece_offset)
		}
	{}

	forceinline constexpr Move(const uint32_t from, const uint32_t to, const PieceType piece, const PieceType promotion_piece) :
		encodedMove{
		(from << index_from_offset) |
		(to << index_to_offset) |
		(piece << piece_offset) |
		(promotion_piece << promotion_piece_offset)
		}
	{}

	forceinline constexpr Move(const uint32_t from, const uint32_t to, const PieceType piece, const PieceType promotion_piece, const bool castling) :
		encodedMove{
		(from << index_from_offset) |
		(to << index_to_offset) |
		(piece << piece_offset) |
		(promotion_piece << promotion_piece_offset) |
		(castling << castling_offset)
		}
	{}

	forceinline constexpr Move(const uint32_t from, const uint32_t to, const PieceType piece, const PieceType promotion_piece, const bool castling, const bool EP) :
		encodedMove{
		(from << index_from_offset) |
		(to << index_to_offset) |
		(piece << piece_offset) |
		(promotion_piece << promotion_piece_offset) |
		(castling << castling_offset) |
		(EP << EP_offset)
		}
	{}

	forceinline constexpr bool is_kingside_castling() const
	{
		constexpr Bitboard king_startpos_bitmask = king_startpos<WHITE>() | king_startpos<BLACK>();
		constexpr Bitboard kingside_rooks = kingside_castling_rook<WHITE>() | kingside_castling_rook<BLACK>();

		return (from() & king_startpos_bitmask) && (to() & kingside_rooks);
	}

	forceinline constexpr bool is_queenside_castling() const
	{
		constexpr Bitboard king_startpos_bitmask = king_startpos<WHITE>() | king_startpos<BLACK>();
		constexpr Bitboard queenside_rooks = queenside_castling_rook<WHITE>() | queenside_castling_rook<BLACK>();

		return (from() & king_startpos_bitmask) && (to() & queenside_rooks);
	}

private:
	uint32_t encodedMove;
};