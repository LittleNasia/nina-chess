#include "individual_search_context.h"
#include "move_gen.h"
#include "uci_incremental_updater.h"

forceinline Score GetScoreFromTranspositionTable(const Position& position,
	const AlphaBeta& alphaBeta, const TranspositionTable& transpositionTable, const int64_t remainingDepth)
{
	const TranspositionTableEntry& transpositionTableEntry = transpositionTable.Get(position.Hash);

	if (transpositionTableEntry.Key == position.Hash)
	{
		if (transpositionTableEntry.Depth >= remainingDepth)
		{
			if (transpositionTableEntry.Flag == TTFlag::EXACT)
			{
				return transpositionTableEntry.Score;
			}
			else if (transpositionTableEntry.Flag == TTFlag::ALPHA &&
					 transpositionTableEntry.Score <= alphaBeta.Alpha)
			{
				return alphaBeta.Alpha;
			}
			else if (transpositionTableEntry.Flag == TTFlag::BETA &&
					 transpositionTableEntry.Score >= alphaBeta.Beta)
			{
				return alphaBeta.Beta;
			}
		}
	}

	return Score::UNKNOWN;
}

template<Color sideToMove, bool isRootNode = false>
inline Score Search(AlphaBeta alphaBeta, SearchIncrementalUpdater& incrementalUpdater, IndividualSearchContext& searchContext)
{
	auto& nodes = searchContext.Nodes;
	auto& cancellationPolicy = static_cast<SharedSearchContext&>(searchContext).GetCancellationPolicy();
	auto& transpositionTable = static_cast<SharedSearchContext&>(searchContext).GetTranspositionTable();
	auto& positionStack = incrementalUpdater.GetPositionStack();
	auto& evaluator = incrementalUpdater.GetEvaluator();

	constexpr Color oppositeSide = GetOppositeColor<sideToMove>();
	const Position& position = positionStack.GetCurrentPosition();

	// check for search aborted
	if ((searchContext.Nodes & 0xFFF) == 0xFFF)
		if (cancellationPolicy.CheckForAbort(nodes))
			return Score::UNKNOWN;
	
	Score score;
	if (!isRootNode)
	{
		// is position drawn
		if (position.IsDrawn() || positionStack.IsThreefoldRepetition())
		{
			nodes++;

			const size_t randomSeed = nodes;
			return GetDrawValueWithSmallVariance(randomSeed);
		}
	}

	// is score in TT
	if ((score = GetScoreFromTranspositionTable(position, alphaBeta, transpositionTable, incrementalUpdater.GetRemainingDepth())) != Score::UNKNOWN)
	{
		nodes++;
		ValidateScore(score);
		return score;
	}

	// is leaf node for another reason
	[[maybe_unused]] const auto& guard = incrementalUpdater.MoveGenerationUpdate<sideToMove>();
	const MoveList& moveList = positionStack.GetMoveList<sideToMove>();
	if (incrementalUpdater.GetRemainingDepth() == 0 || moveList.GetNumMoves() == 0)
	{
		score = evaluator.Evaluate<sideToMove>(moveList, incrementalUpdater.GetSearchDepth());
		ValidateScore(score);

		const TranspositionTableEntry entry = { position.Hash, score, incrementalUpdater.GetRemainingDepth(), Move(), TTFlag::EXACT };
		transpositionTable.Insert(entry, isRootNode);

		nodes++;
		return score;
	}

	// no
	TTFlag transpositionTableEntryFlag = TTFlag::ALPHA;
	Move bestMove;
	Score bestValue = Score::NEGATIVE_INF;

	for (uint32_t moveIndex = 0; moveIndex < moveList.GetNumMoves(); moveIndex++)
	{
		const Move& currentMove = moveList[moveIndex];

		incrementalUpdater.MakeMoveUpdate<sideToMove>(currentMove);
		score = -Search<oppositeSide>(alphaBeta.Invert(), incrementalUpdater, searchContext);
		incrementalUpdater.UndoMoveUpdate();

		if (cancellationPolicy.IsAborted())
		{
			return Score::UNKNOWN;
		}

		ValidateScore(score);

		if (score > bestValue)
		{
			bestValue = score;
			bestMove = currentMove;
		}

		if (score > alphaBeta.Alpha)
		{
			if (score >= alphaBeta.Beta)
			{
				const TranspositionTableEntry entry = { position.Hash, score, incrementalUpdater.GetRemainingDepth(), bestMove, TTFlag::BETA };
				transpositionTable.Insert(entry, isRootNode);

				return alphaBeta.Beta;
			}
		
			transpositionTableEntryFlag = TTFlag::EXACT;
			alphaBeta.Alpha = score;
		}
	}

	const TranspositionTableEntry entry = { position.Hash, score, incrementalUpdater.GetRemainingDepth(), bestMove, transpositionTableEntryFlag };
	transpositionTable.Insert(entry, isRootNode);

	return alphaBeta.Alpha;
}

template<Color color, bool showOutput>
std::vector<SearchResult> IterativeDeepening(UciIncrementalUpdater& incrementalUpdater, SharedSearchContext& searchContext)
{
	std::vector<SearchResult> searchResults;

	for (int64_t depth = 1; depth <= searchContext.GetSearchDepth(); depth++)
	{
		const Position& rootPos = incrementalUpdater.GetPositionStack().GetCurrentPosition();

		AlphaBeta alphaBeta = { Score::NEGATIVE_INF, Score::POSITIVE_INF };
		SearchIncrementalUpdater searchIncrementalUpdater = incrementalUpdater.CreateSearchIncrementalUpdater(depth);
		IndividualSearchContext individualSearchContext = IndividualSearchContext(searchContext);

		auto startTimepoint = std::chrono::high_resolution_clock::now();
		const auto score = Search<color, true>(alphaBeta, searchIncrementalUpdater, individualSearchContext);
		auto endTimepoint = std::chrono::high_resolution_clock::now();
		size_t duration = (size_t)std::chrono::duration_cast<std::chrono::milliseconds>(endTimepoint - startTimepoint).count();

		if (searchContext.GetCancellationPolicy().IsAborted())
		{
			break;
		}

		SearchResult result;
		result.Nodes = individualSearchContext.Nodes;
		result.Score = score;
		result.Depth = depth;

		Position currentPosition = rootPos;
		for (int pvDepth = 0; pvDepth < depth; pvDepth++)
		{
			MoveList currentPositionMoves = GenerateMoves(currentPosition, incrementalUpdater.GetPositionStack().GetMoveList());
			const auto& currentTranspositionTableEntry = searchContext.GetTranspositionTable().Get(currentPosition.Hash);

			if (currentTranspositionTableEntry.BestMove.FromBitmask() == 0 && currentTranspositionTableEntry.BestMove.ToBitmask() == 0)
				break;
			for (uint32_t moveIndex = 0; moveIndex < currentPositionMoves.GetNumMoves(); moveIndex++)
			{
				if (currentPositionMoves[moveIndex] == currentTranspositionTableEntry.BestMove)
				{
					result.Pv[pvDepth] = currentTranspositionTableEntry.BestMove;
					result.PvLength++;
					Position newPosition;
					position::MakeMove(currentPosition, newPosition, result.Pv[pvDepth]);
					currentPosition = newPosition;
					break;
				}
			}
		}

		DEBUG_ASSERT(result.PvLength != 0);
		ValidateScore(result.Score);

		if constexpr (showOutput)
			result.PrintUciInfo(duration);

		searchResults.push_back(result);
	}

	return searchResults;
}

template<bool showOutput>
std::vector<SearchResult> StartSearch(UciIncrementalUpdater& incrementalUpdater, SharedSearchContext& searchContext)
{
	std::vector<SearchResult> results;

	if constexpr (showOutput)
		std::cout << "info time limit " << searchContext.GetCancellationPolicy().GetTimeLimit() << std::endl;

	const Position& rootPosition = incrementalUpdater.GetPositionStack().GetCurrentPosition();

	if (rootPosition.SideToMove == Color::WHITE)
	{
		results = IterativeDeepening<Color::WHITE, showOutput>(incrementalUpdater, searchContext);
	}
	else
	{
		results = IterativeDeepening<Color::BLACK, showOutput>(incrementalUpdater, searchContext);
	}

	if constexpr (showOutput)
		std::cout << "bestmove " << results.back().Pv[0].ToUciMove() << std::endl;

	return results;
}
