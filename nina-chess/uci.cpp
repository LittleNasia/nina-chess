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
	inline static constexpr int hash_size = 16;
	inline static constexpr int min_hash_size = 1;
	inline static constexpr int max_hash_size = 1 << 30;
	inline static const std::string weights_filename = "weights";
};

struct UciState
{
	UciState():
		hash_size(UciDefaultSettings::hash_size),
		weights_filename{ UciDefaultSettings::weights_filename },
		transposition_table(std::make_unique<TranspositionTable>(hash_size)),
		search_running{},
		search_thread{},
		evaluator(std::make_unique<Evaluator>(weights_filename)),
		position_stack(std::make_unique<PositionStack>()),
		incremental_updater{ evaluator.get(), position_stack.get(), Position() }
	{
	}

	TranspositionTable& GetTranspositionTable() { return *transposition_table; }

	int hash_size;
	std::string weights_filename;

	std::atomic_flag search_running = ATOMIC_FLAG_INIT;
	std::thread search_thread;

	std::unique_ptr<PositionStack> position_stack;
	std::unique_ptr<Evaluator> evaluator;
	std::unique_ptr<TranspositionTable> transposition_table;

	UciIncrementalUpdater incremental_updater;
};

static UciState current_state;

struct GoState
{
	int depth = -1;
	int wtime = -1;
	int btime = -1;
	int winc = -1;
	int binc = -1;
	int nodes = -1;
	int movetime = -1;
};

void dump_uci_state(const std::string_view& file_name)
{
	std::ofstream file(file_name.data(), std::ios::binary);

	current_state.transposition_table->Serialize(file);
}

void load_uci_state(const std::string_view& file_name)
{
	std::ifstream file(file_name.data(), std::ios::binary);

	current_state.transposition_table->Deserialize(file);
}

void ucinewgame()
{
	current_state.transposition_table = std::make_unique<TranspositionTable>(current_state.hash_size);
}

void search_thread_function(const TimePoint& search_start_timepoint, const SearchConstraints& search_constraints)
{
	current_state.search_running.test_and_set();
	SharedSearchContext search_context(search_constraints, search_start_timepoint, &current_state.GetTranspositionTable());
	start_search(current_state.incremental_updater, search_context);
	current_state.search_running.clear();
}

void go(const GoState& state)
{
	const TimePoint search_start_timepoint = std::chrono::high_resolution_clock::now();
	const Position& current_position = current_state.incremental_updater.GetPositionStack().GetCurrentPosition();
	if (current_state.search_running.test())
	{
		return;
	}

	SearchConstraints constraints;
	constraints.depth = state.depth;
	constraints.movetime = state.movetime;

	int time_for_move = -1;
	if (current_position.side_to_move == WHITE && state.wtime != -1)
	{
		time_for_move = state.wtime;
		if (state.winc != -1)
		{
			time_for_move += state.winc;
		}
	}
	else if (current_position.side_to_move == BLACK && state.btime != -1)
	{
		time_for_move = state.btime;
		if (state.binc != -1)
		{
			time_for_move += state.binc;
		}
	}
	constraints.time = time_for_move;
	constraints.nodes = state.nodes;

	current_state.search_thread = std::thread(search_thread_function, search_start_timepoint, constraints);
	current_state.search_thread.join();
}

GoState parse_go(std::stringstream& input)
{
	GoState state;
	std::string token;
	while (input >> token)
	{
		if (token == "depth")
		{
			input >> state.depth;
		}
		if (token == "wtime")
		{
			input >> state.wtime;
		}
		if (token == "btime")
		{
			input >> state.btime;
		}
		if (token == "winc")
		{
			input >> state.winc;
		}
		if (token == "binc")
		{
			input >> state.binc;
		}
		if (token == "nodes")
		{
			input >> state.nodes;
		}
		if (token == "movetime")
		{
			input >> state.movetime;
		}
	}
	return state;
}

struct UciMove
{
	uint32_t move_from_index;
	uint32_t move_to_index;
	PieceType promotion_piece = PIECE_TYPE_NONE;
};

UciMove parse_move(const std::string& move)
{
	uint32_t move_from_index = square_index_from_square_name(move.substr(0, 2).c_str());
	uint32_t move_to_index = square_index_from_square_name(move.substr(2, 2).c_str());
	if (move.length() == 5)
	{
		switch (move[4])
		{
		case 'q':
			return { move_from_index, move_to_index, QUEEN };
		case 'r':
			return { move_from_index, move_to_index, ROOK };
		case 'b':
			return { move_from_index, move_to_index, BISHOP };
		case 'n':
			return { move_from_index, move_to_index, KNIGHT };
		}
	}
	return { move_from_index, move_to_index };
}

void find_and_make_uci_move(const Position& pos, const MoveList& move_list, const UciMove& move)
{
	for (uint32_t move_id = 0; move_id < move_list.GetNumMoves(); move_id++)
	{
		Move curr_move = move_list[move_id];
		if (curr_move.from_index() == move.move_from_index && curr_move.to_index() == move.move_to_index)
		{
			if (move.promotion_piece != PIECE_TYPE_NONE && curr_move.promotion_piece() != move.promotion_piece)
				continue;

			if (pos.side_to_move == WHITE)
				current_state.incremental_updater.FullUpdate<WHITE>(curr_move);
			else
				current_state.incremental_updater.FullUpdate<BLACK>(curr_move);

			return;
		}
	}
	throw std::runtime_error("Move not found");
}

void find_castling_move_fallback(const Position& pos, const MoveList& move_list, const UciMove& move)
{
	Bitboard move_from_bb = 1ULL << move.move_from_index;
	Bitboard move_to_bb = 1ULL << move.move_to_index;

	const Side& side_to_move_pieces = pos.side_to_move == WHITE ? pos.white_pieces : pos.black_pieces;

	if (move_from_bb & side_to_move_pieces.king)
	{
		constexpr Bitboard kingside_king_destinations = kingside_castling_king_dest<WHITE>() | kingside_castling_king_dest<BLACK>();
		constexpr Bitboard queenside_king_destinations = queenside_castling_king_dest<WHITE>() | queenside_castling_king_dest<BLACK>();

		const bool is_kingside_castling = move_to_bb & (kingside_king_destinations);
		const bool is_queenside_castling = move_to_bb & (queenside_king_destinations);
		if (is_kingside_castling || is_queenside_castling)
		{
			for (uint32_t move_id = 0; move_id < move_list.GetNumMoves(); move_id++)
			{
				Move curr_move = move_list[move_id];
				if ((curr_move.is_kingside_castling() && is_kingside_castling)
					||
					(curr_move.is_queenside_castling() && is_queenside_castling))
				{
					if (pos.side_to_move == WHITE)
						current_state.incremental_updater.FullUpdate<WHITE>(curr_move);
					else
						current_state.incremental_updater.FullUpdate<BLACK>(curr_move);

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
		UciMove move = parse_move(token);

		const auto& last_pos = current_state.incremental_updater.GetPositionStack().GetCurrentPosition();

		MoveList curr_pos_move_list;
		generate_moves(last_pos, curr_pos_move_list);

		bool found_move = false;
		try
		{
			find_and_make_uci_move(last_pos, curr_pos_move_list, move);
		}
		catch(...)
		{
			find_castling_move_fallback(last_pos, curr_pos_move_list, move);
		}
	}
}

void setposition(std::stringstream& input)
{
	std::string token;
	bool parsing_fen = false;
	std::string current_fen;
	while (input >> token)
	{
		if (token == "moves")
		{
			parsing_fen = false;
			if (!current_fen.empty())
			{
				current_state.incremental_updater = 
					UciIncrementalUpdater(current_state.evaluator.get(), current_state.position_stack.get(), position::ParseFen(current_fen));
			}

			parse_moves(input);
		}
		else if (token == "startpos")
		{
			current_state.incremental_updater =
				UciIncrementalUpdater(current_state.evaluator.get(), current_state.position_stack.get(), Position());
		}
		else if (parsing_fen || token == "fen")
		{
			parsing_fen = true;
			if (token == "fen")
				continue;
			current_fen += token + " ";
 		}
	}

	// we did not receive any moves and as such the parsing loop has exited
	if (parsing_fen)
	{
		current_state.incremental_updater =
			UciIncrementalUpdater(current_state.evaluator.get(), current_state.position_stack.get(), position::ParseFen(current_fen));
	}
}

void setoption(std::stringstream& input)
{
	

}

void isready()
{
	std::cout << "readyok" << std::endl;
}

void quit()
{
	exit(0);
}

void uci_info()
{
	std::cout << "id name nina-chess" << std::endl;
	std::cout << "id author LittleNasia" << std::endl;

	std::cout << std::endl;

	std::cout << "option name hash type spin default " << UciDefaultSettings::hash_size <<
		" min " << UciDefaultSettings::min_hash_size << " max " << UciDefaultSettings::max_hash_size << std::endl;

	std::cout << "uciok" << std::endl;
}

void uci::Loop()
{
	ucinewgame();
	
	while (true)
	{
		std::string token;
		std::string input;
		std::getline(std::cin, input);

		std::stringstream input_stream(input);
		
		input_stream >> token;
		if (token == "quit" || token == "stop")
		{
			quit();
		}
		// wait for search to finish in case it is running before executing further commands
		while (current_state.search_running.test())
		{
			std::this_thread::sleep_for(std::chrono::milliseconds(1000));
		}

		if (token == "load")
		{
			std::string file_name;
			input_stream >> file_name;
			load_uci_state(file_name);
		}
		if (token == "position")
		{
			setposition(input_stream);
		}
		if (token == "go")
		{
			auto go_state = parse_go(input_stream);
			go(go_state);
		}
		if (token == "ucinewgame")
		{
			ucinewgame();
		}
		if (token == "uci")
		{
			uci_info();
		}
		
		if (token == "setoption")
		{
			setoption(input_stream);
		}
		if (token == "print")
		{
			position::PrintBoard(current_state.incremental_updater.GetPositionStack().GetCurrentPosition());
			std::cout << std::endl;
		}
		if (token == "isready")
		{
			isready();
		}
	}
}
