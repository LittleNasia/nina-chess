#include "position.h"
#include "zobrist.h"

forceinline Position::Position() :
	white_pieces(65280ULL, 66ULL, 36ULL, 129ULL, 16ULL, 8ULL),
	black_pieces(71776119061217280ULL, 4755801206503243776ULL, 2594073385365405696ULL,
		9295429630892703744ULL, 1152921504606846976ULL, 576460752303423488ULL),
	occupied(18446462598732906495ULL), EP_square(0ULL), side_to_move(WHITE), castling(0b1111),
	hash(CalculateHash()),
	fifty_move_rule(0)
{
}

forceinline Position::Position(const Side& white_pieces, const Side& black_pieces,
		const Bitboard EP_square, const CastlingType castling, const Color side_to_move,
		const uint32_t fifty_move_rule) :
	white_pieces(white_pieces.pawns, white_pieces.knights, white_pieces.bishops, white_pieces.rooks, white_pieces.queens, white_pieces.king),
	black_pieces(black_pieces.pawns, black_pieces.knights, black_pieces.bishops, black_pieces.rooks, black_pieces.queens, black_pieces.king),
	occupied(this->white_pieces.pieces | this->black_pieces.pieces),
	EP_square(EP_square),
	castling(castling),
	side_to_move(side_to_move),
	hash(CalculateHash()),
	fifty_move_rule(fifty_move_rule)
{
}

forceinline Position::Position(const Side& white_pieces, const Side& black_pieces,
		const Bitboard EP_square, const CastlingType castling, const Color side_to_move,
		const uint32_t fifty_move_rule, const uint64_t hash) :
	white_pieces(white_pieces.pawns, white_pieces.knights, white_pieces.bishops, white_pieces.rooks, white_pieces.queens, white_pieces.king),
	black_pieces(black_pieces.pawns, black_pieces.knights, black_pieces.bishops, black_pieces.rooks, black_pieces.queens, black_pieces.king),
	occupied(this->white_pieces.pieces | this->black_pieces.pieces),
	EP_square(EP_square),
	castling(castling),
	side_to_move(side_to_move),
	hash(hash),
	fifty_move_rule(fifty_move_rule)
{
}

forceinline constexpr CastlingType Position::GetCurrentCastling() const
{
	return castling >> (2 * side_to_move) & 0b11;
}

forceinline uint64_t Position::CalculateHash() const
{
	uint64_t hash = 0ULL;
	hash ^= (zobrist_side_to_move * side_to_move);
	hash ^= zobrist_castling[castling];
	hash ^= zobrist_ep_square[bit_index(EP_square)];

	for (Color color = WHITE; auto& side : { white_pieces, black_pieces })
	{
		for (PieceType piece = PAWN; piece < PIECE_TYPE_NONE; piece++)
		{
			Bitboard piece_bb = side.get_piece_bb(piece);
			while (piece_bb)
			{
				const Bitboard square = pop_bit(piece_bb);
				const size_t index = bit_index(square);
				hash ^= zobrist_keys[index][color * PIECE_TYPE_NONE + piece];
			}
		}
		color = BLACK;
	}
	return hash;
}

forceinline constexpr bool Position::IsDrawn() const
{
	return IsFiftyMoveRule() || IsInsufficientMaterial();
}

forceinline constexpr bool Position::IsFiftyMoveRule() const
{
	return fifty_move_rule >= 100;
}

forceinline constexpr bool Position::IsInsufficientMaterial() const
{
	// always can mate with horizontal sliders or potential horizontal sliders
	if (
		(white_pieces.pawns | black_pieces.pawns) ||
		(white_pieces.rooks | black_pieces.rooks) ||
		(white_pieces.queens | black_pieces.queens)
		)
	{
		return false;
	}

	/* only bishops and knights beyond this point */

	const uint32_t occupied_count = popcnt(occupied);

	// king bishop/knight vs king is always a draw
	if (occupied_count <= 3)
		return true;

	if (occupied_count >= 4)
	{
		// there always can be mate with two knights on the board
		if (popcnt(white_pieces.knights | black_pieces.knights) >= 2)
			return false;

		// bishops on the same color are always a draw
		const Bitboard light_squared_bishops = light_squares & (white_pieces.bishops | black_pieces.bishops);
		const Bitboard dark_squared_bishops = dark_squares & (white_pieces.bishops | black_pieces.bishops);
		const uint32_t light_squared_bishops_count = popcnt(light_squared_bishops);
		const uint32_t dark_squared_bishops_count = popcnt(dark_squared_bishops);

		if (!(light_squared_bishops_count == 0 || dark_squared_bishops_count == 0))
			return false;

		return true;
	}

	return false;
}

forceinline constexpr void Position::UpdateOccupiedBitboard()
{
	white_pieces.pieces = white_pieces.pawns | white_pieces.knights | white_pieces.bishops | white_pieces.rooks | white_pieces.queens | white_pieces.king;
	black_pieces.pieces = black_pieces.pawns | black_pieces.knights | black_pieces.bishops | black_pieces.rooks | black_pieces.queens | black_pieces.king;
	occupied = white_pieces.pieces | black_pieces.pieces;
}


template<Color side_to_move>
forceinline constexpr uint64_t position::update_hash(uint64_t hash, const PieceType moving_piece, Bitboard move)
{
	const Bitboard first_square = pop_bit(move);
	const Bitboard second_square = pop_bit(move);
	hash ^= zobrist_keys[bit_index(first_square)][side_to_move * PIECE_TYPE_NONE + moving_piece];
	if(second_square)
		hash ^= zobrist_keys[bit_index(second_square)][side_to_move * PIECE_TYPE_NONE + moving_piece];
	return hash;
}

template<Color color>
forceinline constexpr const Side& Position::GetSide() const
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

template<Color color>
inline constexpr Side& Position::GetSide()
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
forceinline Position position::MakeMove(const Position& pos, const Move& m)
{
	constexpr Color opposite_color = get_opposite_color<side_to_move>();

	Position new_pos(pos);
	new_pos.hash ^= zobrist_side_to_move;
	new_pos.hash ^= zobrist_ep_square[bit_index(pos.EP_square)];
	new_pos.EP_square = 0ULL;
	new_pos.fifty_move_rule++;
	new_pos.side_to_move = opposite_color;
	
	Side& own_pieces(new_pos.GetSide<side_to_move>());
	Side& enemy_pieces(new_pos.GetSide<opposite_color>());
	Side& white_pieces = (side_to_move == WHITE ? own_pieces : enemy_pieces);
	Side& black_pieces = (side_to_move == BLACK ? own_pieces : enemy_pieces);
	if constexpr (castling)
	{
		new_pos.hash ^= zobrist_castling[new_pos.castling];
		if (m.to() == queenside_castling_rook<side_to_move>())
		{
			const Bitboard rook_move_bb = (m.to() | queenside_castling_rook_dest<side_to_move>());
			const Bitboard king_move_bb = (m.from() | queenside_castling_king_dest<side_to_move>());
			own_pieces.rooks ^= rook_move_bb;
			own_pieces.king ^= king_move_bb;
			new_pos.hash = update_hash<side_to_move>(new_pos.hash, ROOK, rook_move_bb);
			new_pos.hash = update_hash<side_to_move>(new_pos.hash, KING, king_move_bb);
		}
		else if (m.to() == kingside_castling_rook<side_to_move>())
		{
			const Bitboard rook_move_bb = (m.to() | kingside_castling_rook_dest<side_to_move>());
			const Bitboard king_move_bb = (m.from() | kingside_castling_king_dest<side_to_move>());
			own_pieces.rooks ^= rook_move_bb;
			own_pieces.king ^= king_move_bb;
			new_pos.hash = update_hash<side_to_move>(new_pos.hash, ROOK, rook_move_bb);
			new_pos.hash = update_hash<side_to_move>(new_pos.hash, KING, king_move_bb);
		}
		new_pos.castling &= ~castling_perms<side_to_move>();
		new_pos.hash ^= zobrist_castling[new_pos.castling];
		new_pos.hash ^= zobrist_ep_square[bit_index(new_pos.EP_square)];
	}
	else if constexpr (EP)
	{
		new_pos.fifty_move_rule = 0;
		const auto EP_index = bit_index(pos.EP_square);
		const auto EP_victim = EP_victims_lookup[EP_index];
		enemy_pieces.pawns ^= EP_victim;
		own_pieces.pawns ^= (m.from() | pos.EP_square);
		new_pos.hash = update_hash<side_to_move>(new_pos.hash, PAWN, (m.from() | pos.EP_square));
		new_pos.hash = update_hash<opposite_color>(new_pos.hash, PAWN, EP_victim);
		new_pos.hash ^= zobrist_ep_square[bit_index(new_pos.EP_square)];
	}
	else
	{
		new_pos.hash ^= zobrist_castling[new_pos.castling];
		// normal move, maybe capture

		// remove castling perms because nobody wants them
		if (m.piece() == KING)
		{
			new_pos.castling &= ~castling_perms<side_to_move>();
		}

		if (m.to() & enemy_pieces.pieces)
		{
			new_pos.fifty_move_rule = 0;

			if (m.to() & enemy_pieces.pawns)
				new_pos.hash = update_hash<opposite_color>(new_pos.hash, PAWN, m.to());
			else if (m.to() & enemy_pieces.knights)
				new_pos.hash = update_hash<opposite_color>(new_pos.hash, KNIGHT, m.to());
			else if (m.to() & enemy_pieces.bishops)
				new_pos.hash = update_hash<opposite_color>(new_pos.hash, BISHOP, m.to());
			else if (m.to() & enemy_pieces.rooks)
				new_pos.hash = update_hash<opposite_color>(new_pos.hash, ROOK, m.to());
			else if (m.to() & enemy_pieces.queens)
				new_pos.hash = update_hash<opposite_color>(new_pos.hash, QUEEN, m.to());
			enemy_pieces.remove_pieces(m.to());
		}

		Bitboard& piece_bb = own_pieces.get_piece_bb(m.piece());
		piece_bb ^= m.from() | m.to();
		new_pos.hash = update_hash<side_to_move>(new_pos.hash, m.piece(), m.from() | m.to());

		if (m.piece() == PAWN)
		{
			new_pos.fifty_move_rule = 0;
			if (std::abs((long long)bit_index(m.from()) - (long long)bit_index(m.to())) == 16)
			{
				new_pos.EP_square = get_pawn_advances<side_to_move>(m.from());
			}
			if constexpr (side_to_move == WHITE)
			{
				piece_bb &= 0xffffffffffffff;
				if (m.to() & ~0xffffffffffffff)
				{
					new_pos.hash ^= zobrist_keys[bit_index(m.to())][WHITE_PAWN];
				}
			}
			else
			{
				piece_bb &= 0xffffffffffffff00;
				if (m.to() & ~0xffffffffffffff00)
				{
					new_pos.hash ^= zobrist_keys[bit_index(m.to())][BLACK_PAWN];
				}
			}
			if (m.promotion_piece() != PIECE_TYPE_NONE)
			{
				own_pieces.get_piece_bb(m.promotion_piece()) |= m.to();
				new_pos.hash = update_hash<side_to_move>(new_pos.hash, m.promotion_piece(), m.to());
			}
		}
		new_pos.castling &= update_castling_rights(white_pieces.rooks, black_pieces.rooks);
		new_pos.hash ^= zobrist_ep_square[bit_index(new_pos.EP_square)];
		new_pos.hash ^= zobrist_castling[new_pos.castling];
	}
	
	new_pos.UpdateOccupiedBitboard();

	DEBUG_IF(new_pos.hash != new_pos.CalculateHash())
		throw std::runtime_error("hashes don't match");

	return new_pos;
}

template<Color side_to_move>
forceinline Position position::MakeMove(const Position& pos, const Move& m)
{
	const auto& own_pieces = pos.GetSide<side_to_move>();
	const bool castling = m.is_castling();
	const bool EP = m.is_EP();

	if		(!castling && !EP)  return MakeMove<side_to_move, false, false>(pos, m); 
	else if ( castling && !EP)  return MakeMove<side_to_move, true , false>(pos, m);
	else if (!castling &&  EP)  return MakeMove<side_to_move, false, true >(pos, m);

	return Position();
}

forceinline Position position::MakeMove(const Position& pos, const Move& m)
{
	const auto& side_to_move = pos.side_to_move;
	const auto& own_pieces = ((pos.side_to_move == WHITE) ? pos.white_pieces : pos.black_pieces);
	const bool castling = m.is_castling();
	const bool EP = m.is_EP();

	if		(side_to_move == WHITE && !castling && !EP)  return MakeMove<WHITE, false, false>(pos, m);
	else if (side_to_move == BLACK && !castling && !EP)  return MakeMove<BLACK, false, false>(pos, m);
	else if (side_to_move == WHITE &&  castling && !EP)  return MakeMove<WHITE, true , false>(pos, m);
	else if (side_to_move == BLACK &&  castling && !EP)  return MakeMove<BLACK, true , false>(pos, m);
	else if (side_to_move == WHITE && !castling &&  EP)  return MakeMove<WHITE, false, true >(pos, m);
	else if (side_to_move == BLACK && !castling &&  EP)  return MakeMove<BLACK, false, true >(pos, m);

	return Position();
}