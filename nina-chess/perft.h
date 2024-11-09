#pragma once
#include "utils.h"

#include <chrono>
#include <fstream>
#include <iostream>
#include <limits>
#include <sstream>

#include "evaluator.h"
#include "move_gen.h"
#include "position.h"
#include "position_stack.h"
#include "search.h"
#include "search_constraints.h"
#include "shared_search_context.h"
#include "transposition_table.h"
#include "uci_incremental_updater.h"

struct PerftTestEntry
{
	std::string Fen{};
	size_t Depth{};
	size_t ExpectedNodes{};
};

inline std::vector<PerftTestEntry> ParsePerftTestSuite(const std::string& filePath)
{
	std::ifstream perftTestSuite(filePath);
	if (!perftTestSuite.is_open())
	{
		const std::string error_message = "could not find the test suite";
		std::cerr << error_message << "\n";
		throw std::runtime_error(error_message);
	}

	// step 1: get the lines in the file
	std::vector<std::string> lines;
	std::string currentLine;
	while (std::getline(perftTestSuite, currentLine))
	{
		lines.push_back(currentLine);
	}

	// step 2: split each line by ';', first entry is the FEN, all the other ones are the depths and expected nodes
	// the format is: <FEN> ;Dn <expected_nodes>; Dm <expected_nodes>; ...
	std::vector<PerftTestEntry> testEntries;
	for (const auto& line : lines)
	{
		std::istringstream iss(line);
		std::vector<std::string> tokens;

		while (iss)
		{
			std::string token;
			std::getline(iss, token, ';');
			tokens.push_back(token);
		}

		// step 3: extract the FEN and the depth from the tokens
		for (uint32_t tokenId = 1; tokenId < tokens.size(); tokenId++)
		{
			if (tokens[tokenId].empty())
			{
				continue;
			}

			PerftTestEntry entry;
			entry.Fen = tokens[0];
			
			// ignore the "D" from the depths in the tokens
			std::string depthStr = tokens[tokenId].substr(tokens[tokenId].find_first_of('D') + 1);
			std::istringstream depthStream(depthStr);
			depthStream >> entry.Depth;
			depthStream >> entry.ExpectedNodes;

			testEntries.push_back(entry);
		}
	}

	return testEntries;
}


struct PerftInfo
{
	size_t Nodes;
	size_t RemainingDepth;
};

template<Color sideToMove>
inline void Perft(PositionStack& PositionStack, PerftInfo& PerftInfo)
{
	constexpr Color oppositeSide = GetOppositeColor<sideToMove>();
	const Position& position = PositionStack.GetCurrentPosition();

	if (PerftInfo.RemainingDepth <= 0)
	{
		PerftInfo.Nodes++;
		return;
	}
	
	Position& newPosition = PositionStack.GetNextPosition();
	const auto& moveList = PositionStack.GetMoveListSkippingHashCheck<sideToMove>();
	for (uint32_t moveId = 0; moveId < moveList.GetNumMoves(); moveId++)
	{
		position::MakeMove<sideToMove>(position, newPosition, moveList[moveId]);

		PositionStack.IncrementDepth();
		PerftInfo.RemainingDepth--;
		Perft<oppositeSide>(PositionStack, PerftInfo);
		PositionStack.DecrementDepth();
		PerftInfo.RemainingDepth++;
	}
}

inline size_t TestPerft(const bool hideOutput = false, const size_t nodeLimit = std::numeric_limits<size_t>::max())
{
	PositionStack* positionStackMemory = new PositionStack;
	PositionStack& positionStack = *positionStackMemory;

	const auto& testPositions = ParsePerftTestSuite("./test/perftsuite.epd");

	size_t totalNodes = 0;
	double totalDuration = 0;

	std::string previousFen;
	for (const auto& testPosition : testPositions)
	{
		if (testPosition.ExpectedNodes > nodeLimit)
		{
			continue;
		}
		if (!hideOutput && testPosition.Fen != previousFen)
		{
			std::cout << "position: " << testPosition.Fen << "\n";
			previousFen = testPosition.Fen;
		}

		if (!hideOutput)
			std::cout << "testing on depth " << testPosition.Depth << ", expected " << testPosition.ExpectedNodes;
		
		PerftInfo perftInfo;
		perftInfo.Nodes = 0;
		perftInfo.RemainingDepth = testPosition.Depth;
		positionStack.SetCurrentPosition(position::ParseFen(testPosition.Fen));

		const auto start = std::chrono::high_resolution_clock::now();
		if (positionStack.GetCurrentPosition().SideToMove == WHITE)
		{
			Perft<WHITE>(positionStack, perftInfo);
		}
		else
		{
			Perft<BLACK>(positionStack, perftInfo);
		}
		const auto stop = std::chrono::high_resolution_clock::now();
		const auto duration = std::chrono::duration_cast<std::chrono::microseconds>(stop - start);

		totalDuration += double(duration.count()) / 1000000;
		totalNodes += perftInfo.Nodes;

		if (!hideOutput)
			std::cout << ", received " << perftInfo.Nodes << "\n";

		if (testPosition.ExpectedNodes != perftInfo.Nodes)
		{
			std::cout << "found error in position\n";
			std::cout << testPosition.Fen << "\n";
			position::PrintBoard(positionStack.GetCurrentPosition());
			return false;
		}
	}

	size_t nps = static_cast<size_t>(static_cast<double>(totalNodes) / totalDuration);

	if(!hideOutput)
		std::cout << "nps: " << nps << "\n";

	delete positionStackMemory;
	return nps;
}

inline size_t TestSearch(const bool hideOutput = false, const size_t depthLimit = std::numeric_limits<size_t>::max())
{
	try
	{
		PositionStack* positionStackMemory = new PositionStack;
		PositionStack& positionStack = *positionStackMemory;
		Evaluator* evaluatorMemory = new Evaluator();
		Evaluator& evaluator = *evaluatorMemory;

		constexpr size_t tt_size = 16;
		TranspositionTable* transpositionTable = new TranspositionTable(tt_size);

		SearchConstraints searchConstraints;

		const auto& testPositions = ParsePerftTestSuite("./test/perftsuite.epd");

		double totalDuration = 0;
		size_t totalNodes = 0;

		for (const auto& testPosition : testPositions)
		{
			if (testPosition.Depth > depthLimit)
			{
				continue;
			}

			if (!hideOutput)
			{
				std::cout << "Running search on position: " << testPosition.Fen << " " << " depth " << testPosition.Depth << "\n";
			}

			searchConstraints.Depth = int(testPosition.Depth);
			positionStack.SetCurrentPosition(position::ParseFen(testPosition.Fen));
			const Position& currentPosition = positionStack.GetCurrentPosition();

			UciIncrementalUpdater incrementalUpdater(&evaluator, &positionStack, currentPosition);
			SharedSearchContext searchContext(searchConstraints, std::chrono::high_resolution_clock().now(), transpositionTable);

			const auto start = std::chrono::high_resolution_clock::now();

			const auto& searchResults = StartSearch<false>(incrementalUpdater, searchContext);

			const auto stop = std::chrono::high_resolution_clock::now();
			const auto duration = std::chrono::duration_cast<std::chrono::microseconds>(stop - start);

			totalNodes += searchResults.back().Nodes;
			totalDuration += double(duration.count()) / 1000000;
		}

		size_t nps = static_cast<size_t>(static_cast<double>(totalNodes) / totalDuration);

		if (!hideOutput)
			std::cout << "nps: " << nps << "\n";

		delete positionStackMemory;
		delete evaluatorMemory;
		delete transpositionTable;
		return nps;
	}
	catch (const std::exception& ex)
	{
		std::cout << "Search failed: " << ex.what() << "\n";
		return 0;
	}
	
}