#pragma once
#include "utils.h"

#include "evaluator.h"
#include "move_gen.h"
#include "position.h"
#include "search_stack.h"

class IncrementalUpdater
{
public:
	IncrementalUpdater() :
		evaluator(std::make_unique<Evaluator>("weights")),
		search_stack(std::make_unique<SearchStack>())
	{}

	Evaluator& GetEvaluator() const { return *evaluator; }
	SearchStack& GetSearchStack() const { return *search_stack; }
	
	forceinline void Reset()
	{
		search_stack->Reset();
		evaluator->Reset(*search_stack);
	}

	forceinline void Reset(const Position& pos)
	{
		search_stack->Reset(pos);
		evaluator->Reset(*search_stack);
	}

	template<Color side_to_move>
	forceinline constexpr void FullUpdate(const Move& move)
	{
		MakeMoveUpdate<side_to_move>(move);
		MoveGenerationUpdateWithoutGuard<get_opposite_color<side_to_move>()>();
	}

	template<Color side_to_move>
	forceinline constexpr void MakeMoveUpdate(const Move& move)
	{
		const Position& prev_pos = search_stack->GetCurrentPosition();
		Position& new_pos = search_stack->GetNextPosition();
		position::MakeMove<side_to_move>(prev_pos, new_pos, move);

		search_stack->IncrementDepth();
	}

	forceinline constexpr void UndoUpdate()
	{
		search_stack->DecrementDepth();
	}

	template<Color side_to_move>
	forceinline constexpr void MoveGenerationUpdateWithoutGuard()
	{
		const auto& curr_pos = search_stack->GetCurrentPosition();
		const auto& move_list = search_stack->GetMoveList<side_to_move>();

		evaluator->IncrementalUpdate<side_to_move>(curr_pos, move_list);
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
	[[nodiscard]] forceinline constexpr MoveGenerationUpdateGuard MoveGenerationUpdate()
	{
		MoveGenerationUpdateWithoutGuard<side_to_move>();

		return MoveGenerationUpdateGuard(evaluator.get());
	}

	forceinline constexpr void UndoMoveGenerationUpdate()
	{
		evaluator->UndoUpdate();
	}

private:
	std::unique_ptr<Evaluator> evaluator;
	std::unique_ptr<SearchStack> search_stack;
};