#pragma once
#include "utils.h"

#include "evaluator.h"
#include "move_list.h"
#include "position.h"
#include "transposition_table.h"

struct SearchStack
{
public:
	forceinline SearchStack(const int depth, TranspositionTable& tt, Evaluator& evaluator):
		depth(0),
		remaining_depth(depth),
		nodes(0),
		tt(&tt),
		evaluator(&evaluator),
		move_list_stack(std::make_unique<MoveList[]>(depth + 1)),
		position_stack(std::make_unique<Position[]>(depth + 1)),
		hash_stack(std::make_unique<uint64_t[]>(depth + 1))
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

	forceinline constexpr Evaluator& GetEvaluator() const { return *evaluator; }

	forceinline constexpr void SetCurrentPosition(const Position& position) { position_stack[depth] = position; }

	forceinline constexpr const Position& GetCurrentPosition() const { return position_stack[depth]; }

	forceinline constexpr Position& GetNextPosition() { return position_stack[depth + 1]; }

	forceinline constexpr MoveList& GetMoveList() { return move_list_stack[depth]; }

	forceinline constexpr void SetCurrentPositionHash() { hash_stack[depth] = GetCurrentPosition().hash; }

	forceinline constexpr void SetNextPositionHash() { hash_stack[depth + 1] = GetNextPosition().hash; }
	
	forceinline constexpr uint64_t GetHashAtPly(const int ply) const { return hash_stack[ply]; }

private:
	std::unique_ptr<MoveList[]> move_list_stack;
	std::unique_ptr<Position[]> position_stack;
	std::unique_ptr<uint64_t[]> hash_stack;
};