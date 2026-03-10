#pragma once
#include "Eval/chess_bitboard_feature_iterator.h"
#include "Chess/color.h"
#include "Eval/evaluator.h"
#include "Chess/move.h"
#include "MoveGen/move_list.h"
#include "Chess/position.h"
#include "Search/position_stack.h"
#include "rng.h"
#include "Eval/score.h"
#include "Search/search.h"
#include "Search/search_constraints.h"
#include "Search/SearchContext/SearchCancellationPolicies/search_time_cancellation_policy.h"
#include "Search/SearchContext/shared_search_context.h"
#include "Search/transposition_table.h"
#include "Core/Engine/utils.h"
#include <atomic>
#include <chrono>
#include <cstdint>
#include <cstring>
#include <exception>
#include <fstream>
#include <functional>
#include <iostream>
#include <mutex>
#include <random>
#include <stdexcept>
#include <string>
#include <thread>
#include <vector>

enum GameResult : uint32_t
{
	BLACK_WIN = 0,
	GAME_DRAW = 1,
	WHITE_WIN = 2,
	RESULT_UNKNOWN = 255
};

struct PositionEntry
{
	uint64_t Features[ChessBitboardFeatureIterator::NumBitboardFeatures()];
	Score SearchScore;
	Move BestMove;
	uint32_t Result;
	uint32_t SideToMove;
};

using Game = std::vector<PositionEntry>;

struct GameGenerationSettings
{
	int NumThreads = 1;
	int NumGames = 1000;
	int MaxDepth = 3;
	int MaxMovetime = 0;
	int NumRandomMovesInOpening = 8;
	int MaxRandomMoves = 10;
	float RandomMoveChance = 0.5f;
	float RandomMoveChanceDecay = 0.0f;
	bool ReplyRandomWithRandom = false;
	int MaxGameLength = 100;
	bool ScoreBySearchEval = true;
	std::string OutputFile = "data.bin";
};

struct MoveDecision
{
	Move ChosenMove;
	Score SearchScore;
	bool IsRandom;
};

forceinline PositionEntry PackPosition(const Position& position, const MoveListMiscellaneous& moveListMisc,
	const Score score, const Move& bestMove)
{
	PositionEntry entry{};

	std::memcpy(
		&entry.Features[ChessBitboardFeatureIterator::WHITE_PIECES_START],
		&position.WhitePieces.Pawns,
		ChessBitboardFeatureIterator::NUM_SIDE_FEATURES * sizeof(uint64_t));

	std::memcpy(
		&entry.Features[ChessBitboardFeatureIterator::BLACK_PIECES_START],
		&position.BlackPieces.Pawns,
		ChessBitboardFeatureIterator::NUM_SIDE_FEATURES * sizeof(uint64_t));

	entry.Features[ChessBitboardFeatureIterator::EN_PASSANT_INDEX] = position.EnPassantSquare;
	entry.Features[ChessBitboardFeatureIterator::CASTLING_INDEX] = position.CastlingPermissions.CurrentCastlingPermissions;

	std::memcpy(
		&entry.Features[ChessBitboardFeatureIterator::MOVELIST_MISC_START],
		&moveListMisc.PieceMoves[0],
		ChessBitboardFeatureIterator::NUM_MOVELIST_MISC_FEATURES * sizeof(uint64_t));

	entry.SearchScore = score;
	entry.BestMove = bestMove;
	entry.SideToMove = static_cast<uint32_t>(position.SideToMove);
	entry.Result = RESULT_UNKNOWN;

	return entry;
}

forceinline SearchConstraints BuildSearchConstraints(const GameGenerationSettings& settings)
{
	SearchConstraints constraints;
	constraints.Depth = settings.MaxDepth;
	if (settings.MaxMovetime > 0)
		constraints.Movetime = settings.MaxMovetime;
	return constraints;
}

forceinline GameResult IsGameOver(const Position& position, const PositionStack& positionStack, const MoveList& moveList)
{
	if (position.IsDrawn() || positionStack.IsThreefoldRepetition())
		return GAME_DRAW;

	if (moveList.GetNumMoves() == 0)
	{
		if (moveList.MoveListMisc.Checkers)
			return (position.SideToMove == WHITE) ? BLACK_WIN : WHITE_WIN;
		return GAME_DRAW;
	}

	return RESULT_UNKNOWN;
}

forceinline Move GetRandomMoveIfNeeded(const MoveList& moveList, Xorshift64& rng,
	const GameGenerationSettings& settings, const int ply, const int randomMovesAfterOpening,
	const bool lastMoveWasRandom, const float currentRandomChance)
{
	const bool isInOpening = ply < settings.NumRandomMovesInOpening;
	const bool forceRandomReply = settings.ReplyRandomWithRandom && lastMoveWasRandom;
	const bool randomBudgetRemaining = randomMovesAfterOpening < settings.MaxRandomMoves;

	std::uniform_real_distribution<float> randomChanceDist(0.0f, 1.0f);
	const bool chanceRoll = currentRandomChance > 0.0f && randomChanceDist(rng) < currentRandomChance;
	const bool useRandom = isInOpening ||
		(randomBudgetRemaining && (forceRandomReply || (!isInOpening && chanceRoll)));

	if (!useRandom)
		return Move();

	std::uniform_int_distribution<uint32_t> moveDist(0, moveList.GetNumMoves() - 1);
	return moveList[moveDist(rng)];
}

forceinline MoveDecision DecideOnMove(PositionStack& positionStack, Evaluator& evaluator,
	TranspositionTable& transpositionTable, const MoveList& moveList, Xorshift64& rng,
	const GameGenerationSettings& settings, const int ply, const int randomMovesAfterOpening,
	const bool lastMoveWasRandom, const float currentRandomChance)
{
	const SearchConstraints constraints = BuildSearchConstraints(settings);
	const TimePoint searchStart = std::chrono::high_resolution_clock::now();
	SharedSearchContext searchContext(constraints, searchStart, &transpositionTable);
	const auto results = StartSearch<false>(positionStack, evaluator, searchContext);

	if (results.empty())
		throw std::runtime_error("search returned no results");

	const auto& lastResult = results.back();
	const Move bestMove = lastResult.Pv[0];
	const Score searchScore = lastResult.Score;

	const Move randomMove = GetRandomMoveIfNeeded(moveList, rng, settings, ply,
		randomMovesAfterOpening, lastMoveWasRandom, currentRandomChance);

	const bool isRandom = !(randomMove == Move());
	return MoveDecision{ isRandom ? randomMove : bestMove, searchScore, isRandom };
}

forceinline void UpdateRandomMoveState(const GameGenerationSettings& settings, const int ply,
	int& randomMovesAfterOpening, float& currentRandomChance)
{
	const bool isInOpening = ply < settings.NumRandomMovesInOpening;
	if (!isInOpening)
	{
		randomMovesAfterOpening++;
		currentRandomChance -= settings.RandomMoveChanceDecay;
		if (currentRandomChance < 0.0f)
			currentRandomChance = 0.0f;
	}
}

forceinline void MakeMoveAndUpdate(PositionStack& positionStack, Evaluator& evaluator, const Move& move, const Color sideToMove)
{
	positionStack.MakeMove(move);
	auto& newMoveList = positionStack.GetMoveList();
	if (sideToMove == WHITE)
		evaluator.IncrementalUpdate<WHITE>(positionStack.GetCurrentPosition(), newMoveList);
	else
		evaluator.IncrementalUpdate<BLACK>(positionStack.GetCurrentPosition(), newMoveList);
}

inline Game PlayOneGame(const GameGenerationSettings& settings, const uint64_t seed)
{
	Xorshift64 rng(seed);

	PositionStack positionStack;
	positionStack.Reset(Position());
	Evaluator evaluator;
	evaluator.Reset(positionStack);
	TranspositionTable transpositionTable(16);

	Game game;
	GameResult result = RESULT_UNKNOWN;
	int ply = 0;
	int randomMovesAfterOpening = 0;
	float currentRandomChance = settings.RandomMoveChance;
	bool lastMoveWasRandom = false;

	while (ply < settings.MaxGameLength)
	{
		const Position& currentPosition = positionStack.GetCurrentPosition();
		MoveList& moveList = positionStack.GetMoveList();

		const GameResult gameOver = IsGameOver(currentPosition, positionStack, moveList);
		if (gameOver != RESULT_UNKNOWN)
		{
			result = gameOver;
			break;
		}

		const MoveDecision decision = DecideOnMove(positionStack, evaluator, transpositionTable,
			moveList, rng, settings, ply, randomMovesAfterOpening, lastMoveWasRandom, currentRandomChance);

		const PositionEntry entry = PackPosition(currentPosition, moveList.MoveListMisc,
			decision.SearchScore, decision.ChosenMove);
		game.push_back(entry);

		MakeMoveAndUpdate(positionStack, evaluator, decision.ChosenMove, currentPosition.SideToMove);

		if (decision.IsRandom)
			UpdateRandomMoveState(settings, ply, randomMovesAfterOpening, currentRandomChance);

		lastMoveWasRandom = decision.IsRandom;
		ply++;
	}

	if (result == RESULT_UNKNOWN)
		result = GAME_DRAW;

	for (auto& entry : game)
		entry.Result = result;

	return game;
}

struct SharedGameGenState
{
	std::mutex Mutex;
	std::ofstream OutputFile;
	std::atomic<int> GamesCompleted{ 0 };
	int TotalGames;
	std::exception_ptr Exception;
};

inline void GameGenThreadWorker(const GameGenerationSettings& settings, const int threadId, const int gamesForThread,
	SharedGameGenState& shared)
{
	try
	{
		std::vector<PositionEntry> buffer;
		buffer.reserve(4096);

		for (int gameIndex = 0; gameIndex < gamesForThread; gameIndex++)
		{
			if (shared.Exception)
				return;

			uint64_t seed = static_cast<uint64_t>(threadId) * 1000000ULL + static_cast<uint64_t>(gameIndex);
			seed ^= std::chrono::high_resolution_clock::now().time_since_epoch().count();

			const Game game = PlayOneGame(settings, seed);
			buffer.insert(buffer.end(), game.begin(), game.end());

			const int completed = shared.GamesCompleted.fetch_add(1) + 1;
			if (completed % 100 == 0 || completed == shared.TotalGames)
			{
				std::cout << "Games: " << completed << "/" << shared.TotalGames << std::endl;
			}

			if (buffer.size() >= 4096)
			{
				const std::lock_guard<std::mutex> lock(shared.Mutex);
				shared.OutputFile.write(reinterpret_cast<const char*>(buffer.data()),
					static_cast<std::streamsize>(buffer.size() * sizeof(PositionEntry)));
				buffer.clear();
			}
		}

		if (!buffer.empty())
		{
			const std::lock_guard<std::mutex> lock(shared.Mutex);
			shared.OutputFile.write(reinterpret_cast<const char*>(buffer.data()),
				static_cast<std::streamsize>(buffer.size() * sizeof(PositionEntry)));
		}
	}
	catch (...)
	{
		const std::lock_guard<std::mutex> lock(shared.Mutex);
		if (!shared.Exception)
			shared.Exception = std::current_exception();
	}
}

inline void RunGameGeneration(const GameGenerationSettings& settings)
{
	std::cout << "Starting game generation:" << std::endl;
	std::cout << "  Games: " << settings.NumGames << std::endl;
	std::cout << "  Threads: " << settings.NumThreads << std::endl;
	std::cout << "  Depth: " << settings.MaxDepth << std::endl;
	std::cout << "  Movetime: " << settings.MaxMovetime << std::endl;
	std::cout << "  Opening random moves: " << settings.NumRandomMovesInOpening << std::endl;
	std::cout << "  Max random moves: " << settings.MaxRandomMoves << std::endl;
	std::cout << "  Random chance: " << settings.RandomMoveChance << std::endl;
	std::cout << "  Random chance decay: " << settings.RandomMoveChanceDecay << std::endl;
	std::cout << "  Reply random with random: " << (settings.ReplyRandomWithRandom ? "yes" : "no") << std::endl;
	std::cout << "  Max game length: " << settings.MaxGameLength << std::endl;
	std::cout << "  Score by eval: " << (settings.ScoreBySearchEval ? "yes" : "no") << std::endl;
	std::cout << "  Output: " << settings.OutputFile << std::endl;
	std::cout << "  Entry size: " << sizeof(PositionEntry) << " bytes" << std::endl;

	SharedGameGenState shared;
	shared.TotalGames = settings.NumGames;
	shared.OutputFile.open(settings.OutputFile, std::ios::binary);
	if (!shared.OutputFile.is_open())
	{
		std::cerr << "Failed to open output file: " << settings.OutputFile << std::endl;
		return;
	}

	const auto startTime = std::chrono::high_resolution_clock::now();

	const int gamesPerThread = settings.NumGames / settings.NumThreads;
	const int remainder = settings.NumGames % settings.NumThreads;

	std::vector<std::thread> threads;
	for (int threadIndex = 0; threadIndex < settings.NumThreads; threadIndex++)
	{
		const int gamesForThread = gamesPerThread + (threadIndex < remainder ? 1 : 0);
		threads.emplace_back(GameGenThreadWorker, std::cref(settings), threadIndex, gamesForThread, std::ref(shared));
	}

	for (auto& thread : threads)
		thread.join();

	shared.OutputFile.close();

	if (shared.Exception)
		std::rethrow_exception(shared.Exception);

	const auto endTime = std::chrono::high_resolution_clock::now();
	const auto duration = std::chrono::duration_cast<std::chrono::seconds>(endTime - startTime).count();
	std::cout << "Done. " << settings.NumGames << " games in " << duration << "s" << std::endl;
}
