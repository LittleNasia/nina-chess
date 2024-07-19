#include <iostream>

forceinline Score get_score_from_tt(const Position& position, const size_t depth,
		const Score alpha, const Score beta, const TranspositionTable& tt)
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
			else if (tt_entry.flag == TTFlag::ALPHA && tt_entry.score <= alpha)
			{
				return alpha;
			}
			else if (tt_entry.flag == TTFlag::BETA && tt_entry.score >= beta)
			{
				return beta;
			}
		}
	}

	return Score::UNKNOWN;
}

template<Color side_to_move>
inline Score search(const Board& board, const size_t depth,
		Score alpha, Score beta, TranspositionTable& tt)
{
	constexpr Color opposite_side = get_opposite_color<side_to_move>();
	const Position& position = board.GetPosition();

	// is score in TT
	Score score;
	if ((score = get_score_from_tt(position, depth, alpha, beta, tt)) != Score::UNKNOWN)
	{
		return score;
	}
	// is this leaf node
	const MoveList& move_list = generate_moves<side_to_move>(position);
	if (depth == 0 || move_list.get_num_moves() == 0 || board.GetPosition().IsDrawn())
	{
		score = board.Evaluate(move_list);
		TranspositionTableEntry entry = { position.hash, score, depth, Move(), TTFlag::EXACT };
		tt.Insert(entry);
		return score;
	}

	// no
	TTFlag tt_flag = TTFlag::ALPHA;
	Move best_move;
	Score best_value = Score::NEGATIVE_INF;

	for (size_t move_index = 0; move_index < move_list.get_num_moves(); move_index++)
	{
		const Move& curr_move = move_list.moves[move_index];

		Board new_board = board.MakeMove<side_to_move>(curr_move);
		score = -search<opposite_side>(new_board, depth - 1, -beta, -alpha, tt);

		if (score > best_value)
		{
			best_value = score;
			best_move = curr_move;
		}
		if (score >= beta)
		{
			TranspositionTableEntry entry = { position.hash, score, depth, best_move, TTFlag::BETA };
			tt.Insert(entry);
			return beta;
		}
		else if (score > alpha)
		{
			tt_flag = TTFlag::EXACT;
			alpha = score;
		}
	}

	TranspositionTableEntry entry = { position.hash, score, depth, best_move, tt_flag };
	tt.Insert(entry);

	return alpha;
}