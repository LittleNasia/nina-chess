#pragma once
#include <cstdint>
#include <intrin.h>
#include <stdexcept>

using Bitboard = std::uint64_t;

inline constexpr size_t board_rows = 8;
inline constexpr size_t board_cols = 8;
inline constexpr size_t num_board_squares = 64;
inline constexpr size_t max_ply = 512;

#ifdef _DEBUG
inline constexpr bool is_debug = true;
#else
inline constexpr bool is_debug = false;
#endif

#define DEBUG_IF(x) if constexpr(is_debug) if (x)

// TODO determine which functions should be forceinlined and which shouldnt
// forceinlining everything doesn't seem to give performance benefits anymore
// maybe not anymore, forceinlining everything seems to be the way to go now for some reason ? ? ?
// there are functions that probably still shouldn't be inlined but forceinline as a default seems fine now
#define forceinline __forceinline

enum Color: uint8_t
{
	WHITE,
	BLACK,
	COLOR_NONE
};

enum Piece : uint32_t
{
	WHITE_PAWN,
	WHITE_KNIGHT,
	WHITE_BISHOP,
	WHITE_ROOK,
	WHITE_QUEEN,
	WHITE_KING,

	BLACK_PAWN,
	BLACK_KNIGHT,
	BLACK_BISHOP, 
	BLACK_ROOK,
	BLACK_QUEEN, 
	BLACK_KING,

	PIECE_NONE
};

enum PieceType : uint32_t
{
	PAWN,
	KNIGHT,
	BISHOP,
	ROOK,
	QUEEN,
	KING,
	PIECE_TYPE_NONE
};

forceinline constexpr PieceType operator++(PieceType& piece, int)
{
	return piece = static_cast<PieceType>(piece + 1);
}


enum class Score : int32_t
{
	NEGATIVE_INF = -1000000,
	LOSS = -10000,
	DRAW = 0,
	WIN = 10000,
	POSITIVE_INF = 1000000
};


using CastlingType = uint32_t;

inline constexpr Bitboard empty_bitboard = 0ULL;
inline constexpr Bitboard full_bitboard = 0xffffffffffffffffULL;

constexpr char piece_names[] =
{
	'P','N','B','R','Q','K',
	'p','n','b','r','q','k',
};

forceinline constexpr uint32_t two_d_to_one_d(const uint32_t row, const uint32_t col)
{
	return row * board_cols + col;
}

forceinline size_t pext(const Bitboard b, const Bitboard mask)
{
	return _pext_u64(b, mask);
}

#pragma warning( push )
#pragma warning (disable:4244)
forceinline uint32_t popcnt(const Bitboard bb)
{
	return __popcnt64(bb);
}
#pragma warning( pop ) 

forceinline Bitboard pop_bit(Bitboard& bb)
{
	uint64_t lsb = _blsi_u64(bb);
	bb ^= lsb;
	return lsb;
}

#pragma warning( push )
#pragma warning ( disable:4244 )
forceinline uint32_t bit_index(const Bitboard bb)
{
	return _tzcnt_u64(bb);
}
#pragma warning( pop ) 

inline constexpr Bitboard row_bitmasks[board_rows] =
{
	255ULL,
	65280ULL,
	16711680ULL,
	4278190080ULL,
	1095216660480ULL,
	280375465082880ULL,
	71776119061217280ULL,
	18374686479671623680ULL,
};


inline constexpr Bitboard col_bitmasks[board_cols] =
{
	72340172838076673ULL,
	144680345676153346ULL,
	289360691352306692ULL,
	578721382704613384ULL,
	1157442765409226768ULL,
	2314885530818453536ULL,
	4629771061636907072ULL,
	9259542123273814144ULL,
};

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

// this lookup table returns a bitmask of all squares between the two column indices in a row
// for example, given pieces in the same row, one in column 1, another one in column 4
// [1][4] will return bitmask of all squares between these two
// in this case, it will be bits on index 2 and 3 set
inline constexpr Bitboard bitmask_between_coords[board_cols][board_cols] =
{
	{0, 0, 2, 6, 14, 30, 62, 126, },
	{0, 0, 0, 4, 12, 28, 60, 124, },
	{2, 0, 0, 0, 8, 24, 56, 120, },
	{6, 4, 0, 0, 0, 16, 48, 112, },
	{14, 12, 8, 0, 0, 0, 32, 96, },
	{30, 28, 24, 16, 0, 0, 0, 64, },
	{62, 60, 56, 48, 32, 0, 0, 0, },
	{126, 124, 120, 112, 96, 64, 0, 0, },
};

struct point
{
	int row;
	int col;
};

inline constexpr int num_directions = 4;
inline constexpr point rook_moves[num_directions] =
{
	{-1, 0},
	{ 1, 0},
	{ 0, 1},
	{ 0,-1},
};

inline constexpr point bishop_moves[num_directions] =
{
	{ 1, 1},
	{ 1,-1},
	{-1, 1},
	{-1,-1},
};

inline constexpr Bitboard pawns_can_attack_left = 0x7f7f7f7f7f7f7f7f;
inline constexpr Bitboard pawns_can_attack_right = 0xfefefefefefefefe;

template<Color color>
forceinline constexpr Color get_opposite_color()
{
	if constexpr (color == WHITE)
	{
		return BLACK;
	}
	if constexpr (color == BLACK)
	{
		return WHITE;
	}
}

forceinline constexpr Color get_opposite_color(const Color color)
{
	if (color == WHITE)
	{
		return BLACK;
	}
	else if (color == BLACK)
	{
		return WHITE;
	}
}

template<PieceType piece, Color color>
forceinline constexpr Piece get_piece_from_type()
{
	if constexpr (piece == KING && color == WHITE)
	{
		return WHITE_KING;
	}
	else if constexpr (piece == KNIGHT && color == WHITE)
	{
		return WHITE_KNIGHT;
	}
	else if constexpr (piece == BISHOP && color == WHITE)
	{
		return WHITE_BISHOP;
	}
	else if constexpr (piece == PAWN && color == WHITE)
	{
		return WHITE_PAWN;
	}
	else if constexpr (piece == ROOK && color == WHITE)
	{
		return WHITE_ROOK;
	}
	else if constexpr (piece == QUEEN && color == WHITE)
	{
		return WHITE_QUEEN;
	}
	else if constexpr (piece == KING && color == BLACK)
	{
		return BLACK_KING;
	}
	else if constexpr (piece == PAWN && color == BLACK)
	{
		return BLACK_PAWN;
	}
	else if constexpr (piece == KNIGHT && color == BLACK)
	{
		return BLACK_KNIGHT;
	}
	else if constexpr (piece == BISHOP && color == BLACK)
	{
		return BLACK_BISHOP;
	}
	else if constexpr (piece == ROOK && color == BLACK)
	{
		return BLACK_ROOK;
	}
	else
	{
		return BLACK_QUEEN;
	}
}

// gets the pawns that can capture EP on a given square
inline constexpr Bitboard EP_candidates_lookup[64] =
{
	0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,
	0x2000000,0x5000000, 0xa000000, 0x14000000,0x28000000,0x50000000,0xa0000000, 0x40000000,
	0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,
	0x200000000,0x500000000, 0xa00000000, 0x1400000000, 0x2800000000, 0x5000000000, 0xa000000000, 0x4000000000,
	0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,
};

// gets the pawns that will be captured by EP on a given square
inline constexpr Bitboard EP_victims_lookup[64] =
{
	0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,
	0x1000000, 0x2000000, 0x4000000, 0x8000000, 0x10000000, 0x20000000, 0x40000000, 0x80000000,
	0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,
	0x100000000, 0x200000000, 0x400000000, 0x800000000, 0x1000000000, 0x2000000000, 0x4000000000, 0x8000000000,
	0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,
};

template<Color color, CastlingType castling>
forceinline constexpr CastlingType get_castling()
{
	if constexpr (color == WHITE)
	{
		return castling & 0b11;
	}
	else
	{
		return (castling & 0b1100) >> 2;
	}
}

// bitmask of the path of king during castling
template<Color color>
forceinline constexpr Bitboard kingside_castling_castling_king_path()
{
	if constexpr (color == WHITE)
	{
		return 0xe;
	}
	else
	{
		return 0xe00000000000000;
	}
}

// bitmask of the path of king during castling
template<Color color>
forceinline constexpr Bitboard kingside_castling_castling_rook_path()
{
	if constexpr (color == WHITE)
	{
		return 0x6;
	}
	else
	{
		return 0x600000000000000;
	}
}

// bitmask of the path of king during castling
template<Color color>
forceinline constexpr Bitboard queenside_castling_king_path()
{
	if constexpr (color == WHITE)
	{
		return 0x38;
	}
	else
	{
		return 0x3800000000000000;
	}
}

// bitmask of the path of king during castling
template<Color color>
forceinline constexpr Bitboard queenside_castling_rook_path()
{
	if constexpr (color == WHITE)
	{
		return 0x70;
	}
	else
	{
		return 0x7000000000000000;
	}
}

template<Color color>
forceinline constexpr Bitboard kingside_castling_rook()
{
	if constexpr (color == WHITE)
	{
		return 0x1;
	}
	else
	{
		return 0x100000000000000;
	}
}

template<Color color>
forceinline constexpr Bitboard queenside_castling_rook()
{
	if constexpr (color == WHITE)
	{
		return 0x80;
	}
	else
	{
		return 0x8000000000000000;
	}
}

template<Color color>
forceinline constexpr Bitboard kingside_castling_king_dest()
{
	if constexpr (color == WHITE)
	{
		return 0x2;
	}
	else
	{
		return 0x200000000000000;
	}
}

template<Color color>
forceinline constexpr Bitboard queenside_castling_king_dest()
{
	if constexpr (color == WHITE)
	{
		return 0x20;
	}
	else
	{
		return 0x2000000000000000;
	}
}

template<Color color>
forceinline constexpr Bitboard queenside_castling_rook_dest()
{
	if constexpr (color == WHITE)
	{
		return 0x10;
	}
	else
	{
		return 0x1000000000000000;
	}
}

template<Color color>
forceinline constexpr Bitboard kingside_castling_rook_dest()
{
	if constexpr (color == WHITE)
	{
		return 0x4;
	}
	else
	{
		return 0x400000000000000;
	}
}

forceinline constexpr CastlingType update_castling_rights(const Bitboard white_rooks, const Bitboard black_rooks)
{
	CastlingType castling_perms = 0b1111;
	if (!(kingside_castling_rook<WHITE>() & white_rooks))
	{
		castling_perms &= ~0b0001;
	}
	if (!(queenside_castling_rook<WHITE>() & white_rooks))
	{
		castling_perms &= ~0b0010;
	}
	if (!(kingside_castling_rook<BLACK>() & black_rooks))
	{
		castling_perms &= ~0b0100;
	}
	if (!(queenside_castling_rook<BLACK>() & black_rooks))
	{
		castling_perms &= ~0b1000;
	}
	return castling_perms;
}

template<Color color>
forceinline constexpr Bitboard castling_perms()
{
	if constexpr (color == WHITE)
	{
		return 0b11;
	}
	else
	{
		return 0b1100;
	}
}

template<Color color>
forceinline constexpr Bitboard kingside_castling_perms()
{
	if constexpr (color == WHITE)
	{
		return 0b1;
	}
	else
	{
		return 0b100;
	}
}

template<Color color>
forceinline constexpr Bitboard queenside_castling_perms()
{
	if constexpr (color == WHITE)
	{
		return 0b10;
	}
	else
	{
		return 0b1000;
	}
}

template<Color color>
forceinline constexpr Bitboard king_startpos()
{
	if constexpr (color == WHITE)
	{
		return 0x8;
	}
	else
	{
		return 0x800000000000000;
	}
}

template<Color color>
forceinline constexpr Bitboard promotion_rank()
{
	if constexpr (color == WHITE)
	{
		return 0xff00000000000000;
	}
	else
	{
		return 0xff;
	}
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

inline constexpr PieceType promotion_pieces[4] =
{
	KNIGHT,
	BISHOP,
	ROOK,
	QUEEN
};

forceinline constexpr uint32_t get_col_from_file(const char file)
{
	int col = file - 'a';
	// "h" file is actually column number 0 internally
	col = board_cols - (col + 1);
	return col;
}

forceinline constexpr uint32_t get_row_from_rank(const char rank)
{
	return rank - '1';
}

forceinline constexpr uint32_t square_index_from_square_name(const char* square_name)
{
	uint32_t row = get_row_from_rank(square_name[1]);
	uint32_t col = get_col_from_file(square_name[0]);
	return two_d_to_one_d(row, col);
}