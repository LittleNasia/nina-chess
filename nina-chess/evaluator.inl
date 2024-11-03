#include "evaluator.h"

forceinline Score get_score(const float wdl_chances)
{
	DEBUG_ASSERT(wdl_chances >= -1.0f && wdl_chances <= 1.0f);

	constexpr int32_t win_score = static_cast<int32_t>(Score::WIN);
	return static_cast<Score>(wdl_chances * win_score);
}

forceinline Score get_mated_score(const int64_t mate_in)
{
	DEBUG_ASSERT(mate_in >= 0);

	return Score(int32_t(Score::LOSS) + mate_in);
}

forceinline Evaluator::Evaluator(const std::string_view& weights_filename) :
	depth{ 0 },
	psqt{ std::ifstream{ weights_filename.data() } }
{
}

forceinline constexpr void Evaluator::Reset(PositionStack& position_stack)
{
	depth = 0;
	psqt.Reset(position_stack.GetPositionAt(0), position_stack.GetMoveListAt(0));

	for (int position_depth = 0; position_depth < position_stack.GetDepth(); position_depth++)
	{
		const auto& curr_position = position_stack.GetPositionAt(position_depth);
		const auto& curr_move_list = position_stack.GetMoveListAt(position_depth);

		if (curr_position.side_to_move == WHITE)
			IncrementalUpdate<WHITE>(curr_position, curr_move_list);
		else
			IncrementalUpdate<BLACK>(curr_position, curr_move_list);
	}
}

template<Color side_to_move>
forceinline constexpr Score Evaluator::Evaluate(const MoveList& move_list, const int64_t search_depth)
{
	validate_color<side_to_move>();
	if (move_list.GetNumMoves() == 0)
	{
		if (move_list.move_list_misc.checkers)
		{
			Score mated_score = get_mated_score(search_depth);
			ValidateScore(mated_score);
			return mated_score;
		}
		else
		{
			return Score::DRAW;
		}
	}

	Score score = get_score(psqt.Evaluate() * (side_to_move == Color::WHITE ? 1 : -1));
	ValidateScore(score);

	return score;
}

template<Color side_to_move>
forceinline constexpr void Evaluator::IncrementalUpdate(const Position& new_pos, const MoveList& move_list)
{
	validate_color<side_to_move>();
	depth++;
	psqt.IncrementalUpdate(new_pos, move_list);
}
