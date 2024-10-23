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
	forceinline Evaluator(const std::string_view& weights_filename);
	
	forceinline constexpr void Reset(PositionStack& position_stack);

	template<Color side_to_move>
	forceinline constexpr Score Evaluate(const Position& position, const MoveList& move_list, const int64_t search_depth);

	template<Color side_to_move>
	forceinline constexpr void IncrementalUpdate(const Position& new_pos, const MoveList& move_list);

	forceinline constexpr void UndoUpdate() { psqt.UndoUpdate(); depth--; }
private:
	int depth;

	PSQT psqt;
};

#include "evaluator.inl"