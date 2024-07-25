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

using namespace uci;

struct UciDefaultSettings
{
	inline static constexpr int hash_size = 16;
	inline static constexpr int min_hash_size = 1;
	inline static constexpr int max_hash_size = 1 << 30;
};

struct UciState
{
	int hash_size = UciDefaultSettings::hash_size;

	std::vector<Position> position_stack;
	std::unique_ptr<TranspositionTable> transposition_table;
	std::unique_ptr<Evaluator> evaluator;

	std::atomic_flag search_running = ATOMIC_FLAG_INIT;
	std::thread search_thread;
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

void ucinewgame()
{
	current_state.position_stack.resize(0);
	current_state.transposition_table = std::make_unique<TranspositionTable>(current_state.hash_size);
	current_state.evaluator = std::make_unique<Evaluator>();
	current_state.position_stack.push_back(Position());
}

void search_thread_function(TranspositionTable& tt, Evaluator& evaluator, const SearchConstraints& search_constraints)
{
	current_state.search_running.test_and_set();
	start_search(current_state.position_stack, tt, evaluator, search_constraints);
	current_state.search_running.clear();
}

void go(const GoState& state)
{
	const Position& current_position = current_state.position_stack.back();
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

	current_state.search_thread = std::thread(search_thread_function, std::ref(*current_state.transposition_table), std::ref(*current_state.evaluator), constraints);
	current_state.search_thread.detach();
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
	uint32_t move_from;
	uint32_t move_to;
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

void parse_moves(std::stringstream& input)
{
	std::string token;
	while (input >> token)
	{
		UciMove move = parse_move(token);

		const auto& last_pos = current_state.position_stack.back();

		MoveList curr_pos_move_list;
		generate_moves(last_pos, curr_pos_move_list);

		bool found_move = false;
		for (uint32_t move_id = 0; move_id < curr_pos_move_list.get_num_moves(); move_id++)
		{
			Move curr_move = curr_pos_move_list.moves[move_id];
			if (move.promotion_piece != PIECE_TYPE_NONE && curr_move.promotion_piece() != move.promotion_piece)
				continue;
			if (bit_index(curr_move.from()) == move.move_from && bit_index(curr_move.to()) == move.move_to)
			{
				Position new_position;
				position::MakeMove(last_pos, new_position, curr_move);
				current_state.position_stack.push_back(new_position);
				found_move = true;
				break;
			}
		}
		if (!found_move)
		{
			// some guis give castling not as king takes rook, but as king moves somewhere idk where probably to castling dest
			Bitboard move_from_bb = 1ULL << move.move_from;
			Bitboard move_to_bb = 1ULL << move.move_to;

			const Side& side_to_move_pieces = last_pos.side_to_move == WHITE ? last_pos.white_pieces : last_pos.black_pieces;

			if (move_from_bb & side_to_move_pieces.king)
			{
				constexpr Bitboard kingside_king_destinations = kingside_castling_king_dest<WHITE>() | kingside_castling_king_dest<BLACK>();
				constexpr Bitboard queenside_king_destinations = queenside_castling_king_dest<WHITE>() | queenside_castling_king_dest<BLACK>();
				constexpr Bitboard kingside_rooks = kingside_castling_rook<WHITE>() | kingside_castling_rook<BLACK>();
				constexpr Bitboard queenside_rooks = queenside_castling_rook<WHITE>() | queenside_castling_rook<BLACK>();

				if (move_to_bb & (kingside_king_destinations | queenside_king_destinations))
				{
					for (uint32_t move_id = 0; move_id < curr_pos_move_list.get_num_moves(); move_id++)
					{
						Move curr_move = curr_pos_move_list.moves[move_id];
						if (curr_move.from() & side_to_move_pieces.king &&
							curr_move.to() & kingside_rooks &&
							move_to_bb & kingside_king_destinations)
						{
							Position new_position;
							position::MakeMove(last_pos, new_position, curr_move);
							current_state.position_stack.push_back(new_position);
							break;
						}
						else if (curr_move.from() & side_to_move_pieces.king &&
							curr_move.to() & queenside_rooks &&
							move_to_bb & queenside_king_destinations)
						{
							Position new_position;
							position::MakeMove(last_pos, new_position, curr_move);
							current_state.position_stack.push_back(new_position);
							break;
						}
					}
				}
			}
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
				current_state.position_stack.clear();
				current_state.position_stack.push_back(position::ParseFen(current_fen));
				current_fen.clear();
			}

			parse_moves(input);
		}
		else if (token == "startpos")
		{
			current_state.position_stack.clear();
			current_state.position_stack.push_back(Position());
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
		std::cout << current_fen << "\n";
		current_state.position_stack.clear();
		current_state.position_stack.push_back(position::ParseFen(current_fen));
		current_fen.clear();
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
	std::string token;
	while (true)
	{
		std::string input;
		std::getline(std::cin, input);

		std::stringstream input_stream(input);
		
		input_stream >> token;
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
		if (token == "quit")
		{
			quit();
		}
		if (token == "setoption")
		{
			setoption(input_stream);
		}
		if (token == "print")
		{
			position::PrintBoard(current_state.position_stack.back());
		}
		if (token == "isready")
		{
			isready();
		}
	}
}
