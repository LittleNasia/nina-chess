#pragma once
#include "utils.h"

#include <atomic>
#include <chrono>

#include "move_gen.h"
#include "position.h"

struct SearchStack
{
public:
	forceinline SearchStack() :
		depth{ 0 },
		remaining_depth{ 0 },
		nodes{ 0 },
		search_aborted{},
		start_time{},
		move_list_stack(std::make_unique<MoveList[]>(max_ply)),
		position_stack(std::make_unique<Position[]>(max_ply))
	{}

	int depth;
	int remaining_depth;
	size_t nodes;

	forceinline void Reset();
	forceinline void Reset(const Position& pos);

	forceinline constexpr void IncrementDepth()
	{
		++depth;
		--remaining_depth;
	}
	forceinline constexpr void DecrementDepth()
	{
		--depth;
		++remaining_depth;
	}

	forceinline constexpr bool IsThreefoldRepetition() const
	{
		const Position& current_position = GetCurrentPosition();
		const int ply_to_search_to = 0;
		int repetition_count = 0;

		for (int ply = depth - 2; ply >= ply_to_search_to; ply-=2)
		{
			if (current_position.hash == GetHashAtPly(ply))
			{
				return true;
			}
		}
		return false;
	}

	forceinline constexpr		void	  SetCurrentPosition(const Position& position);
	forceinline constexpr		Position& GetPositionAt(const int depth);
	forceinline constexpr const Position& GetCurrentPosition() const;
	forceinline constexpr		Position& GetNextPosition();

	template<Color side_to_move>
	forceinline constexpr MoveList& GetMoveListSkippingHashCheck();
	forceinline constexpr MoveList& GetMoveListAt(const int depth);
	template<Color side_to_move>
	forceinline constexpr MoveList& GetMoveList();
	forceinline constexpr MoveList& GetMoveList();
	
	forceinline void StartSearch();
	forceinline int64_t GetSearchDurationInMs() const;

	forceinline void AbortSearch() { search_aborted.test_and_set(); }

	forceinline bool IsSearchAborted() const { return search_aborted.test(); }

private:
	forceinline void ResetSearchAbortedFlag() { search_aborted.clear(); }
	forceinline constexpr uint64_t GetHashAtPly(const int ply) const { return position_stack[ply].hash; }

	std::chrono::time_point<std::chrono::high_resolution_clock> start_time;
	std::atomic_flag search_aborted;

	std::unique_ptr<MoveList[]> move_list_stack;
	std::unique_ptr<Position[]> position_stack;
};

#include "search_stack.inl"