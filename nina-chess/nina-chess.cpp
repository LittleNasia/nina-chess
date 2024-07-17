#include <iostream>

#include "move_gen.h"
#include "perft.h"
#include "position.h"
#include "targets.h"
#include "utils.h"

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
            const int curr_row = row + move_offsets[direction].row;
            const int curr_col = col + move_offsets[direction].col;
            //squares outside the Position can't really be attacked
            if ((curr_row >= 8 || curr_row < 0) || (curr_col >= 8 || curr_col < 0))
            {
                break;
            }

            const int curr_index = curr_row * board_rows + curr_col;
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
        
        const uint64_t edges = ((row_bitmasks[0] | row_bitmasks[board_rows - 1]) & ~row_bitmasks[square_row[square]])
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


void initialize_pin_between_tables(Bitboard* pin_between_table)
{
    size_t table_offset = 0;
    for (int square_from = 0; square_from < num_board_squares; square_from++)
    {
        for (int square_to = 0; square_to < num_board_squares; square_to++)
        {
			if (square_from == square_to)
			{
				pin_between_table[square_from * num_board_squares + square_to] = 0ULL;
				continue;
			}
			Bitboard result = 0ULL;
			int row = square_row[square_from];
			int col = square_col[square_from];
			
            // squares are in the same row
			if (row == square_row[square_to])
			{
				int direction = square_col[square_to] > square_col[square_from] ? 1 : -1;
				for (int curr_col = col + direction; curr_col != square_col[square_to]; curr_col += direction)
				{
					const int curr_index = row * board_rows + curr_col;
					result |= (1ULL << curr_index);
				}
                const int curr_index = row * board_rows + square_col[square_to];
                result |= (1ULL << curr_index);
			}
			// squares are in the same column
            if (col == square_col[square_to])
            {
                int direction = square_row[square_to] > square_row[square_from] ? 1 : -1;
                for (int curr_row = row + direction; curr_row != square_row[square_to]; curr_row += direction)
                {
                    const int curr_index = curr_row * board_rows + col;
                    result |= (1ULL << curr_index);
                }
                const int curr_index = square_row[square_to] * board_rows + col;
                result |= (1ULL << curr_index);
            }
			// squares are on the same diagonal
			if (abs(row - square_row[square_to]) == abs(col - square_col[square_to]))
			{
				int row_direction = square_row[square_to] > row ? 1 : -1;
				int col_direction = square_col[square_to] > col ? 1 : -1;
				int curr_row = row + row_direction;
				int curr_col = col + col_direction;
				while (curr_row != square_row[square_to])
				{
					const int curr_index = curr_row * board_rows + curr_col;
					result |= (1ULL << curr_index);
					curr_row += row_direction;
					curr_col += col_direction;
				}
				const int curr_index = square_row[square_to] * board_rows + square_col[square_to];
				result |= (1ULL << curr_index);
			}
			pin_between_table[square_from * num_board_squares + square_to] = result;
        }
    }
}

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
            perft(position::MakeMove(pos, curr_move), nodes, depth);
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
    Bitboard* pin_between_table = new Bitboard[64 * 64];
	initialize_pin_between_tables(pin_between_table);

	std::cout << "inline constexpr Bitboard pin_between_table[64][64] = {\n";
	for (int square_from = 0; square_from < 64; square_from++)
	{
		std::cout << "{";
		for (int square_to = 0; square_to < 64; square_to++)
		{
			std::cout << pin_between_table[square_from * 64 + square_to] << "ULL, ";
		}
		std::cout << "},\n";
	}
	std::cout << "};\n";
}
#endif
