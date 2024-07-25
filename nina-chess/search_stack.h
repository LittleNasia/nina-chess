#pragma once
#include "utils.h"

#include <atomic>
#include <chrono>

#include "evaluator.h"
#include "move_gen.h"
#include "position.h"
#include "transposition_table.h"

struct SearchStack
{
public:
	forceinline SearchStack(const std::vector<Position>& position_stack, const int depth, TranspositionTable& tt, Evaluator& evaluator):
		depth((int)position_stack.size() - 1),
		remaining_depth(depth),
		nodes(0),
		tt(&tt),
		evaluator(&evaluator),
		move_list_stack(std::make_unique<MoveList[]>(this->depth + remaining_depth + 1)),
		position_stack(std::make_unique<Position[]>(this->depth + remaining_depth + 1)),
		hash_stack(std::make_unique<uint64_t[]>(this->depth + remaining_depth + 1)),
		start_time{},
		search_aborted{}
	{

		int curr_depth = 0;
		for (const auto& position : position_stack)
		{
			this->position_stack[curr_depth] = position;
			this->hash_stack[curr_depth] = this->position_stack[curr_depth].hash;
			generate_moves(this->position_stack[curr_depth], this->move_list_stack[curr_depth]);
			curr_depth++;
		}
	}

	forceinline SearchStack(const int depth, TranspositionTable& tt, Evaluator& evaluator) :
		depth(0),
		remaining_depth(depth),
		nodes(0),
		tt(&tt),
		evaluator(&evaluator),
		move_list_stack(std::make_unique<MoveList[]>(this->depth + remaining_depth + 1)),
		position_stack(std::make_unique<Position[]>(this->depth + remaining_depth + 1)),
		hash_stack(std::make_unique<uint64_t[]>(this->depth + remaining_depth + 1)),
		start_time{},
		search_aborted{}
	{
	}

	int depth;
	int remaining_depth;
	size_t nodes;

	TranspositionTable* tt;
	Evaluator* evaluator;

	forceinline constexpr TranspositionTable& GetTranspositionTable() const { return *tt; }

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

	forceinline constexpr Evaluator& GetEvaluator() const { return *evaluator; }

	forceinline constexpr void SetCurrentPosition(const Position& position) { position_stack[depth] = position; }

	forceinline constexpr const Position& GetCurrentPosition() const { return position_stack[depth]; }

	forceinline constexpr Position& GetNextPosition() { return position_stack[depth + 1]; }

	forceinline constexpr MoveList& GetMoveList() { return move_list_stack[depth]; }

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