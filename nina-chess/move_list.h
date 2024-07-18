#pragma once
#include "utils.h"

struct MoveList
{
	Move moves[100];
	forceinline constexpr MoveList() = default;
	forceinline void push_move(const Move&& move)
	{
		moves[num_moves++] = move;
	}
	forceinline void reset()
	{
		num_moves = 0;
	}
	forceinline constexpr uint32_t get_num_moves() const { return num_moves; }

	Bitboard piece_moves[PIECE_TYPE_NONE] = { 0,0,0,0,0,0 };
	Bitboard pinmask = 0;
	Bitboard checkmask = 0;
	Bitboard pinners = 0;
	Bitboard checkers = 0;
	Bitboard attacked_squares = 0;
	uint32_t num_moves = 0;
};