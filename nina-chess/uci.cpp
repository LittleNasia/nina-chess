#include "uci.h"

#include <atomic>
#include <iostream>
#include <sstream>
#include <thread>

#include "evaluator.h"
#include "move_gen.h"
#include "position.h"
#include "search.h"
#include "transposition_table.h"
#include "uci_incremental_updater.h"

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
		UciTranspositionTable(std::make_unique<TranspositionTable>(HashSize)),
		IncrementalUpdater{ UciEvaluator.get(), UciPositionStack.get(), Position() }
	{
	}

	TranspositionTable& GetTranspositionTable() { return *UciTranspositionTable; }

	uint64_t HashSize;
	std::string WeightsFilename;

	std::atomic_flag SearchRunning = ATOMIC_FLAG_INIT;
	std::thread SearchThread;

	std::unique_ptr<Evaluator> UciEvaluator;
	std::unique_ptr<PositionStack> UciPositionStack;
	std::unique_ptr<TranspositionTable> UciTranspositionTable;

	UciIncrementalUpdater IncrementalUpdater;
};

static UciState currentState;

struct GoState
{
	int Depth = -1;
	int Wtime = -1;
	int Btime = -1;
	int Winc = -1;
	int Binc = -1;
	int Nodes = -1;
	int Movetime = -1;
};

void DumpUciState(const std::string_view& filename)
{
	std::ofstream file(filename.data(), std::ios::binary);

	currentState.UciTranspositionTable->Serialize(file);
}

void LoadUciState(const std::string_view& filename)
{
	std::ifstream file(filename.data(), std::ios::binary);

	currentState.UciTranspositionTable->Deserialize(file);
}

void Ucinewgame()
{
	currentState.UciTranspositionTable = std::make_unique<TranspositionTable>(currentState.HashSize);
}

void SearchThreadFunction(const TimePoint& search_start_timepoint, const SearchConstraints& search_constraints)
{
	currentState.SearchRunning.test_and_set();
	SharedSearchContext search_context(search_constraints, search_start_timepoint, &currentState.GetTranspositionTable());
	StartSearch<true>(currentState.IncrementalUpdater, search_context);
	currentState.SearchRunning.clear();
}

void Go(const GoState& state)
{
	const TimePoint search_start_timepoint = std::chrono::high_resolution_clock::now();
	const Position& current_position = currentState.IncrementalUpdater.GetPositionStack().GetCurrentPosition();
	if (currentState.SearchRunning.test())
	{
		return;
	}

	SearchConstraints constraints;
	constraints.Depth = state.Depth;
	constraints.Movetime = state.Movetime;

	int time_for_move = -1;
	if (current_position.SideToMove == WHITE && state.Wtime != -1)
	{
		time_for_move = state.Wtime;
		if (state.Winc != -1)
		{
			time_for_move += state.Winc;
		}
	}
	else if (current_position.SideToMove == BLACK && state.Btime != -1)
	{
		time_for_move = state.Btime;
		if (state.Binc != -1)
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
	uint32_t moveFromIndex = GetSquareIndexFromChessSquareName(move.substr(0, 2).c_str());
	uint32_t moveToIndex = GetSquareIndexFromChessSquareName(move.substr(2, 2).c_str());
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
				currentState.IncrementalUpdater.FullUpdate<WHITE>(currentMove);
			else
				currentState.IncrementalUpdater.FullUpdate<BLACK>(currentMove);

			return;
		}
	}
	throw std::runtime_error("Move not found");
}

void FindCastlingMoveFallback(const Position& position, const MoveList& moveList, const UciMove& uciMove)
{
	Bitboard moveFromBitmask = 1ULL << uciMove.MoveFromIndex;
	Bitboard moveToBitmask = 1ULL << uciMove.MoveToIndex;

	const Side& sideToMovePieces = position.SideToMove == WHITE ? position.WhitePieces : position.BlackPieces;

	if (moveFromBitmask & sideToMovePieces.King)
	{
		constexpr Bitboard kingsideKingDestinations = KingsideCastlingKingDestination<WHITE>() | KingsideCastlingKingDestination<BLACK>();
		constexpr Bitboard queensideKingDestinations = QueensideCastlingKingDestination<WHITE>() | QueensideCastlingKingDestination<BLACK>();

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
						currentState.IncrementalUpdater.FullUpdate<WHITE>(curr_move);
					else
						currentState.IncrementalUpdater.FullUpdate<BLACK>(curr_move);

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

		const auto& lastPosition = currentState.IncrementalUpdater.GetPositionStack().GetCurrentPosition();

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
				currentState.IncrementalUpdater = 
					UciIncrementalUpdater(currentState.UciEvaluator.get(), currentState.UciPositionStack.get(), position::ParseFen(currentFen));
			}

			parse_moves(input);
		}
		else if (token == "startpos")
		{
			currentState.IncrementalUpdater =
				UciIncrementalUpdater(currentState.UciEvaluator.get(), currentState.UciPositionStack.get(), Position());
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
		currentState.IncrementalUpdater =
			UciIncrementalUpdater(currentState.UciEvaluator.get(), currentState.UciPositionStack.get(), position::ParseFen(currentFen));
	}
}

void Setoption(std::stringstream& input)
{
	std::string token;
	input >> token;
	throw new std::runtime_error("Not implemented");
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
			position::PrintBoard(currentState.IncrementalUpdater.GetPositionStack().GetCurrentPosition());
			std::cout << std::endl;
		}
		if (token == "isready")
		{
			Isready();
		}
	}
}
