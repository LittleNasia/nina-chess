#include "search.h"

#include "incremental_updater.h"
#include "move_gen.h"

forceinline Score get_score_from_tt(const Position& position,
	const AlphaBeta& alpha_beta, const SearchNecessities& search_necessities, const SearchStack& search_stack)
{
	const TranspositionTableEntry& tt_entry = search_necessities.GetTranspositionTable().Get(position.hash);

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

template<Color side_to_move, bool is_root_node = false>
inline Score search(AlphaBeta alpha_beta, const SearchNecessities& search_necessities, IncrementalUpdater& incremental_updater, const SearchConstraints& search_constraints)
{
	auto& search_stack = incremental_updater.GetSearchStack();

	constexpr Color opposite_side = get_opposite_color<side_to_move>();
	const Position& position = search_stack.GetCurrentPosition();

	// check for search aborted
	if ((search_stack.nodes & 0xFFF) == 0xFFF)
	{
		const int64_t time_taken = search_stack.GetSearchDurationInMs();
		if (time_taken >= (search_constraints.time * 0.95))
		{
			search_stack.AbortSearch();
			return Score::UNKNOWN;
		}
	}
	
	Score score;
	if (!is_root_node)
	{
		// is position drawn
		if (position.IsDrawn() || search_stack.IsThreefoldRepetition())
		{
			search_stack.nodes++;

			const size_t random_seed = search_stack.nodes;
			return GetDrawValueWithSmallVariance(random_seed);
		}
	}

	// is score in TT
	if ((score = get_score_from_tt(position, alpha_beta, search_necessities, search_stack)) != Score::UNKNOWN)
	{
		search_stack.nodes++;
		return score;
	}
	

	// is leaf node for another reason
	const auto& guard = incremental_updater.MoveGenerationUpdate<side_to_move>();
	const MoveList& move_list = search_stack.GetMoveList<side_to_move>();
	if (search_stack.remaining_depth == 0 || move_list.GetNumMoves() == 0)
	{
		score = search_necessities.GetEvaluator().Evaluate<side_to_move>(position, move_list);

		const TranspositionTableEntry entry = { position.hash, score, search_stack.remaining_depth, Move(), TTFlag::EXACT };
		search_necessities.GetTranspositionTable().Insert(entry, is_root_node);

		search_stack.nodes++;
		return score;
	}

	// no
	TTFlag tt_flag = TTFlag::ALPHA;
	Move best_move;
	Score best_value = Score::NEGATIVE_INF;

	for (uint32_t move_index = 0; move_index < move_list.GetNumMoves(); move_index++)
	{
		if (search_stack.IsSearchAborted())
		{
			return Score::UNKNOWN;
		}

		const Move& curr_move = move_list[move_index];

		incremental_updater.MakeMoveUpdate<side_to_move>(curr_move);
		score = -search<opposite_side>(alpha_beta.Invert(), search_necessities, incremental_updater, search_constraints);
		incremental_updater.UndoUpdate();

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
				search_necessities.GetTranspositionTable().Insert(entry, is_root_node);

				return alpha_beta.beta;
			}
		
			tt_flag = TTFlag::EXACT;
			alpha_beta.alpha = score;
		}
	}

	const TranspositionTableEntry entry = { position.hash, score, search_stack.remaining_depth, best_move, tt_flag };
	search_necessities.GetTranspositionTable().Insert(entry, is_root_node);

	return alpha_beta.alpha;
}

template<Color color>
std::vector<SearchResult> iterative_deepening(IncrementalUpdater& incremental_updater, const SearchNecessities& search_necessities, const SearchConstraints& search_constraints)
{
	std::vector<SearchResult> search_results;

	for (int depth = 1; depth <= search_constraints.depth; depth++)
	{
		incremental_updater.GetSearchStack().nodes = 0;
		SearchConstraints current_search_constraints = search_constraints;
		current_search_constraints.depth = depth;

		incremental_updater.GetSearchStack().remaining_depth = depth;

		AlphaBeta alpha_beta = { Score::NEGATIVE_INF, Score::POSITIVE_INF };

		auto start_timepoint = std::chrono::high_resolution_clock::now();
		const auto score = search<color, true>(alpha_beta, search_necessities, incremental_updater, current_search_constraints);
		auto end_timepoint = std::chrono::high_resolution_clock::now();
		auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end_timepoint - start_timepoint).count();

		if (incremental_updater.GetSearchStack().IsSearchAborted())
		{
			break;
		}

		SearchResult result;
		result.nodes = incremental_updater.GetSearchStack().nodes;
		result.score = score;
		result.depth = depth;

		const Position& root_pos = incremental_updater.GetSearchStack().GetCurrentPosition();

		Position curr_position = root_pos;
		for (int pv_depth = 0; pv_depth < depth; pv_depth++)
		{
			MoveList curr_position_moves = generate_moves(curr_position, incremental_updater.GetSearchStack().GetMoveList());
			const auto& curr_tt_entry = search_necessities.GetTranspositionTable().Get(curr_position.hash);

			if (curr_tt_entry.best_move.from() == 0 && curr_tt_entry.best_move.to() == 0)
				break;
			for (uint32_t move_index = 0; move_index < curr_position_moves.GetNumMoves(); move_index++)
			{
				if (curr_position_moves[move_index] == curr_tt_entry.best_move)
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

		DEBUG_IF(result.pv_length == 0)
		{
			throw std::runtime_error("PV length is 0");
		}
		DEBUG_IF(result.score == Score::UNKNOWN || result.score == -Score::UNKNOWN)
		{
			throw std::runtime_error("Score is unknown");
		}

		result.PrintUciInfo(duration);

		search_results.push_back(result);
	}

	return search_results;
}

std::vector<SearchResult> start_search(IncrementalUpdater& incremental_updater, const SearchNecessities& search_necessities, const SearchConstraints& search_constraints)
{
	std::vector<SearchResult> results;

	const int depth_to_search_to = search_constraints.depth <= 0 ? max_depth : search_constraints.depth;
	const int time_limit = search_constraints.time == -1 ? std::numeric_limits<int>::max() : int(search_constraints.time * 0.05);
	std::cout << "info time limit " << time_limit << std::endl;
	
	SearchConstraints current_search_constraints = search_constraints;
	current_search_constraints.depth = depth_to_search_to;
	if(search_constraints.movetime != -1)
		current_search_constraints.time = search_constraints.movetime < time_limit ? search_constraints.movetime : time_limit;
	else
		current_search_constraints.time = time_limit;

	const Position& root_position = incremental_updater.GetSearchStack().GetCurrentPosition();

	if (root_position.side_to_move == Color::WHITE)
	{
		results = iterative_deepening<Color::WHITE>(incremental_updater, search_necessities, current_search_constraints);
	}
	else
	{
		results = iterative_deepening<Color::BLACK>(incremental_updater, search_necessities, current_search_constraints);
	}

	std::cout << "bestmove " << results.back().pv[0].ToUciMove() << std::endl;

	return results;
}
