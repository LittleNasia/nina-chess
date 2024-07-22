#include <iostream>

forceinline Score get_score_from_tt(const Position& position,
		const AlphaBeta& alpha_beta, const SearchStack& search_stack)
{
	const TranspositionTableEntry& tt_entry = search_stack.GetTranspositionTable().Get(position.hash);

	if (tt_entry.key == position.hash)
	{
		if (tt_entry.depth >= search_stack.remaining_depth)
		{
			if (tt_entry.flag == TTFlag::EXACT)
			{
				return tt_entry.score;
			}
			else if (tt_entry.flag == TTFlag::ALPHA &&
				tt_entry.score <= alpha_beta.alpha)
			{
				return alpha_beta.alpha;
			}
			else if (tt_entry.flag == TTFlag::BETA &&
				tt_entry.score >= alpha_beta.beta)
			{
				return alpha_beta.beta;
			}
		}
	}

	return Score::UNKNOWN;
}

template<Color side_to_move>
inline Score search(const Board& board, AlphaBeta alpha_beta, SearchStack& search_stack)
{
	constexpr Color opposite_side = get_opposite_color<side_to_move>();
	const Position& position = search_stack.GetCurrentPosition();

	// is score in TT
	Score score;
	if ((score = get_score_from_tt(position, alpha_beta, search_stack)) != Score::UNKNOWN)
	{
		search_stack.nodes++;
		return score;
	}

	// is position drawn
	if(board.IsDrawn(search_stack))
	{
		search_stack.nodes++;
		return Score::DRAW;
	}

	// is leaf node for another reason
	const MoveList& move_list = search_stack.GetMoveList();
	generate_moves<side_to_move>(position, search_stack.GetMoveList());
	if (search_stack.remaining_depth == 0 || move_list.get_num_moves() == 0)
	{
		score = board.Evaluate(move_list, search_stack);

		const TranspositionTableEntry entry = { position.hash, score, search_stack.remaining_depth, Move(), TTFlag::EXACT };
		search_stack.GetTranspositionTable().Insert(entry);

		search_stack.nodes++;
		return score;
	}

	// no
	TTFlag tt_flag = TTFlag::ALPHA;
	Move best_move;
	Score best_value = Score::NEGATIVE_INF;

	for (size_t move_index = 0; move_index < move_list.get_num_moves(); move_index++)
	{
		const Move& curr_move = move_list.moves[move_index];
		const Board& new_board = board.MakeMove<side_to_move>(curr_move, search_stack);

		search_stack.IncrementDepth();
		score = -search<opposite_side>(new_board, alpha_beta.Invert(), search_stack);
		search_stack.DecrementDepth();

		if (score > best_value)
		{
			best_value = score;
			best_move = curr_move;
		}

		if (score >= alpha_beta.beta)
		{
			const TranspositionTableEntry entry = { position.hash, score, search_stack.remaining_depth, best_move, TTFlag::BETA };
			search_stack.GetTranspositionTable().Insert(entry);

			return alpha_beta.beta;
		}
		else if (score > alpha_beta.alpha)
		{
			tt_flag = TTFlag::EXACT;
			alpha_beta.alpha = score;
		}
	}

	const TranspositionTableEntry entry = { position.hash, score, search_stack.remaining_depth, best_move, tt_flag };
	search_stack.GetTranspositionTable().Insert(entry);

	return alpha_beta.alpha;
}