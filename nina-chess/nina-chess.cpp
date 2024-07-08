// chess_move_gen_test.cpp : This file contains the 'main' function. Program execution begins and ends there.
//
#include "move_gen.h"
#include "perft.h"
#include "position.h"
#include "utils.h"
#include <cstdint>
#include <iostream>

// this value has been calculated by checking how many combinations of possible blockers exist
// for all the possible rook placements
inline constexpr int rook_attacks_table_size = 102400;
inline Bitboard rook_attacks_table[rook_attacks_table_size];

// this value has been calculated by checking how many combinations of possible blockers exist
// for all the possible bishop placements
inline constexpr int bishop_attacks_table_size = 5248;
inline Bitboard bishop_attacks_table[bishop_attacks_table_size];

Bitboard generate_sliding_attacks(const int square, Bitboard occupied, const point* move_offsets)
{
    Bitboard result = 0ULL;

    for (int direction = 0; direction < num_directions; direction++)
    {
        int row = square_row[square];
        int col = square_col[square];
        while (true)
        {
            int curr_row = row + move_offsets[direction].row;
            int curr_col = col + move_offsets[direction].col;
            //squares outside the Position can't really be attacked
            if ((curr_row >= 8 || curr_row < 0) || (curr_col >= 8 || curr_col < 0))
            {
                break;
            }

            int curr_index = curr_row * board_rows + curr_col;
            result |= (1ULL << curr_index);

            //we stop on first attacked piece
            if (occupied & (1ULL << curr_index))
            {
                break;
            }
            row = curr_row;
            col = curr_col;
        }
    }
    return result;
}

void initialize_pext_tables(const point* move_offsets, Bitboard* attacks_table, const Bitboard* xray_masks, Bitboard* square_offsets)
{
    size_t table_offset = 0;
    for (int square = 0; square < num_board_squares; square++)
    {
        square_offsets[square] = table_offset;
        
        uint64_t edges = ((row_bitmasks[0] | row_bitmasks[board_rows - 1]) & ~row_bitmasks[square_row[square]])
            | ((col_bitmasks[0] | col_bitmasks[board_cols - 1]) & ~col_bitmasks[square_col[square]]);

        const Bitboard xray_attacks = xray_masks[square] & ~edges;
        
        Bitboard occupied = 0ULL;
        do
        {
            size_t attacks = generate_sliding_attacks(square, occupied, move_offsets);
            size_t pext_result = pext(occupied, xray_attacks);
            // put the result for the given square and combination of attackers into the table
            attacks_table[table_offset + pext_result] = attacks;
            // get next combination of attackers using this neat trick
            occupied = (occupied - xray_attacks) & xray_attacks;
        } while (occupied);
        table_offset += (1ULL << popcnt(xray_attacks));
    }
}

void do_thing(const Position& pos)
{
    while (true)
    {
        auto moves = generate_moves(pos);
        Move all_moves[100];
        size_t num_moves;
        if (pos.side_to_move == WHITE)
        {
            num_moves = fill_moves<WHITE>(moves, all_moves);
        }
        else
        {
            num_moves = fill_moves<BLACK>(moves, all_moves);
        }
        print_board(pos);
        std::cout << "enter the depth of perft\n";
        int depth = 1;
        std::cin >> depth;
        for (int move_id = 0; move_id < num_moves; move_id++)
        {
            const auto& curr_move = all_moves[move_id];
            std::cout << "move index " << move_id << "move " << square_names[bit_index(curr_move.from)] << " " << square_names[bit_index(curr_move.to)];
            size_t nodes = 0;
            perft(make_move(pos, curr_move), nodes, depth);
            std::cout << " nodes: " << nodes << "\n";
        }
        std::cout << "please enter index of move to play ";
        int move_id_to_play;
        std::cin >> move_id_to_play;
        do_thing(make_move(pos, all_moves[move_id_to_play]));
    }
}
#include <chrono>
int main()
{
    constexpr size_t total_runs = 1;
    size_t total_nps = 0;
    for (size_t run = 0; run < total_runs; run++)
    {
        size_t perft_result = test_perft();
        total_nps += perft_result;
    }
    std::cout << total_nps / (total_runs) << std::endl;
    return 0;
    Position pos;// = parse_fen("r3k2r/Pppp1ppp/1b3nbN/nP6/BBP1P3/5q2/P2P1RPP/q2Q1K2 w kq - 0 3");
    //do_thing(pos);
    size_t nodes = 0;
    const auto start = std::chrono::high_resolution_clock::now();
    perft(pos, nodes, 7);
    const auto stop = std::chrono::high_resolution_clock::now();
    const auto duration = std::chrono::duration_cast<std::chrono::microseconds>(stop - start);

    std::cout << "perft result:" << nodes << "\n";
    std::cout << "time taken:" << duration.count() << "\n";
    std::cout << "nps: " << nodes / ((duration.count()) / 1000000 + 1) << "\n";
}
