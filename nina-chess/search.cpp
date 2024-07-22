#include "search.h"

#include "move_gen.h"


SearchResult start_search(const Board& board, const int depth, TranspositionTable& tt)
{
	SearchResult result;
	const Position& position = board.GetPosition();
	
	SearchStack search_stack(depth, tt);
	search_stack.SetCurrentPosition(board.GetPosition());
	search_stack.SetCurrentPositionHash();

	if (position.side_to_move == WHITE)
	{
		AlphaBeta alpha_beta = { Score::NEGATIVE_INF, Score::POSITIVE_INF };
		result.score = search<Color::WHITE>(board, alpha_beta, search_stack);
	}
	else
	{
		AlphaBeta alpha_beta = { Score::NEGATIVE_INF, Score::POSITIVE_INF };
		result.score = search<Color::BLACK>(board, alpha_beta, search_stack);
	}

	result.pv[0] = tt.Get(position.hash).best_move;
	result.pv_length = 1;
	result.nodes = search_stack.nodes;

	Position curr_position;
	position::MakeMove(board.GetPosition(), curr_position, result.pv[0]);
	for (int curr_depth = 1; curr_depth <= depth; curr_depth++)
	{
		MoveList curr_position_moves = generate_moves(curr_position, search_stack.GetMoveList());
		const auto& curr_tt_entry = tt.Get(curr_position.hash);

		for (size_t move_index = 0; move_index < curr_position_moves.get_num_moves(); move_index++)
		{
			if (curr_position_moves.moves[move_index] == curr_tt_entry.best_move)
			{
				result.pv[curr_depth] = curr_tt_entry.best_move;
				result.pv_length++;
				Position new_position;
				position::MakeMove(curr_position, new_position, result.pv[curr_depth]);
				curr_position = new_position;
				break;
			}
		}
	}

	return result;
}
