#pragma once
#include "utils.h"

#include <atomic>
#include <chrono>

#include "move_gen.h"
#include "position.h"

struct alignas(cache_line_size) SearchStack
{
public:
	forceinline SearchStack() :
		depth{ 0 },
		remaining_depth{ 0 },
		nodes{ 0 },
		search_aborted{},
		start_time{},
		move_list_stack(std::make_unique<MoveList[]>(max_ply)),
		position_stack(std::make_unique<Position[]>(max_ply)),
		hash_stack(std::make_unique<uint64_t[]>(max_ply))
	{}

	forceinline void Reset(const std::vector<Position>& position_stack_vec)
	{
		depth = (int)position_stack_vec.size() - 1;
		remaining_depth = 0;
		nodes = 0;

		int curr_depth = 0;
		for (const auto& position : position_stack_vec)
		{
			position_stack[curr_depth] = position;
			hash_stack[curr_depth] = position_stack[curr_depth].hash;
			generate_moves(position_stack[curr_depth], move_list_stack[curr_depth]);
			curr_depth++;
		}
	}

	int depth;
	int remaining_depth;
	size_t nodes;

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
		const int ply_to_search_to = depth < int(current_position.fifty_move_rule) ? depth : int(current_position.fifty_move_rule);

		for (int ply = depth - 2; ply >= ply_to_search_to; ply -= 2)
		{
			if (current_position.hash == GetHashAtPly(ply))
			{
				return true;
			}
		}
		return false;
	}

	forceinline void StartSearch() { start_time = std::chrono::high_resolution_clock::now(); }

	forceinline int GetSearchDurationInMs()
	{
		const auto end_time = std::chrono::high_resolution_clock::now();
		return (int)std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time).count();
	}

	forceinline void AbortSearch() { search_aborted.test_and_set(); }

	forceinline bool IsSearchAborted() const { return search_aborted.test(); }

	forceinline void ResetSearchAbortedFlag() { search_aborted.clear(); }

	forceinline constexpr void SetCurrentPosition(const Position& position) { position_stack[depth] = position; }

	forceinline constexpr const Position& GetCurrentPosition() const { return position_stack[depth]; }

	forceinline constexpr Position& GetNextPosition() { return position_stack[depth + 1]; }

	forceinline constexpr Position& GetPositionAt(const int depth) { return position_stack[depth]; }

	forceinline constexpr MoveList& GetMoveListAt(const int depth) { 
		if (move_list_stack[depth].GetHashOfPosition() != position_stack[depth].hash)
			generate_moves(position_stack[depth], move_list_stack[depth]);
		return move_list_stack[depth];
	}

	forceinline constexpr MoveList& GetMoveList()
	{
		const Position& curr_pos = GetCurrentPosition();
		if (curr_pos.side_to_move == WHITE)
			return GetMoveList<WHITE>();
		else
			return GetMoveList<BLACK>();
	}

	template<Color side_to_move, bool skip_check = false>
	forceinline constexpr MoveList& GetMoveList()
	{
		const Position& curr_pos = GetCurrentPosition();
		MoveList& curr_move_list = move_list_stack[depth];
		if constexpr (skip_check)
		{
			generate_moves<side_to_move>(curr_pos, curr_move_list);
		}
		else
		{
			if (curr_move_list.GetHashOfPosition() != curr_pos.hash)
			{
				generate_moves<side_to_move>(curr_pos, curr_move_list);
			}
		}
			
		return curr_move_list;
	}

	forceinline constexpr void SetCurrentPositionHash() { hash_stack[depth] = GetCurrentPosition().hash; }

	forceinline constexpr void SetNextPositionHash() { hash_stack[depth + 1] = GetNextPosition().hash; }
	
	forceinline constexpr uint64_t GetHashAtPly(const int ply) const { return hash_stack[ply]; }

private:
	std::chrono::time_point<std::chrono::high_resolution_clock> start_time;
	std::atomic_flag search_aborted;
	std::unique_ptr<MoveList[]> move_list_stack;
	std::unique_ptr<Position[]> position_stack;
	std::unique_ptr<uint64_t[]> hash_stack;
};