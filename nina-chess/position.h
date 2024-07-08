#pragma once
#include "utils.h"
#include <iomanip>
#include <iostream>
#include <sstream>

struct Side
{
	forceinline constexpr Side() :
		pawns(0ULL), knights(0ULL), bishops(0ULL),
		rooks(0ULL), queens(0ULL), king(0ULL),
		pieces(0ULL)
	{

	}
	forceinline constexpr Side(Bitboard pawns, Bitboard knights, Bitboard bishops,
		Bitboard rooks, Bitboard queens, Bitboard king):
		pawns(pawns), knights(knights), bishops(bishops),
		rooks(rooks), queens(queens), king(king),
		pieces(pawns | knights | bishops | rooks | queens | king)
	{
		
	}
	Bitboard pawns = 0;
	Bitboard knights = 0;
	Bitboard bishops = 0;
	Bitboard rooks = 0;
	Bitboard queens = 0;
	Bitboard king = 0;

	constexpr Bitboard& get_piece_bb(PieceType piece_type)
	{
		return (&pawns)[piece_type];
	}

	forceinline constexpr void remove_pieces(const Bitboard piece)
	{
		pawns &= ~piece;
		knights &= ~piece;
		bishops &= ~piece;
		rooks &= ~piece;
		queens &= ~piece;
	}

	const Bitboard pieces = 0;
};

forceinline constexpr CastlingType remove_castling_rights(Bitboard white_rooks, Bitboard black_rooks)
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

struct Position
{
	forceinline constexpr Position() :
		white_pieces(65280ULL, 66ULL, 36ULL, 129ULL, 16ULL, 8ULL),
		black_pieces(71776119061217280ULL, 4755801206503243776ULL, 2594073385365405696ULL,
			9295429630892703744ULL, 1152921504606846976ULL, 576460752303423488ULL),
		occupied(18446462598732906495ULL), EP_square(0ULL), side_to_move(WHITE), castling(0b1111),
		ply(0)
	{
	}
	forceinline constexpr Position(
		Bitboard w_pawns, Bitboard w_knights, Bitboard w_bishops,
		Bitboard w_rooks, Bitboard w_queens, Bitboard w_king,
		Bitboard b_pawns, Bitboard b_knights, Bitboard b_bishops,
		Bitboard b_rooks, Bitboard b_queens, Bitboard b_king,
		Bitboard EP_square, CastlingType castling, Color side_to_move, 
		int ply) :
		white_pieces(w_pawns, w_knights, w_bishops, w_rooks, w_queens, w_king),
		black_pieces(b_pawns, b_knights, b_bishops, b_rooks, b_queens, b_king),
		occupied(w_pawns | w_knights | w_bishops | w_rooks | w_queens | w_king |
			b_pawns | b_knights | b_bishops | b_rooks | b_queens | b_king),
		EP_square(EP_square), castling(castling & remove_castling_rights(w_rooks, b_rooks)), side_to_move(side_to_move),
		ply(ply)
	{
	}

	forceinline constexpr Position(
		Side& white_pieces,
		Side& black_pieces,
		Bitboard EP_square, CastlingType castling, Color side_to_move,
		int ply) :
		white_pieces(white_pieces.pawns, white_pieces.knights, white_pieces.bishops,
			white_pieces.rooks, white_pieces.queens, white_pieces.king),
		black_pieces(black_pieces.pawns, black_pieces.knights, black_pieces.bishops,
			black_pieces.rooks, black_pieces.queens, black_pieces.king),
		occupied(this->white_pieces.pieces | this->black_pieces.pieces),
		EP_square(EP_square), castling(castling & remove_castling_rights(white_pieces.rooks, black_pieces.rooks)), side_to_move(side_to_move),
		ply(ply)
	{
	}

	const Side white_pieces;
	const Side black_pieces;
	const int ply;

	const Bitboard occupied;
	const Bitboard EP_square;
	const CastlingType castling;
	const Color side_to_move;

	template<Color color>
	constexpr const Side& get_side() const
	{
		if constexpr (color == WHITE)
		{
			return white_pieces;
		}
		else
		{
			return black_pieces;
		}
	}	

	forceinline constexpr CastlingType get_curr_castling() const
	{
		return castling >> (2 * side_to_move) & 0b11;
	}
};

Position make_move(const Position& pos, const Move& move);

inline void print_board(const Position& curr_pos)
{
	int rank = 8;
	for (int square = 63; square >= 0; square--)
	{
		if ((square+1) % 8 == 0)
		{
			std::cout << "\n";
			std::cout << rank-- << ' ';
		}
		std::cout << '[';
		const Bitboard curr_bit = 1ULL << square;
		if (curr_pos.white_pieces.pawns & curr_bit)
		{
			std::cout << 'P';
		}
		else if (curr_pos.white_pieces.knights & curr_bit)
		{
			std::cout << 'N';
		}
		else if (curr_pos.white_pieces.bishops & curr_bit)
		{
			std::cout << 'B';
		}
		else if (curr_pos.white_pieces.rooks & curr_bit)
		{
			std::cout << 'R';
		}
		else if (curr_pos.white_pieces.queens & curr_bit)
		{
			std::cout << 'Q';
		}
		else if (curr_pos.white_pieces.king & curr_bit)
		{
			std::cout << 'K';
		}

		else if (curr_pos.black_pieces.pawns & curr_bit)
		{
			std::cout << 'p';
		}
		else if (curr_pos.black_pieces.knights & curr_bit)
		{
			std::cout << 'n';
		}
		else if (curr_pos.black_pieces.bishops & curr_bit)
		{
			std::cout << 'b';
		}
		else if (curr_pos.black_pieces.rooks & curr_bit)
		{
			std::cout << 'r';
		}
		else if (curr_pos.black_pieces.queens & curr_bit)
		{
			std::cout << 'q';
		}
		else if (curr_pos.black_pieces.king & curr_bit)
		{
			std::cout << 'k';
		}
		else
		{
			std::cout << ' ';
		}
		std::cout << ']';
	}
	std::cout << "\n  ";
	for (int file = 0; file < 8; file++)
	{
		std::cout << ' ' << char('a' + file) << ' ';
	}
	std::cout << "\n\n";
	for (int square = 63; square >= 0; square--)
	{
		if ((square + 1) % 8 == 0)
		{
			std::cout << "\n";
		}
		std::cout << "[";
		if (curr_pos.occupied & (1ULL << square))
		{
			std::cout << "X";
		}
		else
			std::cout << " ";
		std::cout << "]";
	}
	std::cout << "\n";
	std::cout << "castling: BLACK: ";
	std::cout << bool(curr_pos.castling & 0b1000) << bool(curr_pos.castling & 0b100) << " WHITE: " << bool(curr_pos.castling & 0b10) << bool(curr_pos.castling & 0b1) << "\n";
	std::cout << "ply: " << curr_pos.ply << "\n";
	std::cout << "side_to_move " << (curr_pos.side_to_move == WHITE ? "WHITE" : "BLACK");
}

inline constexpr uint32_t get_col_from_file(const char file)
{
	int col = file - 'a';
	// "h" file is actually column number 0 internally
	col = board_cols - (col + 1);
	return col;
}

inline constexpr uint32_t get_row_from_rank(const char rank)
{
	return rank - '1';
}

inline constexpr uint32_t square_index_from_square_name(const char* square_name)
{
	uint32_t row = get_row_from_rank(square_name[1]);
	uint32_t col = get_col_from_file(square_name[0]);
	return two_d_to_one_d(row, col);
}

inline Position parse_fen(const std::string_view fen)
{
	int row = 7;
	int col = 7;
	int index = 0;
	Side white_pieces;
	Side black_pieces;
	for (index = 0; index < fen.length(); index++)
	{
		// "col 0" is actually on the right of the board (where square 0 is)
		if (fen[index] == '/')
		{
			row--;
			col = 7;
			continue;
		}
		else if (fen[index] == ' ')
		{
			// we go parse side to move and other things afterwards
			break;
		}
		PieceType curr_piece_type = PIECE_NONE;
		Color piece_color = COLOR_NONE;
		const size_t curr_index = two_d_to_one_d(row, col);
		const size_t curr_piece = 1ULL << curr_index;
		// put pieces
		switch (fen[index])
		{
		case 'p':
			curr_piece_type = PAWN;
			piece_color = BLACK;
			black_pieces.pawns ^= curr_piece;
			break;
		case 'r':
			curr_piece_type = ROOK;
			piece_color = BLACK;
			black_pieces.rooks ^= curr_piece;
			break;
		case 'b':
			curr_piece_type = BISHOP;
			piece_color = BLACK;
			black_pieces.bishops ^= curr_piece;
			break;
		case 'q':
			curr_piece_type = QUEEN;
			piece_color = BLACK;
			black_pieces.queens ^= curr_piece;
			break;
		case 'k':
			curr_piece_type = KING;
			piece_color = BLACK;
			black_pieces.king ^= curr_piece;
			break;
		case 'n':
			curr_piece_type = KNIGHT;
			piece_color = BLACK;
			black_pieces.knights ^= curr_piece;
			break;
		case 'P':
			curr_piece_type = PAWN;
			piece_color = WHITE;
			white_pieces.pawns ^= curr_piece;
			break;
		case 'R':
			curr_piece_type = ROOK;
			piece_color = WHITE;
			white_pieces.rooks ^= curr_piece;
			break;
		case 'B':
			curr_piece_type = BISHOP;
			piece_color = WHITE;
			white_pieces.bishops ^= curr_piece;
			break;
		case 'Q':
			curr_piece_type = QUEEN;
			piece_color = WHITE;
			white_pieces.queens ^= curr_piece;
			break;
		case 'K':
			curr_piece_type = KING;
			piece_color = WHITE;
			white_pieces.king ^= curr_piece;
			break;
		case 'N':
			curr_piece_type = KNIGHT;
			piece_color = WHITE;
			white_pieces.knights ^= curr_piece;
			break;
			// if it's not a piece, it's a number (new rows and end of pieces was already handled)
		default:
		{
			int num_of_empty_squares = fen[index] - '0';
			for (int empty_square = 0; empty_square < num_of_empty_squares; empty_square++)
			{
				col--;
			}
			continue;
		}
		}
		col--;
	}

	std::stringstream ss(fen.substr(index).data());

	char fen_side_to_move;
	ss >> fen_side_to_move;
	Color side_to_move;
	switch (fen_side_to_move)
	{
	case 'w':
		side_to_move = WHITE;
		break;
	case 'b':
		side_to_move = BLACK;
		break;
	default:
		std::cout << "wrong fen lmao\n";
		break;
	}
	
	std::string fen_castling_rights;
	ss >> fen_castling_rights;
	CastlingType castling_rights = 0;
	for (int castling_index = 0; castling_index < fen_castling_rights.length(); castling_index++)
	{
		if (fen_castling_rights[castling_index] == '-')
		{
			break;
		}
		switch (fen_castling_rights[castling_index])
		{
		case 'K':
			castling_rights |= kingside_castling_perms<WHITE>();
			break;
		case 'Q':
			castling_rights |= queenside_castling_perms<WHITE>();
			break;
		case 'k':
			castling_rights |= kingside_castling_perms<BLACK>();
			break;
		case 'q':
			castling_rights |= queenside_castling_perms<BLACK>();
			break;
		default:
			std::cout << "fen sucks\n";
			break;
		}
	}

	std::string fen_en_passant_square;
	ss >> fen_en_passant_square;
	Bitboard EP_square;
	if (fen_en_passant_square == "-")
	{
		EP_square = 0ULL;
	}
	else
	{
		const size_t EP_index = square_index_from_square_name(fen_en_passant_square.c_str());
		EP_square = (1ULL << EP_index);
		// sometimes en_passant square is just wrongly set in some fens
		// this stops some fens from completely ruining my sweet little program
		// my sweet little ilusia will be okay too
		//if (b.curr_pos.occupied & EP_square)
		//{
		//	EP_square = 0ULL;
		//}
	}

	return Position(
		white_pieces.pawns, white_pieces.knights, white_pieces.bishops, white_pieces.rooks, white_pieces.queens, white_pieces.king,
		black_pieces.pawns, black_pieces.knights, black_pieces.bishops, black_pieces.rooks, black_pieces.queens, black_pieces.king,
		EP_square, castling_rights, side_to_move, 0);
}