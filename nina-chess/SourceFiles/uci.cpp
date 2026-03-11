#include "Core/Build/targets.h"
#if _UCI

#include "UCI/uci.h"
#include <atomic>
#include <iostream>
#include <sstream>
#include <thread>
#include "Eval/evaluator.h"
#include "MoveGen/move_gen.h"
#include "Chess/position.h"
#include "Search/search.h"
#include "Search/transposition_table.h"
#include <chrono>
#include <cstdint>
#include <cstdlib>
#include <fstream>
#include <memory>
#include <stdexcept>
#include <string>
#include <string_view>
#include "Chess/castling.h"
#include "Chess/chess_constants.h"
#include "Chess/color.h"
#include "Chess/move.h"
#include "MoveGen/move_list.h"
#include "Chess/piece_type.h"
#include "Search/position_stack.h"
#include "Search/search_constraints.h"
#include "Search/SearchContext/SearchCancellationPolicies/search_time_cancellation_policy.h"
#include "Search/SearchContext/shared_search_context.h"
#include "Chess/side.h"
#include "Core/Engine/utils.h"

using namespace uci;

struct UciDefaultSettings
{
	inline static constexpr uint64_t HashSize = 16;
	inline static constexpr uint64_t MinimalHashSize = 1;
	inline static constexpr uint64_t MaximalHashSize = 1 << 30;
	inline static const std::string WeightsFilename = "weights";
};

struct UciState
{
	UciState():
		HashSize(UciDefaultSettings::HashSize),
		WeightsFilename{ UciDefaultSettings::WeightsFilename },
		SearchRunning{},
		SearchThread{},
		UciEvaluator(std::make_unique<Evaluator>(WeightsFilename)),
		UciPositionStack(std::make_unique<PositionStack>()),
		UciTranspositionTable(std::make_unique<TranspositionTable>(HashSize))
	{
	}

	TranspositionTable& GetTranspositionTable() const { return *UciTranspositionTable; }

	uint64_t HashSize;
	std::string WeightsFilename;

	std::atomic_flag SearchRunning = ATOMIC_FLAG_INIT;
	std::thread SearchThread;

	std::unique_ptr<Evaluator> UciEvaluator;
	std::unique_ptr<PositionStack> UciPositionStack;
	std::unique_ptr<TranspositionTable> UciTranspositionTable;
};
static UciState currentState;

struct GoState
{
	int Depth = invalidInt;
	int Wtime = invalidInt;
	int Btime = invalidInt;
	int Winc = invalidInt;
	int Binc = invalidInt;
	int Nodes = invalidInt;
	int Movetime = invalidInt;
};

void DumpUciState(const std::string_view& filename)
{
	std::ofstream file(filename.data(), std::ios::binary);
}

void LoadUciState(const std::string_view& filename)
{
	std::ifstream file(filename.data(), std::ios::binary);
}

void Ucinewgame()
{
	currentState.UciTranspositionTable = std::make_unique<TranspositionTable>(currentState.HashSize);
	currentState.UciPositionStack->Reset(Position());
	currentState.UciEvaluator->Reset(*currentState.UciPositionStack);
}

void SearchThreadFunction(const TimePoint& search_start_timepoint, const SearchConstraints& search_constraints)
{
	currentState.SearchRunning.test_and_set();
	SharedSearchContext search_context(search_constraints, search_start_timepoint, &currentState.GetTranspositionTable());
	StartSearch<true>(*currentState.UciPositionStack, *currentState.UciEvaluator, search_context);
	currentState.SearchRunning.clear();
}

void Go(const GoState& state)
{
	const TimePoint search_start_timepoint = std::chrono::high_resolution_clock::now();
	const Position& current_position = currentState.UciPositionStack->GetCurrentPosition();
	if (currentState.SearchRunning.test())
	{
		return;
	}

	SearchConstraints constraints;
	constraints.Depth = state.Depth;
	constraints.Movetime = state.Movetime;

	int time_for_move = invalidInt;
	if (current_position.SideToMove == WHITE && state.Wtime != invalidInt)
	{
		time_for_move = state.Wtime;
		if (state.Winc != invalidInt)
		{
			time_for_move += state.Winc;
		}
	}
	else if (current_position.SideToMove == BLACK && state.Btime != invalidInt)
	{
		time_for_move = state.Btime;
		if (state.Binc != invalidInt)
		{
			time_for_move += state.Binc;
		}
	}
	constraints.Time = time_for_move;
	constraints.Nodes = state.Nodes;

	currentState.SearchThread = std::thread(SearchThreadFunction, search_start_timepoint, constraints);
	currentState.SearchThread.join();
}

GoState ParseGo(std::stringstream& input)
{
	GoState state;
	std::string token;
	while (input >> token)
	{
		if (token == "depth")
		{
			input >> state.Depth;
		}
		if (token == "wtime")
		{
			input >> state.Wtime;
		}
		if (token == "btime")
		{
			input >> state.Btime;
		}
		if (token == "winc")
		{
			input >> state.Winc;
		}
		if (token == "binc")
		{
			input >> state.Binc;
		}
		if (token == "nodes")
		{
			input >> state.Nodes;
		}
		if (token == "movetime")
		{
			input >> state.Movetime;
		}
	}
	return state;
}

struct UciMove
{
	uint32_t MoveFromIndex;
	uint32_t MoveToIndex;
	PieceType PromotionPieceType = PIECE_TYPE_NONE;
};

UciMove ParseMove(const std::string& move)
{
	const uint32_t moveFromIndex = GetSquareIndexFromChessSquareName(move.substr(0, 2).c_str());
	const uint32_t moveToIndex = GetSquareIndexFromChessSquareName(move.substr(2, 2).c_str());
	if (move.length() == 5)
	{
		switch (move[4])
		{
		case 'q':
			return { moveFromIndex, moveToIndex, QUEEN };
		case 'r':
			return { moveFromIndex, moveToIndex, ROOK };
		case 'b':
			return { moveFromIndex, moveToIndex, BISHOP };
		case 'n':
			return { moveFromIndex, moveToIndex, KNIGHT };
		}
	}
	return { moveFromIndex, moveToIndex };
}

void FindAndMakeUciMove(const Position& position, const MoveList& moveList, const UciMove& uciMove)
{
	for (uint32_t moveId = 0; moveId < moveList.GetNumMoves(); moveId++)
	{
		Move currentMove = moveList[moveId];
		if (currentMove.FromIndex() == uciMove.MoveFromIndex && currentMove.ToIndex() == uciMove.MoveToIndex)
		{
			if (uciMove.PromotionPieceType != PIECE_TYPE_NONE && currentMove.PromotionPieceType() != uciMove.PromotionPieceType)
				continue;

			if (position.SideToMove == WHITE)
			{
				currentState.UciPositionStack->MakeMove(currentMove);
				auto& currentPosMoveList = currentState.UciPositionStack->GetMoveList();
				currentState.UciEvaluator->IncrementalUpdate<WHITE>(currentState.UciPositionStack->GetCurrentPosition(), currentPosMoveList);
			}
			else
			{
				currentState.UciPositionStack->MakeMove(currentMove);
				auto& currentPosMoveList = currentState.UciPositionStack->GetMoveList();
				currentState.UciEvaluator->IncrementalUpdate<BLACK>(currentState.UciPositionStack->GetCurrentPosition(), currentPosMoveList);
			}

			return;
		}
	}
	throw std::runtime_error("Move not found");
}

void FindCastlingMoveFallback(const Position& position, const MoveList& moveList, const UciMove& uciMove)
{
	const Bitboard moveFromBitmask = 1ULL << uciMove.MoveFromIndex;
	const Bitboard moveToBitmask = 1ULL << uciMove.MoveToIndex;

	const Side& sideToMovePieces = position.SideToMove == WHITE ? position.WhitePieces : position.BlackPieces;

	if (moveFromBitmask & sideToMovePieces.King)
	{
		constexpr Bitboard kingsideKingDestinations = Castling::KingsideCastlingKingDestination<WHITE>() | Castling::KingsideCastlingKingDestination<BLACK>();
		constexpr Bitboard queensideKingDestinations = Castling::QueensideCastlingKingDestination<WHITE>() | Castling::QueensideCastlingKingDestination<BLACK>();

		const bool isKingsideCastling = moveToBitmask & (kingsideKingDestinations);
		const bool isQueensideCastling = moveToBitmask & (queensideKingDestinations);
		if (isKingsideCastling || isQueensideCastling)
		{
			for (uint32_t moveId = 0; moveId < moveList.GetNumMoves(); moveId++)
			{
				Move curr_move = moveList[moveId];
				if ((curr_move.IsKingsideCastling() && isKingsideCastling)
					||
					(curr_move.IsQueensideCastling() && isQueensideCastling))
				{
					if (position.SideToMove == WHITE)
					{
						currentState.UciPositionStack->MakeMove(curr_move);
						auto& currentPosMoveList = currentState.UciPositionStack->GetMoveList();
						currentState.UciEvaluator->IncrementalUpdate<WHITE>(currentState.UciPositionStack->GetCurrentPosition(), currentPosMoveList);
					}
					else
					{
						currentState.UciPositionStack->MakeMove(curr_move);
						auto& currentPosMoveList = currentState.UciPositionStack->GetMoveList();
						currentState.UciEvaluator->IncrementalUpdate<BLACK>(currentState.UciPositionStack->GetCurrentPosition(), currentPosMoveList);
					}

					return;
				}
			}
		}
	}
	throw std::runtime_error("Move not found");
}

void parse_moves(std::stringstream& input)
{
	std::string token;
	while (input >> token)
	{
		UciMove move = ParseMove(token);

		const auto& lastPosition =  currentState.UciPositionStack->GetCurrentPosition();

		MoveList currentPositionMoveList;
		GenerateMoves(lastPosition, currentPositionMoveList);

		try
		{
			FindAndMakeUciMove(lastPosition, currentPositionMoveList, move);
		}
		catch(...)
		{
			FindCastlingMoveFallback(lastPosition, currentPositionMoveList, move);
		}
	}
}

void Setposition(std::stringstream& input)
{
	std::string token;
	bool parsingFen = false;
	std::string currentFen;
	while (input >> token)
	{
		if (token == "moves")
		{
			parsingFen = false;
			if (!currentFen.empty())
			{
				currentState.UciPositionStack->Reset(Position::ParseFen(currentFen));
				currentState.UciEvaluator->Reset(*currentState.UciPositionStack);
			}

			parse_moves(input);
		}
		else if (token == "startpos")
		{
			currentState.UciPositionStack->Reset(Position());
			currentState.UciEvaluator->Reset(*currentState.UciPositionStack);
		}
		else if (parsingFen || token == "fen")
		{
			parsingFen = true;
			if (token == "fen")
				continue;
			currentFen += token + " ";
 		}
	}

	// we did not receive any moves and as such the parsing loop has exited
	if (parsingFen)
	{
		currentState.UciPositionStack->Reset(Position::ParseFen(currentFen));
		currentState.UciEvaluator->Reset(*currentState.UciPositionStack);
	}
}

void Setoption(std::stringstream& input)
{
	std::string token;
	input >> token;
	if (token == "name")
	{
		input >> token;
		if (token == "hash")
		{
			input >> token;
			if (token == "value")
			{
				input >> currentState.HashSize;
			}
		}
	}
}

void Isready()
{
	std::cout << "readyok" << std::endl;
}

void Quit()
{
	exit(0);
}

void UciInfo()
{
	std::cout << "id name nina-chess" << std::endl;
	std::cout << "id author LittleNasia" << std::endl;

	std::cout << std::endl;

	std::cout << "option name hash type spin default " << UciDefaultSettings::HashSize <<
		" min " << UciDefaultSettings::MinimalHashSize << " max " << UciDefaultSettings::MaximalHashSize << std::endl;

	std::cout << "uciok" << std::endl;
}

void uci::Loop()
{
	Ucinewgame();
	
	while (true)
	{
		std::string token;
		std::string input;
		std::getline(std::cin, input);

		std::stringstream inputStream(input);
		
		inputStream >> token;
		if (token == "quit" || token == "stop")
		{
			Quit();
		}
		// wait for search to finish in case it is running before executing further commands
		while (currentState.SearchRunning.test())
		{
			std::this_thread::sleep_for(std::chrono::milliseconds(1000));
		}

		if (token == "load")
		{
			std::string filename;
			inputStream >> filename;
			LoadUciState(filename);
		}
		if (token == "position")
		{
			Setposition(inputStream);
		}
		if (token == "go")
		{
			auto goState = ParseGo(inputStream);
			Go(goState);
		}
		if (token == "ucinewgame")
		{
			Ucinewgame();
		}
		if (token == "uci")
		{
			UciInfo();
		}
		
		if (token == "setoption")
		{
			Setoption(inputStream);
		}
		if (token == "print")
		{
			Position::PrintBoard( currentState.UciPositionStack->GetCurrentPosition());
			std::cout << std::endl;
		}
		if (token == "isready")
		{
			Isready();
		}
	}
}
#endif
