#pragma once
#include "utils.h"

inline constexpr uint32_t board_rows = 8;
inline constexpr uint32_t board_cols = 8;
inline constexpr uint32_t num_board_squares = 64;

inline constexpr int square_row[num_board_squares] =
{
	0, 0, 0, 0, 0, 0, 0, 0,
	1, 1, 1, 1, 1, 1, 1, 1,
	2, 2, 2, 2, 2, 2, 2, 2,
	3, 3, 3, 3, 3, 3, 3, 3,
	4, 4, 4, 4, 4, 4, 4, 4,
	5, 5, 5, 5, 5, 5, 5, 5,
	6, 6, 6, 6, 6, 6, 6, 6,
	7, 7, 7, 7, 7, 7, 7, 7,
};

inline constexpr int square_col[num_board_squares] =
{
	0, 1, 2, 3, 4, 5, 6, 7,
	0, 1, 2, 3, 4, 5, 6, 7,
	0, 1, 2, 3, 4, 5, 6, 7,
	0, 1, 2, 3, 4, 5, 6, 7,
	0, 1, 2, 3, 4, 5, 6, 7,
	0, 1, 2, 3, 4, 5, 6, 7,
	0, 1, 2, 3, 4, 5, 6, 7,
	0, 1, 2, 3, 4, 5, 6, 7,
};

forceinline constexpr uint32_t two_d_to_one_d(const uint32_t row, const uint32_t col)
{
	DEBUG_ASSERT(row < board_rows);
	DEBUG_ASSERT(col < board_cols);
	return row * board_cols + col;
}

inline const char* square_names[num_board_squares] =
{
	"h1","g1","f1","e1","d1","c1","b1","a1",
	"h2","g2","f2","e2","d2","c2","b2","a2",
	"h3","g3","f3","e3","d3","c3","b3","a3",
	"h4","g4","f4","e4","d4","c4","b4","a4",
	"h5","g5","f5","e5","d5","c5","b5","a5",
	"h6","g6","f6","e6","d6","c6","b6","a6",
	"h7","g7","f7","e7","d7","c7","b7","a7",
	"h8","g8","f8","e8","d8","c8","b8","a8",
};


forceinline constexpr uint32_t get_col_from_file(const char file)
{
	DEBUG_ASSERT(file >= 'a' && file <= 'h');
	uint32_t col = static_cast<uint32_t>(file - 'a');
	// "h" file is actually column number 0 internally
	col = board_cols - (col + 1);
	return col;
}

forceinline constexpr uint32_t get_row_from_rank(const char rank)
{
	DEBUG_ASSERT(rank >= '1' && rank <= '8');
	return static_cast<uint32_t>(rank - '1');
}

forceinline constexpr uint32_t square_index_from_square_name(const char* square_name)
{
	DEBUG_ASSERT(square_name[0] >= 'a' && square_name[0] <= 'h');
	DEBUG_ASSERT(square_name[1] >= '1' && square_name[1] <= '8');
	uint32_t row = get_row_from_rank(square_name[1]);
	uint32_t col = get_col_from_file(square_name[0]);
	return two_d_to_one_d(row, col);
}