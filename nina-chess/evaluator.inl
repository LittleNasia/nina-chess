#include "evaluator.h"
#include "weights.h"

forceinline Evaluator::Evaluator(const std::string_view& weights_filename) :
	depth{ 0 },
	psqt{ }
{
	std::ifstream file{ weights_filename.data() };
	if (!file.is_open())
	{
		throw std::runtime_error("Could not open file");
	}

	ReadWeights(file);
}

forceinline constexpr void Evaluator::Reset(SearchStack& search_stack)
{
	depth = 0;
	psqt->Reset(search_stack.GetPositionAt(0), search_stack.GetMoveListAt(0));

	for (int depth = 0; depth < search_stack.depth; depth++)
	{
		const auto& curr_position = search_stack.GetPositionAt(depth);
		const auto& curr_move_list = search_stack.GetMoveListAt(depth);

		if (curr_position.side_to_move == WHITE)
			IncrementalUpdate<WHITE>(curr_position, curr_move_list);
		else
			IncrementalUpdate<BLACK>(curr_position, curr_move_list);
	}
}

forceinline constexpr void Evaluator::ReadWeights(std::ifstream& file)
{
	// for now we're hardcoding the weights so we don't give a little pik about file reading
	// we'll implement this later
	// for now we're just going to ignore the file

	psqt = std::make_unique<PSQT>(file);
}


template<Color side_to_move>
forceinline constexpr Score Evaluator::Evaluate(const Position& position, const MoveList& move_list)
{
	if (move_list.GetNumMoves() == 0)
	{
		if (move_list.move_list_misc.checkers)
		{
			return get_mated_score(depth);
		}
		else
		{
			return Score::DRAW;
		}
	}

	return get_score(psqt->Evaluate() * (side_to_move == Color::WHITE ? 1 : -1));
}

template<Color side_to_move>
forceinline constexpr void Evaluator::IncrementalUpdate(const Position& new_pos, const MoveList& move_list)
{
	depth++;
	psqt->IncrementalUpdate(new_pos, move_list);
}
