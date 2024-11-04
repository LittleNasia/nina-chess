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

#include "evaluator.inl"