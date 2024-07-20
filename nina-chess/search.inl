#include <iostream>

forceinline Score get_score_from_tt(const Position& position, const size_t depth,
		const AlphaBeta& alpha_beta, const TranspositionTable& tt)
{
	const TranspositionTableEntry& tt_entry = tt.Get(position.hash);
	if (tt_entry.key == position.hash)
	{
		if (tt_entry.depth >= depth)
		{
			if (tt_entry.flag == TTFlag::EXACT)
			{
				return tt_entry.score;
			}
			else if (tt_entry.flag == TTFlag::ALPHA && tt_entry.score <= alpha_beta.alpha)
			{
				return alpha_beta.alpha;
			}
			else if (tt_entry.flag == TTFlag::BETA && tt_entry.score >= alpha_beta.beta)
			{
				return alpha_beta.beta;
			}
		}
	}

	return Score::UNKNOWN;
}

template<Color side_to_move>
inline Score search(const Board& board, AlphaBeta alpha_beta, SearchInfo& search_info)
{
	constexpr Color opposite_side = get_opposite_color<side_to_move>();
	const Position& position = board.GetPosition();

	// is score in TT
	Score score;
	if ((score = get_score_from_tt(position, search_info.remaining_depth, alpha_beta, search_info.tt)) != Score::UNKNOWN)
	{
		return score;
	}
	// is this leaf node
	generate_moves<side_to_move>(position, search_info.GetMoveList());
	const MoveList& move_list = search_info.GetMoveList();
	if (search_info.remaining_depth == 0 || move_list.get_num_moves() == 0 || board.GetPosition().IsDrawn())
	{
		score = board.Evaluate(move_list);
		const TranspositionTableEntry entry = { position.hash, score, search_info.remaining_depth, Move(), TTFlag::EXACT };
		search_info.tt.Insert(entry);
		return score;
	}

	// no
	TTFlag tt_flag = TTFlag::ALPHA;
	Move best_move;
	Score best_value = Score::NEGATIVE_INF;

	for (size_t move_index = 0; move_index < move_list.get_num_moves(); move_index++)
	{
		const Move& curr_move = move_list.moves[move_index];

		const Board& new_board = board.MakeMove<side_to_move>(curr_move);

		search_info.IncrementDepth();
		score = -search<opposite_side>(new_board, alpha_beta.Invert(), search_info);
		search_info.DecrementDepth();

		if (score > best_value)
		{
			best_value = score;
			best_move = curr_move;
		}
		if (score >= alpha_beta.beta)
		{
			const TranspositionTableEntry entry = { position.hash, score, search_info.remaining_depth, best_move, TTFlag::BETA };
			search_info.tt.Insert(entry);
			return alpha_beta.beta;
		}
		else if (score > alpha_beta.alpha)
		{
			tt_flag = TTFlag::EXACT;
			alpha_beta.alpha = score;
		}
	}

	const TranspositionTableEntry entry = { position.hash, score, search_info.remaining_depth, best_move, tt_flag };
	search_info.tt.Insert(entry);

	return alpha_beta.alpha;
}