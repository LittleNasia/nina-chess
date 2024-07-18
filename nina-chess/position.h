#pragma once
#include "utils.h"

#include <string_view>

#include "attacks.h"
#include "side.h"

struct Position
{
	forceinline Position();

	forceinline Position(
		const Bitboard w_pawns, const Bitboard w_knights, const Bitboard w_bishops,
		const Bitboard w_rooks, const Bitboard w_queens,  const Bitboard w_king,
		const Bitboard b_pawns, const Bitboard b_knights, const Bitboard b_bishops,
		const Bitboard b_rooks, const Bitboard b_queens,  const Bitboard b_king,
		const Bitboard EP_square, const CastlingType castling, const Color side_to_move,
		const int ply);

	forceinline Position(
		const Bitboard w_pawns, const Bitboard w_knights, const Bitboard w_bishops,
		const Bitboard w_rooks, const Bitboard w_queens, const Bitboard w_king,
		const Bitboard b_pawns, const Bitboard b_knights, const Bitboard b_bishops,
		const Bitboard b_rooks, const Bitboard b_queens, const Bitboard b_king,
		const Bitboard EP_square, const CastlingType castling, const Color side_to_move,
		const int ply,
		const uint64_t hash);

	forceinline Position(
		const Side& white_pieces,
		const Side& black_pieces,
		const Bitboard EP_square, const CastlingType castling, const Color side_to_move,
		const int ply);

	forceinline Position(
		const Side& white_pieces,
		const Side& black_pieces,
		const Bitboard EP_square, const CastlingType castling, const Color side_to_move,
		const int ply,
		const uint64_t hash);


	template<Color color>
	constexpr const Side& get_side() const;

	forceinline constexpr CastlingType get_curr_castling() const;

	forceinline uint64_t calculate_hash() const;

	const Side white_pieces;
	const Side black_pieces;

	const Bitboard occupied;
	const Bitboard EP_square;
	const CastlingType castling;
	const Color side_to_move;
	const int ply;
	const uint64_t hash;
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

	Position ParseFen(const std::string_view fen);
}

#include "position.inl"