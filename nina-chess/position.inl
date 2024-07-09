#include "position.h"

forceinline constexpr Position::Position() :
	white_pieces(65280ULL, 66ULL, 36ULL, 129ULL, 16ULL, 8ULL),
	black_pieces(71776119061217280ULL, 4755801206503243776ULL, 2594073385365405696ULL,
		9295429630892703744ULL, 1152921504606846976ULL, 576460752303423488ULL),
	occupied(18446462598732906495ULL), EP_square(0ULL), side_to_move(WHITE), castling(0b1111),
	ply(0)
{
}

forceinline constexpr Position::Position(Bitboard w_pawns, Bitboard w_knights, Bitboard w_bishops, Bitboard w_rooks, Bitboard w_queens, Bitboard w_king,
									Bitboard b_pawns, Bitboard b_knights, Bitboard b_bishops, Bitboard b_rooks, Bitboard b_queens, Bitboard b_king,
									Bitboard EP_square, CastlingType castling, Color side_to_move, int ply) :
	white_pieces(w_pawns, w_knights, w_bishops, w_rooks, w_queens, w_king),
	black_pieces(b_pawns, b_knights, b_bishops, b_rooks, b_queens, b_king),
	occupied(w_pawns | w_knights | w_bishops | w_rooks | w_queens | w_king |
			 b_pawns | b_knights | b_bishops | b_rooks | b_queens | b_king),
	EP_square(EP_square),
	castling(castling & update_castling_rights(w_rooks, b_rooks)),
	side_to_move(side_to_move),
	ply(ply)
{
}

forceinline constexpr Position::Position(const Side& white_pieces, const Side& black_pieces, const Bitboard EP_square, const CastlingType castling, const Color side_to_move, const int ply) :
	white_pieces(white_pieces.pawns, white_pieces.knights, white_pieces.bishops,
				 white_pieces.rooks, white_pieces.queens, white_pieces.king),
	black_pieces(black_pieces.pawns, black_pieces.knights, black_pieces.bishops,
				 black_pieces.rooks, black_pieces.queens, black_pieces.king),
	occupied(this->white_pieces.pieces | this->black_pieces.pieces),
	EP_square(EP_square),
	castling(castling& update_castling_rights(white_pieces.rooks, black_pieces.rooks)),
	side_to_move(side_to_move),
	ply(ply)
{
}

forceinline constexpr CastlingType Position::get_curr_castling() const
{
	return castling >> (2 * side_to_move) & 0b11;
}

template<Color color>
forceinline constexpr const Side& Position::get_side() const
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

template<Color side_to_move, bool castling, bool EP>
forceinline Position make_move(const Position& pos, const Move& m)
{
	Bitboard EP_square = 0ULL;
	CastlingType curr_castling_perms = pos.castling;
	constexpr Color opposite_color = get_opposite_color<side_to_move>();
	Side own_pieces(pos.get_side<side_to_move>());
	Side enemy_pieces(pos.get_side<opposite_color>());
	Side& white_pieces = (side_to_move == WHITE ? own_pieces : enemy_pieces);
	Side& black_pieces = (side_to_move == BLACK ? own_pieces : enemy_pieces);
	if constexpr (castling)
	{
		if (m.to() == queenside_castling_rook<side_to_move>())
		{
			own_pieces.rooks ^= (m.to() | queenside_castling_rook_dest<side_to_move>());
			own_pieces.king ^= (m.from() | queenside_castling_king_dest<side_to_move>());
		}
		else if (m.to() == kingside_castling_rook<side_to_move>())
		{
			own_pieces.rooks ^= (m.to() | kingside_castling_rook_dest<side_to_move>());
			own_pieces.king ^= (m.from() | kingside_castling_king_dest<side_to_move>());
		}
		curr_castling_perms &= ~castling_perms<side_to_move>();
		return Position(
			white_pieces, black_pieces, EP_square, curr_castling_perms, opposite_color, pos.ply + 1
		);
	}
	if constexpr (EP)
	{
		const auto EP_index = bit_index(pos.EP_square);
		const auto EP_victim = EP_victims_lookup[EP_index];
		enemy_pieces.pawns ^= EP_victim;
		own_pieces.pawns ^= (m.from() | pos.EP_square);
		return Position(
			white_pieces, black_pieces, EP_square, curr_castling_perms, opposite_color, pos.ply + 1
		);
	}

	// normal move, maybe capture

	// remove castling perms because nobody wants them
	if (m.piece() == KING)
	{
		curr_castling_perms &= ~castling_perms<side_to_move>();
	}

	enemy_pieces.remove_pieces(m.to());
	Bitboard& piece_bb = own_pieces.get_piece_bb(m.piece());
	piece_bb ^= m.from() | m.to();

	if (m.piece() == PAWN)
	{
		if (std::abs((long long)bit_index(m.from()) - (long long)bit_index(m.to())) == 16)
		{
			EP_square = get_pawn_advances<side_to_move>(m.from());
		}
		if constexpr (side_to_move == WHITE)
		{
			piece_bb &= 0xffffffffffffff;
		}
		else
		{
			piece_bb &= 0xffffffffffffff00;
		}
		if (m.promotion_piece() != PIECE_TYPE_NONE)
		{
			own_pieces.get_piece_bb(m.promotion_piece()) |= m.to();
		}
	}

	return Position(white_pieces, black_pieces,
		EP_square, curr_castling_perms, opposite_color, pos.ply + 1);
}

forceinline Position make_move(const Position& pos, const Move& m)
{
	const auto& side_to_move = pos.side_to_move;
	const auto& own_pieces = ((pos.side_to_move == WHITE) ? pos.white_pieces : pos.black_pieces);
	const bool castling = (m.from() == own_pieces.king) && (m.to() & own_pieces.rooks);
	const bool EP = m.to() == pos.EP_square && (m.piece() == PAWN);

		 if (side_to_move == WHITE &&  castling && !EP)  return make_move<WHITE, true , false>(pos, m);
	else if (side_to_move == BLACK &&  castling && !EP)  return make_move<BLACK, true , false>(pos, m);
	else if (side_to_move == WHITE && !castling && !EP)  return make_move<WHITE, false, false>(pos, m);
	else if (side_to_move == BLACK && !castling && !EP)  return make_move<BLACK, false, false>(pos, m);
	else if (side_to_move == WHITE && !castling &&  EP)  return make_move<WHITE, false, true>(pos, m);
	else if (side_to_move == BLACK && !castling &&  EP)  return make_move<BLACK, false, true>(pos, m);

	return Position();
}


inline static constexpr Position startpos;