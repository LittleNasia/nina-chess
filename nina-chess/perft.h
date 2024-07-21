#pragma once
#include "utils.h"

#include <chrono>
#include <fstream>
#include <iostream>
#include <limits>

#include "move_gen.h"
#include "position.h"
#include "search_stack.h"

template<Color side_to_move>
inline void perft(const Position& pos, size_t& nodes, SearchStack& search_info)
{
	if (search_info.remaining_depth <= 0)
	{
		nodes++;
		return;
	}
	const auto& move_list = generate_moves<side_to_move>(pos, search_info.GetMoveList());

	constexpr Color opposite_side = get_opposite_color<side_to_move>();

	Position& new_pos = search_info.GetNextPosition();

	for (uint32_t move_id = 0; move_id < move_list.get_num_moves(); move_id++)
	{
		position::MakeMove<side_to_move>(pos, new_pos, move_list.moves[move_id]);

		search_info.IncrementDepth();
		perft<opposite_side>(new_pos, nodes, search_info);
		search_info.DecrementDepth();
	}
}



inline size_t test_perft(const bool hideOutput = false, const size_t node_limit = std::numeric_limits<size_t>::max())
{
	std::unique_ptr<uint64_t[]> hash_history(new uint64_t[max_ply]);
	uint64_t* raw_hash_history = hash_history.get();
	std::unique_ptr<MoveList[]> move_list(new MoveList[max_depth]);
	MoveList* move_list_ptr = move_list.get();
	TranspositionTable tt(1);

	std::ifstream perft_test_suite("perftsuite.epd");
	if (!perft_test_suite.is_open())
	{
		std::cout << "could not find the test suite\n";
		return false;
	}
	std::string token;

	bool parsing_fen = true;
	int curr_depth;
	std::string fen;
	Position* curr_pos = new Position();
	size_t combined_nodes = 0;
	float total_duration = 0;

	SearchStack search_stack;

	// allocate the memory
	std::unique_ptr<MoveList[]> move_lists = std::make_unique<MoveList[]>(8 + 1);
	std::unique_ptr<Position[]> positions = std::make_unique<Position[]>(8 + 1);
	std::unique_ptr<uint64_t[]> hashes = std::make_unique<uint64_t[]>(8 + 1);
	search_stack.move_list_stack = move_lists.get();
	search_stack.position_stack = positions.get();
	search_stack.hash_stack = hashes.get();
	search_stack.tt = &tt;

	while (perft_test_suite >> token)
	{
		size_t expected_nodes;
		if (token[0] == ';')
		{
			curr_depth = token.back() - '0';
			if (parsing_fen && !hideOutput)
			{
				std::cout << "parsing fen " << fen << "\n";
			}
			parsing_fen = false;
			delete curr_pos;
			curr_pos = new Position(position::ParseFen(fen));
			
			perft_test_suite >> expected_nodes;
			parsing_fen = false;
		}
		else
		{
			if (!parsing_fen)
			{
				fen.clear();
				parsing_fen = true;
			}
			fen += token;
			fen += " ";
			continue;
		}
		size_t curr_nodes = 0;
		
		if (expected_nodes < node_limit)
		{
			combined_nodes += expected_nodes;
			if (!hideOutput)
			std::cout << "testing on depth " << curr_depth << ", expected "
				<< expected_nodes;// << ", received " <<  << "\n";

			search_stack.depth = 0;
			search_stack.remaining_depth = curr_depth;

			const auto start = std::chrono::high_resolution_clock::now();
			if (curr_pos->side_to_move == WHITE)
			{
				perft<WHITE>(*curr_pos, curr_nodes, search_stack);
			}
			else
			{
				perft<BLACK>(*curr_pos, curr_nodes, search_stack);
			}
			const auto stop = std::chrono::high_resolution_clock::now();

			const auto duration = std::chrono::duration_cast<std::chrono::microseconds>(stop - start);
			if (!hideOutput)
			std::cout << ", received " << curr_nodes << "\n";
			total_duration += float(duration.count()) / 1000000;
			if (expected_nodes != curr_nodes)
			{
				std::cout << "found error in position\n";
				std::cout << fen << "\n";
				position::PrintBoard(*curr_pos);
				return false;
			}
		}
	}
	size_t nps = (size_t)(combined_nodes / total_duration);
	if(!hideOutput)
	std::cout << "nps: " << (size_t)(combined_nodes / total_duration) << "\n";
	return nps;
}