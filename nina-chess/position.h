#pragma once
#include "utils.h"

#include <string_view>

#include "attacks.h"
#include "side.h"

struct Position
{
	forceinline constexpr Position();

	forceinline constexpr Position(
		Bitboard w_pawns, Bitboard w_knights, Bitboard w_bishops,
		Bitboard w_rooks, Bitboard w_queens, Bitboard w_king,
		Bitboard b_pawns, Bitboard b_knights, Bitboard b_bishops,
		Bitboard b_rooks, Bitboard b_queens, Bitboard b_king,
		Bitboard EP_square, CastlingType castling, Color side_to_move,
		int ply);

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