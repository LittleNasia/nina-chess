#pragma once
#include "utils.h"

#include "evaluator.h"
#include "move_gen.h"
#include "position.h"
#include "search_stack.h"

class IncrementalUpdater
{
public:
	IncrementalUpdater(Evaluator& evaluator, SearchStack& search_stack) :
		evaluator(&evaluator),
		search_stack(&search_stack)
	{}

	template<Color side_to_move>
	constexpr void MakeMoveUpdate(const Move& move)
	{
		const Position& prev_pos = search_stack->GetCurrentPosition();
		Position& new_pos = search_stack->GetNextPosition();
		position::MakeMove<side_to_move>(prev_pos, new_pos, move);

		search_stack->SetCurrentPositionHash();
		search_stack->IncrementDepth();
	}

	constexpr void UndoUpdate()
	{
		search_stack->DecrementDepth();
	}

	// Move generation update is tricky, because it might not happen on every node (as we might return prematurely)
	// so we can't undo it on every single node, as we don't know whether we've hit the update or not
	// this object just ensures that if move generation update was done, it will be undone on the next return
	struct MoveGenerationUpdateGuard
	{
		forceinline constexpr MoveGenerationUpdateGuard(Evaluator* evaluator) : 
			evaluator(evaluator)
		{}

		forceinline constexpr ~MoveGenerationUpdateGuard()
		{
			evaluator->UndoUpdate();
		}

	private:
		Evaluator* evaluator;
	};

	template<Color side_to_move>
	constexpr MoveGenerationUpdateGuard MoveGenerationUpdate()
	{
		const auto& curr_pos = search_stack->GetCurrentPosition();
		const auto& move_list = search_stack->GetMoveList<side_to_move>();

		evaluator->IncrementalUpdate<side_to_move>(curr_pos, move_list);

		return MoveGenerationUpdateGuard(evaluator);
	}

private:
	Evaluator* evaluator;
	SearchStack* search_stack;
};