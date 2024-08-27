#pragma once
#include "utils.h"

#include "board_features.h"
#include "move_list.h"

class ChessBitboardFeatureIterator
{
public:
	forceinline constexpr ChessBitboardFeatureIterator(const BoardFeatures& board_features, const MoveListMisc& move_list_misc) : 
		board_features(board_features),
		move_list_misc(move_list_misc) 
	{}

	static consteval size_t NumBitboardFeatures()
	{
		return 
			static_cast<size_t>(PIECE_TYPE_NONE) * 2 + // white and black pieces
			2 +   // EP_square and castling
			6 +  // piece_moves
			4 + // pinmask, checkmask, pinners, checkers
			1; // attacked_squares
	}

	forceinline constexpr Bitboard Get(const size_t index) const
	{
		// bleh
		switch (index)
		{
		case 0:
		case 1:
		case 2:
		case 3:
		case 4:
		case 5:
		{
			const auto piece_type = static_cast<PieceType>(index);
			return board_features.white_pieces->get_piece_bb(piece_type);
		}
		case 6:
		case 7:
		case 8:
		case 9:
		case 10:
		case 11:
		{
			const auto piece_type = static_cast<PieceType>(index - 6);
			return board_features.black_pieces->get_piece_bb(piece_type);
		}
		case 12:
			return board_features.EP_square;
		case 13:
			return board_features.castling;
		case 14:
		case 15:
		case 16:
		case 17:
		case 18:
		case 19:
			return move_list_misc.piece_moves[index - 14];
		case 20:
			return move_list_misc.pinmask;
		case 21:
			return move_list_misc.checkmask;
		case 22:
			return move_list_misc.pinners;
		case 23:
			return move_list_misc.checkers;
		case 24:
			return move_list_misc.attacked_squares;
		default:
			throw std::runtime_error("Invalid index");
		}
	}

private:
	const BoardFeatures& board_features;
	const MoveListMisc& move_list_misc;
};