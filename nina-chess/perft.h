#pragma once
#include "utils.h"

#include <chrono>
#include <fstream>
#include <iostream>
#include <limits>
#include <sstream>

#include "move_gen.h"
#include "position.h"
#include "search_stack.h"

struct PerftTestEntry
{
	std::string fen;
	int depth;
	size_t expected_nodes;
};

inline std::vector<PerftTestEntry> parse_perft_test_suite(const std::string& file_path)
{
	std::ifstream perft_test_suite(file_path);
	if (!perft_test_suite.is_open())
	{
		const std::string error_message = "could not find the test suite";
		std::cerr << error_message << "\n";
		throw std::runtime_error(error_message);
	}

	// step 1: get the lines in the file
	std::vector<std::string> lines;
	std::string line;
	while (std::getline(perft_test_suite, line))
	{
		lines.push_back(line);
	}

	// step 2: split each line by ';', first entry is the FEN, all the other ones are the depths and expected nodes
	// the format is: <FEN> ;Dn <expected_nodes>; Dm <expected_nodes>; ...
	std::vector<PerftTestEntry> test_entries;
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
		for (int token_id = 1; token_id < tokens.size(); token_id++)
		{
			if (tokens[token_id].empty())
			{
				continue;
			}

			PerftTestEntry entry;
			entry.fen = tokens[0];
			
			// ignore the "D" from the depths in the tokens
			std::string depth_str = tokens[token_id].substr(tokens[token_id].find_first_of('D') + 1);
			std::istringstream depth_stream(depth_str);
			depth_stream >> entry.depth;
			depth_stream >> entry.expected_nodes;

			test_entries.push_back(entry);
		}
	}

	return test_entries;
}

template<Color side_to_move>
inline void perft(SearchStack& search_info)
{
	constexpr Color opposite_side = get_opposite_color<side_to_move>();
	const Position& pos = search_info.GetCurrentPosition();

	if (search_info.remaining_depth <= 0)
	{
		search_info.nodes++;
		return;
	}
	
	Position& new_pos = search_info.GetNextPosition();
	const auto& move_list = search_info.GetMoveListSkippingHashCheck<side_to_move>();
	for (uint32_t move_id = 0; move_id < move_list.GetNumMoves(); move_id++)
	{
		position::MakeMove<side_to_move>(pos, new_pos, move_list[move_id]);

		search_info.IncrementDepth();
		perft<opposite_side>(search_info);
		search_info.DecrementDepth();
	}
}

inline size_t test_perft(const bool hideOutput = false, const size_t node_limit = std::numeric_limits<size_t>::max())
{
	SearchStack search_stack;

	const auto& test_positions = parse_perft_test_suite("./test/perftsuite.epd");

	size_t total_nodes = 0;
	double total_duration = 0;

	std::string previous_fen;
	for (const auto& test_position : test_positions)
	{
		if (test_position.expected_nodes > node_limit)
		{
			continue;
		}
		if (!hideOutput && test_position.fen != previous_fen)
		{
			std::cout << "position: " << test_position.fen << "\n";
			previous_fen = test_position.fen;
		}

		if (!hideOutput)
			std::cout << "testing on depth " << test_position.depth << ", expected " << test_position.expected_nodes;

		search_stack.nodes = 0;
		search_stack.depth = 0;
		search_stack.remaining_depth = test_position.depth;
		search_stack.SetCurrentPosition(position::ParseFen(test_position.fen));

		const auto start = std::chrono::high_resolution_clock::now();
		if (search_stack.GetCurrentPosition().side_to_move == WHITE)
		{
			perft<WHITE>(search_stack);
		}
		else
		{
			perft<BLACK>(search_stack);
		}
		const auto stop = std::chrono::high_resolution_clock::now();
		const auto duration = std::chrono::duration_cast<std::chrono::microseconds>(stop - start);

		total_duration += double(duration.count()) / 1000000;
		total_nodes += search_stack.nodes;

		if (!hideOutput)
			std::cout << ", received " << search_stack.nodes << "\n";

		if (test_position.expected_nodes != search_stack.nodes)
		{
			std::cout << "found error in position\n";
			std::cout << test_position.fen << "\n";
			position::PrintBoard(search_stack.GetCurrentPosition());
			return false;
		}
	}

	size_t nps = (size_t)(total_nodes / total_duration);

	if(!hideOutput)
		std::cout << "nps: " << (size_t)(total_nodes / total_duration) << "\n";

	return nps;
}