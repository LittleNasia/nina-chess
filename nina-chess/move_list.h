#pragma once
#include "utils.h"

#include "architecture.h"
#include "intrinsics.h"
#include "move.h"

struct MoveListMiscellaneous
{
	Bitboard PieceMoves[PIECE_TYPE_NONE] = { 0,0,0,0,0,0 };
	Bitboard Pinmask = 0;
	Bitboard Checkmask = 0;
	Bitboard Pinners = 0;
	Bitboard Checkers = 0;
	Bitboard AttackedSquares = 0;

	forceinline void Reset()
	{
		std::memset(PieceMoves, 0, sizeof(PieceMoves));
		Pinmask = 0;
		Checkmask = 0;
		Pinners = 0;
		Checkers = 0;
		AttackedSquares = 0;
	}
};

struct MoveList
{
	forceinline constexpr MoveList() = default;

	forceinline void PushMove(const Move&& move)
	{
		m_Moves[m_NumMoves++] = move;
	}

	forceinline void Reset()
	{
		m_NumMoves = 0;
		m_HashOfPosition = 0;
		MoveListMisc.Reset();
	}

	forceinline constexpr uint32_t GetNumMoves() const { return m_NumMoves; }
	forceinline constexpr const Move* GetMoves() const { return m_Moves; }

	forceinline constexpr void SetHashOfPosition(const uint64_t hash) { m_HashOfPosition = hash; }
	forceinline constexpr uint64_t GetHashOfPosition() const { return m_HashOfPosition; }

	forceinline constexpr const Move& operator[](const uint32_t index) const { return m_Moves[index]; }

	MoveListMiscellaneous MoveListMisc;

private:
	alignas(CACHE_LINE_SIZE) Move m_Moves[200];
	uint32_t m_NumMoves = 0;
	uint64_t m_HashOfPosition = 0;
};