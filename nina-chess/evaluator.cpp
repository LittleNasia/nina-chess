#include "evaluator.h"

#include <stdexcept>

Score get_score(const float wdl_chances)
{
	DEBUG_IF(wdl_chances < -1.0f || wdl_chances > 1.0f)
	{
		throw std::runtime_error("wdl_chances must be in the range [-1.0, 1.0]");
	}

	constexpr int32_t win_score = static_cast<int32_t>(Score::WIN);
	return static_cast<Score>(wdl_chances * win_score);
}

Evaluator::Evaluator()
{

}

Score Evaluator::Evaluate(const Position& position)
{
	return get_score(0.0f);
}