#pragma once
#include "utils.h"

#include "move.h"

struct alignas(cache_line_size) MoveListMisc
{
	Bitboard piece_moves[PIECE_TYPE_NONE] = { 0,0,0,0,0,0 };
	Bitboard pinmask = 0;
	Bitboard checkmask = 0;
	Bitboard pinners = 0;
	Bitboard checkers = 0;
	Bitboard attacked_squares = 0;

	void Reset()
	{
		std::memset(piece_moves, 0, sizeof(piece_moves));
		pinmask = 0;
		checkmask = 0;
		pinners = 0;
		checkers = 0;
		attacked_squares = 0;
	}
};

struct alignas(cache_line_size) MoveList
{
	forceinline constexpr MoveList() = default;

	forceinline void PushMove(const Move&& move)
	{
		moves[num_moves++] = move;
	}

	forceinline void Reset()
	{
		num_moves = 0;
		hash_of_position = 0;
		move_list_misc.Reset();
	}

	forceinline constexpr uint32_t GetNumMoves() const { return num_moves; }
	forceinline constexpr const Move* GetMoves() const { return moves; }

	forceinline constexpr void SetHashOfPosition(const uint64_t hash) { hash_of_position = hash; }
	forceinline constexpr uint64_t GetHashOfPosition() const { return hash_of_position; }

	forceinline constexpr const Move& operator[](const uint32_t index) const { return moves[index]; }

	MoveListMisc move_list_misc;

private:
	alignas(cache_line_size) Move moves[200];
	uint32_t num_moves = 0;
	uint64_t hash_of_position = 0;
};