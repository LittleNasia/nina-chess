#include "search.h"

#include "individual_search_context.h"
#include "move_gen.h"
#include "uci_incremental_updater.h"

forceinline Score get_score_from_tt(const Position& position,
	const AlphaBeta& alpha_beta, const TranspositionTable& transposition_table, const int64_t remaining_depth)
{
	const TranspositionTableEntry& tt_entry = transposition_table.Get(position.hash);

	if (tt_entry.key == position.hash)
	{
		if (tt_entry.depth >= remaining_depth)
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
inline Score search(AlphaBeta alpha_beta, SearchIncrementalUpdater& incremental_updater, IndividualSearchContext& search_context)
{
	auto& nodes = search_context.nodes;
	auto& cancellation_policy = static_cast<SharedSearchContext&>(search_context).GetCancellationPolicy();
	auto& transposition_table = static_cast<SharedSearchContext&>(search_context).GetTranspositionTable();
	auto& position_stack = incremental_updater.GetPositionStack();
	auto& evaluator = incremental_updater.GetEvaluator();

	constexpr Color opposite_side = get_opposite_color<side_to_move>();
	const Position& position = position_stack.GetCurrentPosition();

	// check for search aborted
	if ((search_context.nodes & 0xFFF) == 0xFFF)
		if (cancellation_policy.CheckForAbort(nodes))
			return Score::UNKNOWN;
	
	Score score;
	if (!is_root_node)
	{
		// is position drawn
		if (position.IsDrawn() || position_stack.IsThreefoldRepetition())
		{
			nodes++;

			const size_t random_seed = nodes;
			return GetDrawValueWithSmallVariance(random_seed);
		}
	}

	// is score in TT
	if ((score = get_score_from_tt(position, alpha_beta, transposition_table, incremental_updater.GetRemainingDepth())) != Score::UNKNOWN)
	{
		nodes++;
		ValidateScore(score);
		return score;
	}
	

	// is leaf node for another reason
	const auto& guard = incremental_updater.MoveGenerationUpdate<side_to_move>();
	const MoveList& move_list = position_stack.GetMoveList<side_to_move>();
	if (incremental_updater.GetRemainingDepth() == 0 || move_list.GetNumMoves() == 0)
	{
		score = evaluator.Evaluate<side_to_move>(position, move_list, incremental_updater.GetSearchDepth());
		ValidateScore(score);

		const TranspositionTableEntry entry = { position.hash, score, incremental_updater.GetRemainingDepth(), Move(), TTFlag::EXACT };
		transposition_table.Insert(entry, is_root_node);

		nodes++;
		return score;
	}

	// no
	TTFlag tt_flag = TTFlag::ALPHA;
	Move best_move;
	Score best_value = Score::NEGATIVE_INF;

	for (uint32_t move_index = 0; move_index < move_list.GetNumMoves(); move_index++)
	{
		const Move& curr_move = move_list[move_index];

		incremental_updater.MakeMoveUpdate<side_to_move>(curr_move);
		score = -search<opposite_side>(alpha_beta.Invert(), incremental_updater, search_context);
		incremental_updater.UndoMoveUpdate();

		if (cancellation_policy.IsAborted())
		{
			return Score::UNKNOWN;
		}

		ValidateScore(score);

		if (score > best_value)
		{
			best_value = score;
			best_move = curr_move;
		}

		if (score > alpha_beta.alpha)
		{
			if (score >= alpha_beta.beta)
			{
				const TranspositionTableEntry entry = { position.hash, score, incremental_updater.GetRemainingDepth(), best_move, TTFlag::BETA };
				transposition_table.Insert(entry, is_root_node);

				return alpha_beta.beta;
			}
		
			tt_flag = TTFlag::EXACT;
			alpha_beta.alpha = score;
		}
	}

	const TranspositionTableEntry entry = { position.hash, score, incremental_updater.GetRemainingDepth(), best_move, tt_flag };
	transposition_table.Insert(entry, is_root_node);

	return alpha_beta.alpha;
}

template<Color color>
std::vector<SearchResult> iterative_deepening(UciIncrementalUpdater& incremental_updater, SharedSearchContext& search_context)
{
	std::vector<SearchResult> search_results;

	for (int64_t depth = 1; depth <= search_context.GetSearchDepth(); depth++)
	{
		const Position& root_pos = incremental_updater.GetPositionStack().GetCurrentPosition();

		AlphaBeta alpha_beta = { Score::NEGATIVE_INF, Score::POSITIVE_INF };
		SearchIncrementalUpdater search_incremental_updater = incremental_updater.CreateSearchIncrementalUpdater(depth);
		IndividualSearchContext individual_search_context = IndividualSearchContext(search_context);

		auto start_timepoint = std::chrono::high_resolution_clock::now();
		const auto score = search<color, true>(alpha_beta, search_incremental_updater, individual_search_context);
		auto end_timepoint = std::chrono::high_resolution_clock::now();
		auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end_timepoint - start_timepoint).count();

		if (search_context.GetCancellationPolicy().IsAborted())
		{
			break;
		}

		SearchResult result;
		result.nodes = individual_search_context.nodes;
		result.score = score;
		result.depth = depth;

		Position curr_position = root_pos;
		for (int pv_depth = 0; pv_depth < depth; pv_depth++)
		{
			MoveList curr_position_moves = generate_moves(curr_position, incremental_updater.GetPositionStack().GetMoveList());
			const auto& curr_tt_entry = search_context.GetTranspositionTable().Get(curr_position.hash);

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
		DEBUG_IF(result.score >= Score::POSITIVE_INF || result.score <= Score::NEGATIVE_INF)
		{
			throw std::runtime_error("Score is unknown");
		}

		result.PrintUciInfo(duration);

		search_results.push_back(result);
	}

	return search_results;
}

std::vector<SearchResult> start_search(UciIncrementalUpdater& incremental_updater, SharedSearchContext& search_context)
{
	std::vector<SearchResult> results;

	std::cout << "info time limit " << search_context.GetCancellationPolicy().GetTimeLimit() << std::endl;

	const Position& root_position = incremental_updater.GetPositionStack().GetCurrentPosition();

	if (root_position.side_to_move == Color::WHITE)
	{
		results = iterative_deepening<Color::WHITE>(incremental_updater, search_context);
	}
	else
	{
		results = iterative_deepening<Color::BLACK>(incremental_updater, search_context);
	}

	std::cout << "bestmove " << results.back().pv[0].ToUciMove() << std::endl;

	return results;
}
