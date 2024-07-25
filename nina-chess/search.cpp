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
inline Score search(AlphaBeta alpha_beta, SearchStack& search_stack, const SearchConstraints& search_constraints)
{
	constexpr Color opposite_side = get_opposite_color<side_to_move>();
	const Position& position = search_stack.GetCurrentPosition();

	// check for search aborted
	if ((search_stack.nodes & 0xFFF) == 0xFFF)
	{
		const int time_taken = search_stack.GetSearchDurationInMs();
		if (time_taken >= (search_constraints.time * 0.95))
		{
			search_stack.AbortSearch();
			return Score::UNKNOWN;
		}
	}

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
		if (search_stack.IsSearchAborted())
		{
			return Score::UNKNOWN;
		}

		const Move& curr_move = move_list.moves[move_index];
		position::MakeMove<side_to_move>(position, new_pos, curr_move);
		search_stack.SetNextPositionHash();

		search_stack.IncrementDepth();
		score = -search<opposite_side>(alpha_beta.Invert(), search_stack, search_constraints);
		search_stack.DecrementDepth();

		if (score > best_value)
		{
			best_value = score;
			best_move = curr_move;
		}

		if (score > alpha_beta.alpha)
		{
			if (score >= alpha_beta.beta)
			{
				const TranspositionTableEntry entry = { position.hash, score, search_stack.remaining_depth, best_move, TTFlag::BETA };
				search_stack.GetTranspositionTable().Insert(entry);

				return alpha_beta.beta;
			}
		
			tt_flag = TTFlag::EXACT;
			alpha_beta.alpha = score;
		}
	}

	const TranspositionTableEntry entry = { position.hash, score, search_stack.remaining_depth, best_move, tt_flag };
	search_stack.GetTranspositionTable().Insert(entry);

	return alpha_beta.alpha;
}

template<Color color>
std::vector<SearchResult> iterative_deepening(SearchStack& search_stack, const SearchConstraints& search_constraints)
{
	std::vector<SearchResult> search_results;
	for (int depth = 1; depth <= search_constraints.depth; depth++)
	{
		SearchConstraints current_search_constraints = search_constraints;
		current_search_constraints.depth = depth;

		search_stack.remaining_depth = depth;

		AlphaBeta alpha_beta = { Score::NEGATIVE_INF, Score::POSITIVE_INF };

		auto start_timepoint = std::chrono::high_resolution_clock::now();
		const auto& score = search<color>(alpha_beta, search_stack, current_search_constraints);
		auto end_timepoint = std::chrono::high_resolution_clock::now();
		auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end_timepoint - start_timepoint).count();

		if (search_stack.IsSearchAborted())
		{
			break;
		}
		
		SearchResult result;
		result.nodes = search_stack.nodes;
		result.score = score;
		result.depth = depth;

		const Position& root_pos = search_stack.GetCurrentPosition();

		result.pv[0] = search_stack.GetTranspositionTable().Get(root_pos.hash).best_move;
		result.pv_length = 1;

		Position curr_position;
		position::MakeMove(root_pos, curr_position, result.pv[0]);
		for (int pv_depth = 1; pv_depth < depth; pv_depth++)
		{
			MoveList curr_position_moves = generate_moves(curr_position, search_stack.GetMoveList());
			const auto& curr_tt_entry = search_stack.GetTranspositionTable().Get(curr_position.hash);

			if (curr_tt_entry.best_move.from() == 0 && curr_tt_entry.best_move.to() == 0)
				break;
			for (size_t move_index = 0; move_index < curr_position_moves.get_num_moves(); move_index++)
			{
				if (curr_position_moves.moves[move_index] == curr_tt_entry.best_move)
				{
					result.pv[pv_depth] = curr_tt_entry.best_move;
					result.pv_length++;
					Position new_position;
					position::MakeMove(curr_position, new_position, result.pv[pv_depth]);
					curr_position = new_position;
					break;
				}
			}
		}

		result.PrintUciInfo(duration);

		search_results.push_back(result);
	}

	return search_results;
}

std::vector<SearchResult> start_search(std::vector<Position> position_stack, TranspositionTable& tt, Evaluator& evaluator, const SearchConstraints& search_constraints)
{
	std::vector<SearchResult> results;

	const int depth_to_search_to = search_constraints.depth <= 0 ? max_depth : search_constraints.depth;
	const int time_limit = search_constraints.time == -1 ? std::numeric_limits<int>::max() : int(search_constraints.time * 0.05);
	std::cout << "info time limit " << time_limit << std::endl;
	
	SearchStack search_stack(position_stack, depth_to_search_to, tt, evaluator);
	search_stack.StartSearch();

	SearchConstraints current_search_constraints = search_constraints;
	current_search_constraints.depth = depth_to_search_to;
	if(search_constraints.movetime != -1)
		current_search_constraints.time = search_constraints.movetime < time_limit ? search_constraints.movetime : time_limit;
	else
		current_search_constraints.time = time_limit;

	const Position& root_position = search_stack.GetCurrentPosition();

	if (root_position.side_to_move == Color::WHITE)
	{
		results = iterative_deepening<Color::WHITE>(search_stack, current_search_constraints);
	}
	else
	{
		results = iterative_deepening<Color::BLACK>(search_stack, current_search_constraints);
	}

	std::cout << "bestmove " << results.back().pv[0].ToUciMove() << std::endl;

	return results;
}
