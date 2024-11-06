#include "position.h"

#include <iostream>
#include <sstream>

template<Color sideToMove>
forceinline constexpr char GetPieceChar(const Side& side, const Bitboard bit)
{
	ValidateColor<sideToMove>();
	if (side.Pawns & bit)
		return sideToMove == WHITE ? 'P' : 'p';
	if (side.Knights & bit)
		return sideToMove == WHITE ? 'N' : 'n';
	if (side.Bishops & bit)
		return sideToMove == WHITE ? 'B' : 'b';
	if (side.Rooks & bit)
		return sideToMove == WHITE ? 'R' : 'r';
	if (side.Queens & bit)
		return sideToMove == WHITE ? 'Q' : 'q';
	if (side.King & bit)
		return sideToMove == WHITE ? 'K' : 'k';
	return ' ';
}

forceinline constexpr char GetPieceChar(const Position& pos, const Bitboard bit)
{
	if (bit & ~pos.OccupiedBitmask)
		return ' ';
	else if (bit & pos.WhitePieces.Pieces)
		return GetPieceChar<WHITE>(pos.WhitePieces, bit);
	else
		return GetPieceChar<BLACK>(pos.BlackPieces, bit);
}

void position::PrintBoard(const Position& curr_pos)
{
	// rank names and pieces
	int rank = 8;
	for (int square = 63; square >= 0; square--)
	{
		const Bitboard curr_bit = 1ULL << square;

		if ((square + 1) % BOARD_COLUMNS == 0)
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
		std::cout << (curr_pos.OccupiedBitmask & (1ULL << square) ? "X" : " ");
		std::cout << "]";
	}

	// misc info
	std::cout << "\n";
	std::cout << "castling: BLACK: ";
	std::cout <<
		bool(curr_pos.CastlingPermissions.CastlingPermissionsBitmask & 0b1000) <<
		bool(curr_pos.CastlingPermissions.CastlingPermissionsBitmask & 0b0100) <<
		" WHITE: " <<
		bool(curr_pos.CastlingPermissions.CastlingPermissionsBitmask & 0b0010) <<
		bool(curr_pos.CastlingPermissions.CastlingPermissionsBitmask & 0b0001) <<
		"\n";
	std::cout << "side_to_move " << (curr_pos.SideToMove == WHITE ? "WHITE" : "BLACK");
}

Position position::ParseFen(const std::string_view fen)
{
	uint32_t row = 7;
	uint32_t col = 7;
	uint32_t index = 0;
	Side white_pieces;
	Side black_pieces;
	const uint32_t fifty_move_rule = 0;
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
		const size_t curr_index = TwoDimensionalIndexToOneDimensional(row, col);
		const size_t curr_piece = 1ULL << curr_index;
		// put pieces
		switch (fen[index])
		{
		case 'p':
			curr_piece_type = PAWN;
			piece_color = BLACK;
			black_pieces.Pawns ^= curr_piece;
			break;
		case 'r':
			curr_piece_type = ROOK;
			piece_color = BLACK;
			black_pieces.Rooks ^= curr_piece;
			break;
		case 'b':
			curr_piece_type = BISHOP;
			piece_color = BLACK;
			black_pieces.Bishops ^= curr_piece;
			break;
		case 'q':
			curr_piece_type = QUEEN;
			piece_color = BLACK;
			black_pieces.Queens ^= curr_piece;
			break;
		case 'k':
			curr_piece_type = KING;
			piece_color = BLACK;
			black_pieces.King ^= curr_piece;
			break;
		case 'n':
			curr_piece_type = KNIGHT;
			piece_color = BLACK;
			black_pieces.Knights ^= curr_piece;
			break;
		case 'P':
			curr_piece_type = PAWN;
			piece_color = WHITE;
			white_pieces.Pawns ^= curr_piece;
			break;
		case 'R':
			curr_piece_type = ROOK;
			piece_color = WHITE;
			white_pieces.Rooks ^= curr_piece;
			break;
		case 'B':
			curr_piece_type = BISHOP;
			piece_color = WHITE;
			white_pieces.Bishops ^= curr_piece;
			break;
		case 'Q':
			curr_piece_type = QUEEN;
			piece_color = WHITE;
			white_pieces.Queens ^= curr_piece;
			break;
		case 'K':
			curr_piece_type = KING;
			piece_color = WHITE;
			white_pieces.King ^= curr_piece;
			break;
		case 'N':
			curr_piece_type = KNIGHT;
			piece_color = WHITE;
			white_pieces.Knights ^= curr_piece;
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
	Castling castling_rights = 0;
	for (uint32_t castling_index = 0; castling_index < fen_castling_rights.length(); castling_index++)
	{
		if (fen_castling_rights[castling_index] == '-')
		{
			break;
		}
		switch (fen_castling_rights[castling_index])
		{
		case 'K':
			castling_rights.CastlingPermissionsBitmask |= KingsideCastlingPermissionsBitmask<WHITE>();
			break;
		case 'Q':
			castling_rights.CastlingPermissionsBitmask |= QueensideCastlingPermissionsBitmask<WHITE>();
			break;
		case 'k':
			castling_rights.CastlingPermissionsBitmask |= KingsideCastlingPermissionsBitmask<BLACK>();
			break;
		case 'q':
			castling_rights.CastlingPermissionsBitmask |= QueensideCastlingPermissionsBitmask<BLACK>();
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
		const size_t EP_index = GetSquareIndexFromChessSquareName(fen_en_passant_square.c_str());
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
