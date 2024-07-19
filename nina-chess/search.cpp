#include "search.h"

#include "move_gen.h"


SearchResult start_search(const Board& board, const size_t depth, TranspositionTable& tt)
{
	SearchResult result;
	const Position& position = board.GetPosition();

	if (position.side_to_move == WHITE)
	{
		result.score = search<Color::WHITE>(board, depth, Score::NEGATIVE_INF, Score::POSITIVE_INF, tt);
	}
	else
	{
		result.score = search<Color::BLACK>(board, depth, Score::NEGATIVE_INF, Score::POSITIVE_INF, tt);
	}

	result.pv[0] = tt.Get(position.hash).best_move;
	result.pv_length = 1;

	Position* curr_position = new Position(position::MakeMove(position, result.pv[0]));
	for (int curr_depth = 1; curr_depth <= depth; curr_depth++)
	{
		MoveList curr_position_moves = generate_moves(*curr_position);
		const auto& curr_tt_entry = tt.Get(curr_position->hash);

		for (size_t move_index = 0; move_index < curr_position_moves.get_num_moves(); move_index++)
		{
			if (curr_position_moves.moves[move_index] == curr_tt_entry.best_move)
			{
				result.pv[curr_depth] = curr_tt_entry.best_move;
				result.pv_length++;
				auto old_position = curr_position;
				curr_position = new Position(position::MakeMove(*curr_position, curr_tt_entry.best_move));
				delete old_position;
				break;
			}
		}
	}
	delete curr_position;

	return result;
}
