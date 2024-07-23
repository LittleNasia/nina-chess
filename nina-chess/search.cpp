#include "search.h"

#include "move_gen.h"

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
inline Score search(AlphaBeta alpha_beta, SearchStack& search_stack)
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
	if (position.IsDrawn() || search_stack.IsThreefoldRepetition())
	{
		search_stack.nodes++;
		return Score::DRAW;
	}

	// is leaf node for another reason
	const MoveList& move_list = search_stack.GetMoveList();
	generate_moves<side_to_move>(position, search_stack.GetMoveList());
	if (search_stack.remaining_depth == 0 || move_list.get_num_moves() == 0)
	{
		score = search_stack.GetEvaluator().Evaluate(position, move_list, search_stack.depth);

		const TranspositionTableEntry entry = { position.hash, score, search_stack.remaining_depth, Move(), TTFlag::EXACT };
		search_stack.GetTranspositionTable().Insert(entry);

		search_stack.nodes++;
		return score;
	}

	// no
	TTFlag tt_flag = TTFlag::ALPHA;
	Move best_move;
	Score best_value = Score::NEGATIVE_INF;

	Position& new_pos = search_stack.GetNextPosition();
	for (size_t move_index = 0; move_index < move_list.get_num_moves(); move_index++)
	{
		const Move& curr_move = move_list.moves[move_index];
		position::MakeMove<side_to_move>(position, new_pos, curr_move);
		search_stack.SetNextPositionHash();

		search_stack.IncrementDepth();
		score = -search<opposite_side>(alpha_beta.Invert(), search_stack);
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


SearchResult start_search(const Position& position, const int depth, TranspositionTable& tt, Evaluator& evaluator)
{
	SearchResult result;
	
	SearchStack search_stack(depth, tt, evaluator);
	search_stack.SetCurrentPosition(position);
	search_stack.SetCurrentPositionHash();

	if (position.side_to_move == WHITE)
	{
		AlphaBeta alpha_beta = { Score::NEGATIVE_INF, Score::POSITIVE_INF };
		result.score = search<Color::WHITE>(alpha_beta, search_stack);
	}
	else
	{
		AlphaBeta alpha_beta = { Score::NEGATIVE_INF, Score::POSITIVE_INF };
		result.score = search<Color::BLACK>(alpha_beta, search_stack);
	}

	result.pv[0] = tt.Get(position.hash).best_move;
	result.pv_length = 1;
	result.nodes = search_stack.nodes;

	Position curr_position;
	position::MakeMove(search_stack.GetCurrentPosition(), curr_position, result.pv[0]);
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
