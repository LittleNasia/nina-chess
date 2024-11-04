#pragma once
#include "utils.h"

#include <atomic>
#include <chrono>

#include "move_gen.h"
#include "position.h"
#include "search_core.h"
#include "serializable.h"

struct PositionStack
{
public:
	forceinline PositionStack() :
		m_MoveListStack{},
		m_PositionStack{},
		m_Depth{ 0 }
	{}

	forceinline void Reset();
	forceinline void Reset(const Position& position);

	forceinline constexpr int64_t GetDepth() const { return m_Depth; }

	forceinline constexpr void IncrementDepth() { ++m_Depth; }
	forceinline constexpr void DecrementDepth() { --m_Depth; }

	forceinline constexpr bool IsThreefoldRepetition() const;

	forceinline constexpr		void	  SetCurrentPosition(const Position& position);
	forceinline constexpr		Position& GetPositionAt(const int64_t depth);
	forceinline constexpr const Position& GetCurrentPosition() const;
	forceinline constexpr		Position& GetNextPosition();

	template<Color sideToMove>
	forceinline constexpr MoveList& GetMoveListSkippingHashCheck();
	forceinline constexpr MoveList& GetMoveListAt(const int64_t depth);
	template<Color sideToMove>
	forceinline constexpr MoveList& GetMoveList();
	forceinline constexpr MoveList& GetMoveList();

private:
	forceinline constexpr uint64_t GetHashAtPly(const int64_t ply) const { return m_PositionStack[ply].Hash; }

	MoveList m_MoveListStack[MAX_PLY];
	Position m_PositionStack[MAX_PLY];

	int64_t m_Depth;
};

#include "position_stack.inl"