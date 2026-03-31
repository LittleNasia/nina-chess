#pragma once
#include "Chess/color.h"
#include "Chess/move.h"
#include "Chess/position.h"
#include "Core/Engine/rng.h"
#include "Core/Engine/utils.h"
#include "Eval/chess_bitboard_feature_iterator.h"
#include "Eval/evaluator.h"
#include "Eval/score.h"
#include "GameGeneration/book.h"
#include "MoveGen/move_list.h"
#include "Search/SearchContext/SearchCancellationPolicies/search_time_cancellation_policy.h"
#include "Search/SearchContext/shared_search_context.h"
#include "Search/position_stack.h"
#include "Search/search.h"
#include "Search/search_constraints.h"
#include "Search/transposition_table.h"
#include <atomic>
#include <chrono>
#include <cstdint>
#include <cstring>
#include <exception>
#include <fstream>
#include <functional>
#include <iomanip>
#include <iostream>
#include <memory>
#include <mutex>
#include <random>
#include <stdexcept>
#include <string>
#include <thread>
#include <vector>

enum class GameResult : uint32_t
{
	BLACK_WIN = 0,
	DRAW = 1,
	WHITE_WIN = 2,
	UNKNOWN = 255
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
	int NumThreads = 12;
	int NumGames = 100000;
	int MaxDepth = 3;
	int MaxMovetime = 0;
	int NumRandomMovesInOpening = 0;
	int MaxRandomMoves = 10;
	float RandomMoveChance = 0.15f;
	float RandomMoveChanceDecay = 0.01f;
	bool ReplyRandomWithRandom = true;
	int MaxGameLength = 200;
	bool ScoreBySearchEval = true;
	std::string OutputFile = "data.bin";
	std::string BookFile = "D:\\source\\nina-chess\\book.bin";
	int BookPly = 0; // 0 = random from available plies
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
	entry.Result = static_cast<uint32_t>(GameResult::UNKNOWN);

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
		return GameResult::DRAW;

	if (moveList.GetNumMoves() == 0)
	{
		if (moveList.MoveListMisc.Checkers)
			return (position.SideToMove == WHITE) ? GameResult::BLACK_WIN : GameResult::WHITE_WIN;
		return GameResult::DRAW;
	}

	return GameResult::UNKNOWN;
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
		return NULL_MOVE;

	std::uniform_int_distribution<uint32_t> moveDist(0, moveList.GetNumMoves() - 1);
	return moveList[moveDist(rng)];
}

forceinline MoveDecision DecideOnMove(PositionStack& positionStack, Evaluator& evaluator,
	TranspositionTable& transpositionTable, const MoveList& moveList, Xorshift64& rng,
	const GameGenerationSettings& settings, const int ply, int& randomMovesAfterOpening,
	const bool lastMoveWasRandom, float& currentRandomChance)
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

	const bool isRandom = static_cast<bool>(randomMove);

	if (isRandom)
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

	return MoveDecision{ isRandom ? randomMove : bestMove, searchScore, isRandom };
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

inline Game PlayOneGame(const GameGenerationSettings& settings, const uint64_t seed,
	PositionStack& positionStack, Evaluator& evaluator, TranspositionTable& transpositionTable,
	const Book* book = nullptr)
{
	Xorshift64 rng(seed);

	int startPly = 0;
	Position startPosition;

	if (book != nullptr)
	{
		if (settings.BookPly > 0)
		{
			startPly = settings.BookPly;
		}
		else
		{
			const auto availablePlies = book->GetAvailablePlies();
			std::uniform_int_distribution<size_t> plyDist(0, availablePlies.size() - 1);
			startPly = availablePlies[plyDist(rng)];
		}
		startPosition = book->GetRandomPosition(startPly, rng);
	}

	positionStack.Reset(startPosition);
	evaluator.Reset(positionStack);

	Game game;
	GameResult result = GameResult::UNKNOWN;
	int ply = startPly;
	int randomMovesAfterOpening = 0;
	float currentRandomChance = settings.RandomMoveChance;
	bool lastMoveWasRandom = false;

	while (ply < settings.MaxGameLength)
	{
		const Position& currentPosition = positionStack.GetCurrentPosition();
		MoveList& moveList = positionStack.GetMoveList();

		const GameResult gameOver = IsGameOver(currentPosition, positionStack, moveList);
		if (gameOver != GameResult::UNKNOWN)
		{
			const Score terminalScore = (gameOver == GameResult::DRAW) ? Score::DRAW : Score::LOSS;
			const PositionEntry entry = PackPosition(currentPosition, moveList.MoveListMisc, terminalScore, NULL_MOVE);
			game.push_back(entry);

			result = gameOver;
			break;
		}

		const MoveDecision decision = DecideOnMove(positionStack, evaluator, transpositionTable,
			moveList, rng, settings, ply, randomMovesAfterOpening, lastMoveWasRandom, currentRandomChance);

		const PositionEntry entry = PackPosition(currentPosition, moveList.MoveListMisc,
			decision.SearchScore, decision.ChosenMove);
		game.push_back(entry);

		MakeMoveAndUpdate(positionStack, evaluator, decision.ChosenMove, currentPosition.SideToMove);

		lastMoveWasRandom = decision.IsRandom;
		ply++;
	}

	if (result == GameResult::UNKNOWN)
		result = GameResult::DRAW;

	for (auto& entry : game)
		entry.Result = static_cast<uint32_t>(result);

	return game;
}

struct SharedGameGenState
{
	std::mutex Mutex;
	std::ofstream OutputFile;
	std::atomic<int> GamesCompleted{ 0 };
	std::atomic<int> TotalPositions{ 0 };
	int TotalGames;
	std::exception_ptr Exception;
	TimePoint StartTime;
};

inline void PrintProgress(const SharedGameGenState& shared, const int completed)
{
	const auto now = std::chrono::high_resolution_clock::now();
	const double elapsedSeconds = std::chrono::duration<double>(now - shared.StartTime).count();
	const int positions = shared.TotalPositions.load();
	const double gamesPerSecond = completed / elapsedSeconds;
	const double positionsPerSecond = positions / elapsedSeconds;
	const int remainingGames = shared.TotalGames - completed;
	const int etaSeconds = gamesPerSecond > 0 ? static_cast<int>(remainingGames / gamesPerSecond) : 0;
	const int etaMinutes = etaSeconds / 60;
	const int etaRemainderSeconds = etaSeconds % 60;
	const int percent = completed * 100 / shared.TotalGames;

	const int barWidth = 30;
	const int filledWidth = percent * barWidth / 100;
	std::string progressBar(static_cast<size_t>(filledWidth), '#');
	progressBar += std::string(static_cast<size_t>(barWidth - filledWidth), '-');

	std::cout << "\r[" << progressBar << "] "
		<< percent << "% | "
		<< completed << "/" << shared.TotalGames << " | "
		<< std::fixed << std::setprecision(1) << gamesPerSecond << " g/s | "
		<< std::setprecision(0) << positionsPerSecond << " pos/s | "
		<< "ETA " << etaMinutes << "m" << std::setw(2) << std::setfill('0') << etaRemainderSeconds << "s"
		<< std::setfill(' ') << "    " << std::flush;
}

inline constexpr int PROGRESS_UPDATE_INTERVAL = 10;
inline constexpr size_t WRITE_BUFFER_CAPACITY = 4096 * 1024;

forceinline bool ShouldPrintProgress(const int completed, const int totalGames)
{
	return completed % PROGRESS_UPDATE_INTERVAL == 0 || completed == totalGames;
}

forceinline bool ShouldFlushBuffer(const std::vector<PositionEntry>& buffer)
{
	return buffer.size() >= WRITE_BUFFER_CAPACITY;
}

inline void FlushBuffer(std::vector<PositionEntry>& buffer, SharedGameGenState& shared)
{
	const std::lock_guard<std::mutex> lock(shared.Mutex);
	shared.OutputFile.write(reinterpret_cast<const char*>(buffer.data()),
		static_cast<std::streamsize>(buffer.size() * sizeof(PositionEntry)));
	buffer.clear();
}

inline void GameGenThreadWorker(const GameGenerationSettings& settings, const int threadId, const int gamesForThread,
	SharedGameGenState& sharedGameState, const Book* book)
{
	try
	{
		std::vector<PositionEntry> buffer;
		buffer.reserve(WRITE_BUFFER_CAPACITY);

		PositionStack positionStack;
		Evaluator evaluator;
		TranspositionTable transpositionTable(16);

		for (int gameIndex = 0; gameIndex < gamesForThread; gameIndex++)
		{
			if (sharedGameState.Exception)
				return;

			uint64_t seed = static_cast<uint64_t>(threadId) * 1000000ULL + static_cast<uint64_t>(gameIndex);
			seed ^= std::chrono::high_resolution_clock::now().time_since_epoch().count();

			const Game game = PlayOneGame(settings, seed, positionStack, evaluator, transpositionTable, book);
			buffer.insert(buffer.end(), game.begin(), game.end());

			sharedGameState.TotalPositions.fetch_add(static_cast<int>(game.size()));
			const int completed = sharedGameState.GamesCompleted.fetch_add(1) + 1;
			if (ShouldPrintProgress(completed, sharedGameState.TotalGames))
				PrintProgress(sharedGameState, completed);

			if (ShouldFlushBuffer(buffer))
				FlushBuffer(buffer, sharedGameState);
		}

		if (!buffer.empty())
			FlushBuffer(buffer, sharedGameState);
	}
	catch (...)
	{
		const std::lock_guard<std::mutex> lock(sharedGameState.Mutex);
		if (!sharedGameState.Exception)
			sharedGameState.Exception = std::current_exception();
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
	if (!settings.BookFile.empty())
	{
		std::cout << "  Book: " << settings.BookFile << std::endl;
		std::cout << "  Book ply: " << (settings.BookPly > 0 ? std::to_string(settings.BookPly) : "random") << std::endl;
	}

	// Load book if specified
	std::unique_ptr<Book> book;
	if (!settings.BookFile.empty())
		book = std::make_unique<Book>(settings.BookFile);

	SharedGameGenState sharedGameState;
	sharedGameState.TotalGames = settings.NumGames;
	sharedGameState.StartTime = std::chrono::high_resolution_clock::now();
	sharedGameState.OutputFile.open(settings.OutputFile, std::ios::binary);
	if (!sharedGameState.OutputFile.is_open())
	{
		std::cerr << "Failed to open output file: " << settings.OutputFile << std::endl;
		return;
	}

	const int gamesPerThread = settings.NumGames / settings.NumThreads;
	const int remainder = settings.NumGames % settings.NumThreads;

	std::vector<std::thread> threads;
	for (int threadIndex = 0; threadIndex < settings.NumThreads; threadIndex++)
	{
		const int gamesForThread = gamesPerThread + (threadIndex < remainder ? 1 : 0);
		threads.emplace_back(GameGenThreadWorker, std::cref(settings), threadIndex, gamesForThread, std::ref(sharedGameState), book.get());
	}

	for (auto& thread : threads)
		thread.join();

	sharedGameState.OutputFile.close();

	if (sharedGameState.Exception)
		std::rethrow_exception(sharedGameState.Exception);

	const auto endTime = std::chrono::high_resolution_clock::now();
	const double totalSeconds = std::chrono::duration<double>(endTime - sharedGameState.StartTime).count();
	const int totalPositions = sharedGameState.TotalPositions.load();
	const double finalGamesPerSecond = settings.NumGames / totalSeconds;
	const double finalPositionsPerSecond = totalPositions / totalSeconds;
	std::cout << std::endl;
	std::cout << "Done. " << settings.NumGames << " games, " << totalPositions << " positions in "
		<< std::fixed << std::setprecision(1) << totalSeconds << "s"
		<< " (" << finalGamesPerSecond << " g/s, " << std::setprecision(0) << finalPositionsPerSecond << " pos/s)" << std::endl;
}
