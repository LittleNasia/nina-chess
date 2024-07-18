#include <iostream>
#include <random>

#include "move_gen.h"
#include "perft.h"
#include "position.h"
#include "targets.h"
#include "utils.h"

void do_thing(const Position& pos)
{
    while (true)
    {
        const auto& moves = generate_moves(pos);
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
                perft<WHITE>(position::MakeMove(pos, curr_move), nodes, depth);
            }
            else
            {
                perft<BLACK>(position::MakeMove(pos, curr_move), nodes, depth);
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
    if (!test_perft(false))
        return 1;
}
#endif
