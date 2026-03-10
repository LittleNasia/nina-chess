#pragma once
#include "Search/alpha_beta.h"
#include "Search/alpha_beta.h"
#include "Chess/color.h"
#include "Chess/color.h"
#include "Eval/evaluator.h"
#include "Eval/evaluator.h"
#include "Search/SearchContext/individual_search_context.h"
#include "Search/SearchContext/individual_search_context.h"
#include "Chess/move.h"
#include "Chess/move.h"
#include "MoveGen/move_gen.h"
#include "MoveGen/move_list.h"
#include "Chess/position.h"
#include "Chess/position.h"
#include "Search/position_stack.h"
#include "Search/position_stack.h"
#include "Eval/score.h"
#include "Eval/score.h"
#include "Search/search_result.h"
#include "Search/search_result.h"
#include "Search/SearchContext/shared_search_context.h"
#include "Search/SearchContext/shared_search_context.h"
#include "Search/transposition_table.h"
#include "Search/transposition_table.h"
#include "Core/Engine/utils.h"
#include "Core/Engine/utils.h"
#include <chrono>
#include <cstdint>
#include <cstdint>
#include <iostream>
#include <memory>
#include <ostream>
#include <utility>
#include <vector>
#include <vector>

template<bool showOutput>	
forceinline std::vector<SearchResult> StartSearch(PositionStack& posStack, SharedSearchContext& searchContext);


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

class IncrementalUpdater
{
	// Move generation update is tricky, because it might not happen on every node (as we might return prematurely)
	// so we can't undo it on every single node, as we don't know whether we've hit the update or not
	// this object just ensures that if move generation update was done, it will be undone on the next return
	struct MoveGenerationUpdateGuard
	{
		forceinline constexpr MoveGenerationUpdateGuard(MoveGenerationUpdateGuard&& other)
		{
			m_Evaluator = other.m_Evaluator;
			other.m_Evaluator = nullptr;
		}
		forceinline constexpr MoveGenerationUpdateGuard(Evaluator* evaluator) : m_Evaluator(evaluator) {}
		forceinline constexpr ~MoveGenerationUpdateGuard()
		{
			if(m_Evaluator)
				m_Evaluator->UndoUpdate();
		}

	private:
		Evaluator* m_Evaluator;
	};
public:

	IncrementalUpdater(Evaluator& evaluator, PositionStack& positionStack, IndividualSearchContext& searchContext) :
		m_Evaluator(evaluator),
		m_PositionStack(positionStack),
		m_SearchContext(searchContext)
	{}

	void MakeMoveUpdate(const Move& move)
	{
		m_PositionStack.MakeMove(move);
		m_SearchContext.MakeMove();
	}

	void UndoMoveUpdate()
	{
		m_PositionStack.UndoMove();
		m_SearchContext.UndoMove();
	}

	template<Color sideToMove>
	const std::pair<MoveList&, MoveGenerationUpdateGuard> GenerateMoves()
	{
		auto& moveList = m_PositionStack.GetMoveList<sideToMove>();

		return std::pair<MoveList&, MoveGenerationUpdateGuard>( moveList, MoveGenerationUpdate<sideToMove>(moveList));
	}

	template<Color sideToMove>
	[[nodiscard]] [[maybe_unused]] forceinline constexpr MoveGenerationUpdateGuard MoveGenerationUpdate(const MoveList& moveList)
	{
		auto& currentPosition = m_PositionStack.GetCurrentPosition();
		m_Evaluator.IncrementalUpdate<sideToMove>(currentPosition, moveList);

		return MoveGenerationUpdateGuard(&m_Evaluator);
	}
private:
	Evaluator& m_Evaluator;
	PositionStack& m_PositionStack;
	IndividualSearchContext& m_SearchContext;
};

template<Color sideToMove, bool isRootNode = false>
inline Score Search(AlphaBeta alphaBeta, PositionStack& positionStack, Evaluator& evaluator, IndividualSearchContext& searchContext)
{
	IncrementalUpdater incrementalUpdater(evaluator, positionStack, searchContext);

	auto& nodes = searchContext.Nodes;
	auto& cancellationPolicy = static_cast<SharedSearchContext&>(searchContext).GetCancellationPolicy();
	auto& transpositionTable = static_cast<SharedSearchContext&>(searchContext).GetTranspositionTable();

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
	if ((score = GetScoreFromTranspositionTable(position, alphaBeta, transpositionTable, searchContext.GetRemainingDepth())) != Score::UNKNOWN)
	{
		nodes++;
		ValidateScore(score);
		return score;
	}

	// is leaf node for another reason
	[[maybe_unused]] const auto&& [moveList, guard] = incrementalUpdater.GenerateMoves<sideToMove>();
	
	if (searchContext.GetRemainingDepth() == 0 || moveList.GetNumMoves() == 0)
	{
		score = evaluator.Evaluate<sideToMove>(moveList, searchContext.GetSearchDepth());
		ValidateScore(score);

		const TranspositionTableEntry entry = { position.Hash, score, searchContext.GetRemainingDepth(), Move(), TTFlag::EXACT };
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

		incrementalUpdater.MakeMoveUpdate(currentMove);
		score = -Search<oppositeSide>(alphaBeta.Invert(), positionStack, evaluator, searchContext);
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
				const TranspositionTableEntry entry = { position.Hash, score, searchContext.GetRemainingDepth(), bestMove, TTFlag::BETA };
				transpositionTable.Insert(entry, isRootNode);

				return alphaBeta.Beta;
			}

			transpositionTableEntryFlag = TTFlag::EXACT;
			alphaBeta.Alpha = score;
		}
	}

	const TranspositionTableEntry entry = { position.Hash, score, searchContext.GetRemainingDepth(), bestMove, transpositionTableEntryFlag };
	transpositionTable.Insert(entry, isRootNode);

	return alphaBeta.Alpha;
}

template<Color color, bool showOutput>
forceinline std::vector<SearchResult> IterativeDeepening(PositionStack& positionStack, Evaluator& evaluator, SharedSearchContext& searchContext)
{
	std::vector<SearchResult> searchResults;

	for (int64_t depth = 1; depth <= searchContext.GetSearchDepth(); depth++)
	{
		const Position& rootPos = positionStack.GetCurrentPosition();

		AlphaBeta alphaBeta = { Score::NEGATIVE_INF, Score::POSITIVE_INF }; 
		IndividualSearchContext individualSearchContext = IndividualSearchContext(searchContext, depth);

		auto startTimepoint = std::chrono::high_resolution_clock::now();
		const auto score = Search<color, true>(alphaBeta, positionStack, evaluator, individualSearchContext);
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
		std::unique_ptr<MoveList> rootMoveList = std::make_unique<MoveList>();
		for (int pvDepth = 0; pvDepth < depth; pvDepth++)
		{
			MoveList currentPositionMoves = GenerateMoves(currentPosition, *rootMoveList);
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
					Position::MakeMove(currentPosition, newPosition, result.Pv[pvDepth]);
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
forceinline std::vector<SearchResult> StartSearch(PositionStack& positionStack, Evaluator& evaluator, SharedSearchContext& searchContext)
{
	std::vector<SearchResult> results;

	if constexpr (showOutput)
		std::cout << "info time limit " << searchContext.GetCancellationPolicy().GetTimeLimit() << std::endl;

	const Position& rootPosition = positionStack.GetCurrentPosition();

	if (rootPosition.SideToMove == Color::WHITE)
	{
		results = IterativeDeepening<Color::WHITE, showOutput>(positionStack, evaluator, searchContext);
	}
	else
	{
		results = IterativeDeepening<Color::BLACK, showOutput>(positionStack, evaluator, searchContext);
	}

	if constexpr (showOutput)
		std::cout << "bestmove " << results.back().Pv[0].ToUciMove() << std::endl;

	return results;
}
