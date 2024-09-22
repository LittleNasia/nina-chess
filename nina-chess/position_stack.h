#pragma once
#include "utils.h"

#include <atomic>
#include <chrono>

#include "move_gen.h"
#include "position.h"
#include "serializable.h"

struct PositionStack
{
public:
	forceinline PositionStack() :
		depth{ 0 },
		move_list_stack{},
		position_stack{}
	{}

	forceinline void Reset();
	forceinline void Reset(const Position& pos);

	forceinline constexpr int64_t GetDepth() const { return depth; }

	forceinline constexpr void IncrementDepth() { ++depth; }
	forceinline constexpr void DecrementDepth() { --depth; }

	forceinline constexpr bool IsThreefoldRepetition() const;

	forceinline constexpr		void	  SetCurrentPosition(const Position& position);
	forceinline constexpr		Position& GetPositionAt(const int64_t depth);
	forceinline constexpr const Position& GetCurrentPosition() const;
	forceinline constexpr		Position& GetNextPosition();

	template<Color side_to_move>
	forceinline constexpr MoveList& GetMoveListSkippingHashCheck();
	forceinline constexpr MoveList& GetMoveListAt(const int64_t depth);
	template<Color side_to_move>
	forceinline constexpr MoveList& GetMoveList();
	forceinline constexpr MoveList& GetMoveList();

private:
	forceinline constexpr uint64_t GetHashAtPly(const int64_t ply) const { return position_stack[ply].hash; }

	MoveList move_list_stack[max_ply];
	Position position_stack[max_ply];

	int64_t depth;
};

#include "position_stack.inl"