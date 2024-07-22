#include "position.h"

#include <iostream>
#include <sstream>

template<Color side_to_move>
forceinline constexpr char GetPieceChar(const Side& side, const Bitboard bit)
{
	if (side.pawns & bit)
		return side_to_move == WHITE ? 'P' : 'p';
	if (side.knights & bit)
		return side_to_move == WHITE ? 'N' : 'n';
	if (side.bishops & bit)
		return side_to_move == WHITE ? 'B' : 'b';
	if (side.rooks & bit)
		return side_to_move == WHITE ? 'R' : 'r';
	if (side.queens & bit)
		return side_to_move == WHITE ? 'Q' : 'q';
	if (side.king & bit)
		return side_to_move == WHITE ? 'K' : 'k';
	return ' ';
}

forceinline constexpr char GetPieceChar(const Position& pos, const Bitboard bit)
{
	if (bit & ~pos.occupied)
		return ' ';
	else if (bit & pos.white_pieces.pieces)
		return GetPieceChar<WHITE>(pos.white_pieces, bit);
	else
		return GetPieceChar<BLACK>(pos.black_pieces, bit);
}

void position::PrintBoard(const Position& curr_pos)
{
	// rank names and pieces
	int rank = 8;
	for (int square = 63; square >= 0; square--)
	{
		const Bitboard curr_bit = 1ULL << square;

		if ((square + 1) % board_cols == 0)
		{
			std::cout << "\n";
			std::cout << rank-- << ' ';
		}

		std::cout << '[';
		std::cout << GetPieceChar(curr_pos, curr_bit);
		std::cout << ']';
	}

	// file names
	std::cout << "\n  ";
	for (int file = 0; file < 8; file++)
	{
		std::cout << ' ' << char('a' + file) << ' ';
	}

	// occupied bitboard
	std::cout << "\n\n";
	for (int square = 63; square >= 0; square--)
	{
		if ((square + 1) % 8 == 0)
		{
			std::cout << "\n";
		}

		std::cout << "[";
		std::cout << (curr_pos.occupied & (1ULL << square) ? "X" : " ");
		std::cout << "]";
	}

	// misc info
	std::cout << "\n";
	std::cout << "castling: BLACK: ";
	std::cout << bool(curr_pos.castling & 0b1000) << bool(curr_pos.castling & 0b0100) << " WHITE: " << bool(curr_pos.castling & 0b0010) << bool(curr_pos.castling & 0b0001) << "\n";
	std::cout << "side_to_move " << (curr_pos.side_to_move == WHITE ? "WHITE" : "BLACK");
}

Position position::ParseFen(const std::string_view fen)
{
	int row = 7;
	int col = 7;
	int index = 0;
	Side white_pieces;
	Side black_pieces;
	const uint32_t fifty_move_rule = 0;
	const uint32_t ply = 0;
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
		PieceType curr_piece_type = PIECE_TYPE_NONE;
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
	Color side_to_move = WHITE;
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
		white_pieces,
		black_pieces,
		EP_square, castling_rights, side_to_move, fifty_move_rule);
}
