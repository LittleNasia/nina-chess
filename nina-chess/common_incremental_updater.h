#pragma once
#include "utils.h"

#include "evaluator.h"
#include "position.h"
#include "position_stack.h"

class CommonIncrementalUpdater
{
protected:
	CommonIncrementalUpdater(Evaluator* evaluator, PositionStack* positionStack) :
		m_Evaluator(evaluator),
		m_PositionStack(positionStack)
	{}

public:
	forceinline constexpr Evaluator& GetEvaluator() const { return *m_Evaluator; }
	forceinline constexpr PositionStack& GetPositionStack() const { return *m_PositionStack; }
	template<Color sideToMove>
	forceinline constexpr void MakeMoveUpdate(const Move& move);
	forceinline constexpr void UndoMoveUpdate();
	template<Color sideToMove>
	forceinline constexpr void MoveGenerationUpdateWithoutGuard();

	// Move generation update is tricky, because it might not happen on every node (as we might return prematurely)
	// so we can't undo it on every single node, as we don't know whether we've hit the update or not
	// this object just ensures that if move generation update was done, it will be undone on the next return
	struct MoveGenerationUpdateGuard
	{
		forceinline constexpr MoveGenerationUpdateGuard(Evaluator* evaluator) :
			m_Evaluator(evaluator)
		{}

		forceinline constexpr ~MoveGenerationUpdateGuard()
		{
			m_Evaluator->UndoUpdate();
		}

	private:
		Evaluator* m_Evaluator;
	};

	template<Color sideToMove>
	[[nodiscard]] [[maybe_unused]] forceinline constexpr MoveGenerationUpdateGuard MoveGenerationUpdate();

private:
	Evaluator* m_Evaluator;
	PositionStack* m_PositionStack;
};


template<Color sideToMove>
forceinline constexpr void CommonIncrementalUpdater::MakeMoveUpdate(const Move& move)
{
	const Position& previousPosition = m_PositionStack->GetCurrentPosition();
	Position& newPosition = m_PositionStack->GetNextPosition();
	Position::MakeMove<sideToMove>(previousPosition, newPosition, move);

	m_PositionStack->IncrementDepth();
}

template<Color sideToMove>
forceinline constexpr void CommonIncrementalUpdater::MoveGenerationUpdateWithoutGuard()
{
	const auto& currentPosition = m_PositionStack->GetCurrentPosition();
	const auto& moveList = m_PositionStack->GetMoveList<sideToMove>();

	m_Evaluator->IncrementalUpdate<sideToMove>(currentPosition, moveList);
}

template<Color sideToMove>
forceinline constexpr CommonIncrementalUpdater::MoveGenerationUpdateGuard CommonIncrementalUpdater::MoveGenerationUpdate()
{
	MoveGenerationUpdateWithoutGuard<sideToMove>();

	return MoveGenerationUpdateGuard(m_Evaluator);
}

forceinline constexpr void CommonIncrementalUpdater::UndoMoveUpdate()
{
	m_PositionStack->DecrementDepth();
}