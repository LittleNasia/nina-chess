#pragma once
#include "utils.h"

#include <fstream>
#include <memory>

#include "chess.h"
#include "move_list.h"
#include "position_stack.h"
#include "psqt.h"
#include "score.h"

class Evaluator
{
public:
	forceinline Evaluator();
	forceinline Evaluator(const std::string_view& weightsFilename);
	
	forceinline constexpr void Reset(PositionStack& positionStack);

	template<Color sideToMove>
	forceinline constexpr Score Evaluate(const MoveList& moveList, const int64_t searchDepth);

	template<Color sideToMove>
	forceinline constexpr void IncrementalUpdate(const Position& newPos, const MoveList& moveList);

	forceinline constexpr void UndoUpdate() { m_PSQT.UndoUpdate(); m_Depth--; }
private:
	int m_Depth;

	PSQT m_PSQT;
};

forceinline Score GetScore(const float wdlChances)
{
	DEBUG_ASSERT(wdlChances >= -1.0f && wdlChances <= 1.0f);

	constexpr int32_t winScore = static_cast<int32_t>(Score::WIN);
	return static_cast<Score>(wdlChances * winScore);
}

forceinline Score GetMatedScore(const int64_t mateIn)
{
	DEBUG_ASSERT(mateIn >= 0);

	return Score(int32_t(Score::LOSS) + mateIn);
}

forceinline Evaluator::Evaluator() :
	Evaluator("weights")
{
}

forceinline Evaluator::Evaluator(const std::string_view& weightsFilename) :
	m_Depth{ 0 },
	m_PSQT{ std::ifstream{ weightsFilename.data() } }
{
}

forceinline constexpr void Evaluator::Reset(PositionStack& positionStack)
{
	m_Depth = 0;
	m_PSQT.Reset(positionStack.GetPositionAt(0), positionStack.GetMoveListAt(0));

	for (int positionDepth = 0; positionDepth < positionStack.GetDepth(); positionDepth++)
	{
		const auto& currentPosition = positionStack.GetPositionAt(positionDepth);
		const auto& currentMoveList = positionStack.GetMoveListAt(positionDepth);

		if (currentPosition.SideToMove == WHITE)
			IncrementalUpdate<WHITE>(currentPosition, currentMoveList);
		else
			IncrementalUpdate<BLACK>(currentPosition, currentMoveList);
	}
}

template<Color sideToMove>
forceinline constexpr Score Evaluator::Evaluate(const MoveList& moveList, const int64_t searchDepth)
{
	ValidateColor<sideToMove>();
	if (moveList.GetNumMoves() == 0)
	{
		if (moveList.MoveListMisc.Checkers)
		{
			Score matedScore = GetMatedScore(searchDepth);
			ValidateScore(matedScore);
			return matedScore;
		}
		else
		{
			return Score::DRAW;
		}
	}

	Score score = GetScore(m_PSQT.Evaluate() * (sideToMove == Color::WHITE ? 1 : -1));
	ValidateScore(score);

	return score;
}

template<Color sideToMove>
forceinline constexpr void Evaluator::IncrementalUpdate(const Position& newPosition, const MoveList& moveList)
{
	ValidateColor<sideToMove>();
	m_Depth++;
	m_PSQT.IncrementalUpdate(newPosition, moveList);
}
