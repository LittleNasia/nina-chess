#pragma once
#include "utils.h"

#include <string_view>

#include "attacks.h"
#include "castling.h"
#include "move.h"
#include "side.h"

struct Position
{
	forceinline Position();

	forceinline Position(
		const Side& white_pieces,
		const Side& black_pieces,
		const Bitboard EP_square,
		const Castling castling,
		const Color side_to_move,
		const uint32_t fifty_move_rule);

	forceinline Position(
		const Side& white_pieces,
		const Side& black_pieces,
		const Bitboard EP_square,
		const Castling castling,
		const Color side_to_move,
		const uint32_t fifty_move_rule,
		const uint64_t hash);

	template<Color color>
	constexpr const Side& GetSide() const;

	template<Color color>
	constexpr Side& GetSide();

	forceinline constexpr Castling GetCurrentCastling() const;

	forceinline uint64_t CalculateHash() const;

	forceinline constexpr bool IsDrawn() const;
	forceinline constexpr bool IsFiftyMoveRule() const;
	forceinline constexpr bool IsInsufficientMaterial() const;

	forceinline constexpr void UpdateOccupiedBitboard();

	Side white_pieces;
	Side black_pieces;

	Bitboard occupied;
	Bitboard EP_square;
	Castling castling;
	Color side_to_move;
	uint64_t hash;
	uint32_t fifty_move_rule;
};

namespace position
{
	template<Color side_to_move, bool castling, bool EP>
	forceinline Position& MakeMove(const Position& pos, Position& new_pos, const Move& m);

	template<Color side_to_move>
	forceinline Position& MakeMove(const Position& pos, Position& new_pos, const Move& m);

	forceinline Position& MakeMove(const Position& pos, Position& new_pos, const Move& m);

	void PrintBoard(const Position& curr_pos);

	Position ParseFen(const std::string_view fen);
}

#include "position.inl"