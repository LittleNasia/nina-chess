#include <iostream>
#include <random>

#include "move_gen.h"
#include "perft.h"
#include "position.h"
#include "search.h"
#include "targets.h"
#include "utils.h"

void do_thing(const Position& pos)
{
    std::unique_ptr<MoveList[]> move_lists = std::make_unique<MoveList[]>(max_depth);
	TranspositionTable tt(1);
	SearchInfo search_info{ 0, 0, move_lists.get(), tt };
    while (true)
    {
        const auto& moves = generate_moves(pos, search_info.GetMoveList());
        position::PrintBoard(pos);
        std::cout << "enter the depth of perft\n";
        int depth = 1;
        std::cin >> depth;
        for (uint32_t move_id = 0; move_id < moves.get_num_moves(); move_id++)
        {
            const auto& curr_move = moves.moves[move_id];
            std::cout << "move index " << move_id << "move " << square_names[bit_index(curr_move.from())] << " " << square_names[bit_index(curr_move.to())];
            size_t nodes = 0;
            if (pos.side_to_move == WHITE)
            {
                perft<WHITE>(position::MakeMove(pos, curr_move), nodes, search_info);
            }
            else
            {
                perft<BLACK>(position::MakeMove(pos, curr_move), nodes, search_info);
            }
            std::cout << " nodes: " << nodes << "\n";
        }
        std::cout << "please enter index of move to play ";
        int move_id_to_play;
        std::cin >> move_id_to_play;
        do_thing(position::MakeMove(pos, moves.moves[move_id_to_play]));
    }
}

#if _UCI
int main()
{
	std::unique_ptr<uint64_t[]> hash_history = std::make_unique<uint64_t[]>(max_ply);
	std::unique_ptr<Evaluator> evaluator = std::make_unique<Evaluator>();


    TranspositionTable tt(128);
	const Position position = position::ParseFen("4Qnk1/p4ppp/8/7n/2P5/2B1P3/PP3q1P/6RK b - - 0 1", hash_history.get());
    Board board(position, evaluator.get());
    const size_t depth = 8;

	const auto& result = start_search(board, depth, tt);

	for (int pv_move_index = 0; pv_move_index < result.pv_length; pv_move_index++)
	{
		std::cout << "pv move: " << pv_move_index + 1 << " " << square_names[bit_index(result.pv[pv_move_index].from())] << " " << square_names[bit_index(result.pv[pv_move_index].to())] << "\n";
	}
	std::cout << "score " << result.score << "\n";
}
#endif
