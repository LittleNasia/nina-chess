#pragma once
#include "utils.h"

#include <string_view>

#include "attacks.h"
#include "side.h"

struct Position
{
	forceinline constexpr Position();

	forceinline constexpr Position(
		const Bitboard w_pawns, const Bitboard w_knights, const Bitboard w_bishops,
		const Bitboard w_rooks, const Bitboard w_queens,  const Bitboard w_king,
		const Bitboard b_pawns, const Bitboard b_knights, const Bitboard b_bishops,
		const Bitboard b_rooks, const Bitboard b_queens,  const Bitboard b_king,
		const Bitboard EP_square, const CastlingType castling, const Color side_to_move,
		const int ply);

	forceinline constexpr Position(
		const Side& white_pieces,
		const Side& black_pieces,
		const Bitboard EP_square, const CastlingType castling, const Color side_to_move,
		const int ply);


	template<Color color>
	constexpr const Side& get_side() const;

	forceinline constexpr CastlingType get_curr_castling() const;


	const Side white_pieces;
	const Side black_pieces;

	const Bitboard occupied;
	const Bitboard EP_square;
	const CastlingType castling;
	const Color side_to_move;
	const int ply;
};

template<Color side_to_move, bool castling, bool EP>
forceinline Position make_move(const Position& pos, const Move& m);

forceinline Position make_move(const Position& pos, const Move& m);

void print_board(const Position& curr_pos);

Position parse_fen(const std::string_view fen);


#include "position.inl"