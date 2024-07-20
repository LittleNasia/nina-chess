#pragma once
#include "utils.h"

#include <string_view>

#include "attacks.h"
#include "move.h"
#include "side.h"

struct Position
{
	forceinline Position(uint64_t* hash_history);

	forceinline Position(
		const Side& white_pieces,
		const Side& black_pieces,
		const Bitboard EP_square, const CastlingType castling, const Color side_to_move,
		const int ply, const uint32_t fifty_move_rule, uint64_t* hash_history);

	forceinline Position(
		const Side& white_pieces,
		const Side& black_pieces,
		const Bitboard EP_square, const CastlingType castling, const Color side_to_move,
		const int ply, const uint32_t fifty_move_rule,
		const uint64_t hash, uint64_t* hash_history);

	template<Color color>
	constexpr const Side& GetSide() const;

	forceinline constexpr CastlingType GetCurrentCastling() const;

	forceinline uint64_t CalculateHash() const;

	forceinline constexpr bool IsDrawn() const;
	forceinline constexpr bool IsFiftyMoveRule() const;
	forceinline constexpr bool IsThreefoldRepetition() const;
	forceinline constexpr bool IsInsufficientMaterial() const;

	Side white_pieces;
	Side black_pieces;

	Bitboard occupied;
	Bitboard EP_square;
	CastlingType castling;
	Color side_to_move;
	int ply;
	uint64_t hash;
	uint32_t fifty_move_rule;

	uint64_t* hash_history;
};

namespace position
{
	template<Color side_to_move, bool castling, bool EP>
	forceinline Position MakeMove(const Position& pos, const Move& m);

	template<Color side_to_move>
	forceinline Position MakeMove(const Position& pos, const Move& m);

	forceinline Position MakeMove(const Position& pos, const Move& m);

	template<Color side_to_move>
	forceinline constexpr uint64_t update_hash(uint64_t hash, const PieceType moving_piece, Bitboard move);

	void PrintBoard(const Position& curr_pos);

	Position ParseFen(const std::string_view fen, uint64_t* hash_history);
}

#include "position.inl"