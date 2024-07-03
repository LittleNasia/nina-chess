#include "attacks.h"
#include "position.h"


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
		if (m.to == queenside_castling_rook<side_to_move>())
		{
			own_pieces.rooks ^= (m.to | queenside_castling_rook_dest<side_to_move>());
			own_pieces.king ^= (m.from | queenside_castling_king_dest<side_to_move>());
		}
		else if (m.to == kingside_castling_rook<side_to_move>())
		{
			own_pieces.rooks ^= (m.to | kingside_castling_rook_dest<side_to_move>());
			own_pieces.king ^= (m.from | kingside_castling_king_dest<side_to_move>());
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
		own_pieces.pawns ^= (m.from | pos.EP_square);
		return Position(
			white_pieces, black_pieces, EP_square, curr_castling_perms, opposite_color, pos.ply + 1
		);
	}

	// normal move, maybe capture

	// remove castling perms because nobody wants them
	if (m.piece == KING)
	{
		curr_castling_perms &= ~castling_perms<side_to_move>();
	}
	
	enemy_pieces.remove_pieces(m.to);
	Bitboard& piece_bb = own_pieces.get_piece_bb(m.piece);
	piece_bb ^= m.from|m.to;

	if (m.piece == PAWN)
	{
		if (std::abs((long long)bit_index(m.from) - (long long)bit_index(m.to)) == 16)
		{
			EP_square = get_pawn_advances<side_to_move>(m.from);
		}
		if constexpr (side_to_move == WHITE)
		{
			piece_bb &= 0xffffffffffffff;
		}
		else
		{
			piece_bb &= 0xffffffffffffff00;
		}
		if (m.promotion_piece != PIECE_TYPE_NONE)
		{
			own_pieces.get_piece_bb(m.promotion_piece) |= m.to;
		}
	}

	return Position(white_pieces, black_pieces, 
		EP_square, curr_castling_perms, opposite_color, pos.ply + 1);
}
 
Position make_move(const Position& pos, const Move& m)
{
	const auto& side_to_move = pos.side_to_move;
	const auto& own_pieces = ((pos.side_to_move == WHITE) ? pos.white_pieces : pos.black_pieces);
	const bool castling = (m.from == own_pieces.king) && (m.to & own_pieces.rooks) && pos.get_curr_castling();
	const bool EP = m.to == pos.EP_square && (m.piece == PAWN);

	if (side_to_move == WHITE && castling && !EP)  return make_move<WHITE, true, false>(pos, m);
	if (side_to_move == BLACK && castling && !EP)  return make_move<BLACK, true, false>(pos, m);
	if (side_to_move == WHITE && !castling && !EP) return make_move<WHITE, false, false>(pos, m);
	if (side_to_move == BLACK && !castling && !EP) return make_move<BLACK, false, false>(pos, m);
	if (side_to_move == WHITE && !castling && EP)  return make_move<WHITE, false, true>(pos, m);
	if (side_to_move == BLACK && !castling && EP)  return make_move<BLACK, false, true>(pos, m);
}


