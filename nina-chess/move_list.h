#pragma once
#include "utils.h"

#include "move.h"

struct alignas(64) MoveList
{
	alignas(64) Move moves[200];
	forceinline constexpr MoveList() = default;
	forceinline void push_move(const Move&& move)
	{
		moves[num_moves++] = move;
	}
	forceinline void reset()
	{
		num_moves = 0;
		std::memset(piece_moves, 0, sizeof(piece_moves));
		pinmask = 0;
		checkmask = 0;
		pinners = 0;
		checkers = 0;
		attacked_squares = 0;
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