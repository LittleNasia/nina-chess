#pragma once
#include "utils.h"

#include <fstream>
#include <memory>

#include "move_list.h"
#include "position.h"
#include "psqt.h"
#include "search_stack.h"

class Evaluator
{
public:
	forceinline Evaluator(const std::string_view& weights_filename);
	
	forceinline constexpr void Reset(SearchStack& search_stack);

	template<Color side_to_move>
	forceinline constexpr Score Evaluate(const Position& position, const MoveList& move_list);

	template<Color side_to_move>
	forceinline constexpr void IncrementalUpdate(const Position& new_pos, const MoveList& move_list);

	forceinline constexpr void UndoUpdate() { psqt->UndoUpdate(); depth--; }
private:
	forceinline constexpr void ReadWeights(std::ifstream& file);
	int depth;

	std::unique_ptr<PSQT> psqt;
};

forceinline Score get_score(const float wdl_chances)
{
	DEBUG_IF(wdl_chances < -1.0f || wdl_chances > 1.0f)
	{
		throw std::runtime_error("wdl_chances must be in the range [-1.0, 1.0]");
	}

	constexpr int32_t win_score = static_cast<int32_t>(Score::WIN);
	return static_cast<Score>(wdl_chances * win_score);
}

forceinline Score get_mated_score(const int32_t mate_in)
{
	DEBUG_IF(mate_in < 0)
	{
		throw std::runtime_error("mate_in must be non-negative");
	}

	return Score(int32_t(Score::LOSS) + mate_in);
}

#include "evaluator.inl"