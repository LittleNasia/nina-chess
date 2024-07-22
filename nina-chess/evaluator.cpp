#include "evaluator.h"

Score get_score(const float wdl_chances)
{
	DEBUG_IF(wdl_chances < -1.0f || wdl_chances > 1.0f)
	{
		throw std::runtime_error("wdl_chances must be in the range [-1.0, 1.0]");
	}

	constexpr int32_t win_score = static_cast<int32_t>(Score::WIN);
	return static_cast<Score>(wdl_chances * win_score);
}

Score get_mated_score(const int32_t mate_in)
{
	DEBUG_IF(mate_in < 0)
	{
		throw std::runtime_error("mate_in must be non-negative");
	}

	return Score(int32_t(Score::LOSS) + mate_in);
}

Evaluator::Evaluator()
{

}

Score Evaluator::Evaluate(const Position& position, const MoveList& move_list, const SearchStack& search_stack)
{
	if (move_list.get_num_moves() == 0)
	{
		return move_list.checkers ? get_mated_score(search_stack.depth) : get_score(0.0f);
	}

	float score = (float)(int(popcnt(position.GetSide<WHITE>().pieces)) - int(popcnt(position.GetSide<BLACK>().pieces))) / 16.f;
	score *= (position.side_to_move == WHITE ? 1 : -1);

	return get_score(score);
}