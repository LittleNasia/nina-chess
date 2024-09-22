#pragma once
#include "utils.h"

#include "evaluator.h"
#include "position.h"
#include "position_stack.h"

class CommonIncrementalUpdater
{
protected:
	CommonIncrementalUpdater(Evaluator* evaluator, PositionStack* position_stack) :
		evaluator(evaluator),
		position_stack(position_stack)
	{}

public:
	forceinline constexpr Evaluator& GetEvaluator() const { return *evaluator; }
	forceinline constexpr PositionStack& GetPositionStack() const { return *position_stack; }
	template<Color side_to_move>
	forceinline constexpr void MakeMoveUpdate(const Move& move);
	forceinline constexpr void UndoMoveUpdate(); 
	template<Color side_to_move>
	forceinline constexpr void MoveGenerationUpdateWithoutGuard();

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
	[[nodiscard]] forceinline constexpr MoveGenerationUpdateGuard MoveGenerationUpdate();

private:
	Evaluator* evaluator;
	PositionStack* position_stack;
};


template<Color side_to_move>
inline constexpr void CommonIncrementalUpdater::MakeMoveUpdate(const Move& move)
{
	const Position& prev_pos = position_stack->GetCurrentPosition();
	Position& new_pos = position_stack->GetNextPosition();
	position::MakeMove<side_to_move>(prev_pos, new_pos, move);

	position_stack->IncrementDepth();
}

template<Color side_to_move>
inline constexpr void CommonIncrementalUpdater::MoveGenerationUpdateWithoutGuard()
{
	const auto& curr_pos = position_stack->GetCurrentPosition();
	const auto& move_list = position_stack->GetMoveList<side_to_move>();

	evaluator->IncrementalUpdate<side_to_move>(curr_pos, move_list);
}

template<Color side_to_move>
inline constexpr CommonIncrementalUpdater::MoveGenerationUpdateGuard CommonIncrementalUpdater::MoveGenerationUpdate()
{
	MoveGenerationUpdateWithoutGuard<side_to_move>();

	return MoveGenerationUpdateGuard(evaluator);
}

inline constexpr void CommonIncrementalUpdater::UndoMoveUpdate()
{
	position_stack->DecrementDepth();
}