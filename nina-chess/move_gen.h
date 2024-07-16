#pragma once
#include "utils.h"

#include "attacks.h"
#include "bitmasks.h"
#include "position.h"

forceinline void fill_pinmask(const size_t square, Bitboard& pinmask, Bitboard pinners)
{
	// Evil branch
	while (pinners)
	{
		const Bitboard pinner = pop_bit(pinners);
		pinmask |= PinBetween[square][bit_index(pinner)];
	}
}

forceinline void fill_checkmask(const size_t square, Bitboard& checkmask, Bitboard checkers)
{
	// More evil branches
	while (checkers)
	{
		const Bitboard checker = pop_bit(checkers);
		checkmask |= PinBetween[square][bit_index(checker)] | (1ULL<<square);
	}
}
template<Color color>
forceinline constexpr Bitboard get_legal_left_pawn_captures(const Bitboard pawns, const Bitboard bishop_pinmask, const Bitboard rook_pinmask, const Bitboard enemy_pieces)
{
	const Bitboard pinmask = (bishop_pinmask | rook_pinmask);
	const Bitboard unpinned_pawns = pawns & ~pinmask;
	const Bitboard pinned_pawns = pawns & pinmask;
	const Bitboard unpinned_pawn_captures = get_pawn_left_attacks<color>(unpinned_pawns) & enemy_pieces;
	// pawns pinned by rooks can't capture, pawns pinned by bishops can only capture in the direction of a bishop
	const Bitboard pinned_pawn_captures = (get_pawn_left_attacks<color>(pinned_pawns & ~rook_pinmask) & bishop_pinmask) & enemy_pieces;
	return unpinned_pawn_captures | pinned_pawn_captures;
}

template<Color color>
forceinline constexpr Bitboard get_legal_right_pawn_captures(const Bitboard pawns, const Bitboard bishop_pinmask, const Bitboard rook_pinmask, const Bitboard enemy_pieces)
{
	const Bitboard pinmask = (bishop_pinmask | rook_pinmask);
	const Bitboard unpinned_pawns = pawns & ~pinmask;
	const Bitboard pinned_pawns = pawns & pinmask;
	const Bitboard unpinned_pawn_captures = get_pawn_right_attacks<color>(unpinned_pawns) & enemy_pieces;
	// pawns pinned by rooks can't capture, pawns pinned by bishops can only capture in the direction of a bishop
	const Bitboard pinned_pawn_captures = (get_pawn_right_attacks<color>(pinned_pawns & ~rook_pinmask) & bishop_pinmask) & enemy_pieces;
	return unpinned_pawn_captures | pinned_pawn_captures;
}

template<Color color>
forceinline constexpr Bitboard get_legal_pawn_advances(const Bitboard pawns, const Bitboard bishop_pinmask, const Bitboard rook_pinmask, const Bitboard occupied)
{
	const Bitboard pinmask = bishop_pinmask | rook_pinmask;
	const Bitboard unpinned_pawns = pawns & ~pinmask;
	const Bitboard pinned_pawns = pawns & pinmask;
	const Bitboard unpinned_pawn_advances = get_pawn_advances<color>(unpinned_pawns) & ~occupied;
	const Bitboard pinned_pawn_advances = (get_pawn_advances<color>(pinned_pawns & ~bishop_pinmask) & rook_pinmask) & ~occupied;
	return unpinned_pawn_advances | pinned_pawn_advances;
}

template<Color color>
forceinline constexpr Bitboard get_pawn_double_advances(const Bitboard pushed_pawn_moves, const Bitboard occupied)
{
	return pawn_advances<color>(pushed_pawn_moves & get_push_candidates_bitmask<color>()) & ~occupied;
}

forceinline constexpr Bitboard get_king_moves(const size_t king_index, const Bitboard attacked_squares)
{
	return king_moves[king_index] & ~attacked_squares;
}

struct MoveList
{
	Move moves[100];
	forceinline constexpr MoveList() = default;
	forceinline void push_move(const Move&& move)
	{
		moves[num_moves++] = move;
	}
	forceinline void reset()
	{
		num_moves = 0;
	}
	forceinline constexpr uint32_t get_num_moves() const { return num_moves; }
private:
	uint32_t num_moves = 0;
};

template<PieceType piece_type, Color color>
forceinline void write_moves(MoveList& moves, Bitboard moves_mask, const uint32_t piece_index)
{
	constexpr auto moving_piece = piece_type;
	while (moves_mask)
	{
		const Bitboard move = pop_bit(moves_mask);
		const uint32_t move_target = bit_index(move);
		moves.push_move({ piece_index, move_target, moving_piece });
	}
}

template<Color color>
forceinline void write_pawn_moves(MoveList& moves, const Bitboard left_pawn_captures, const Bitboard right_pawn_captures,
	const Bitboard legal_pawn_advances, const Bitboard pawn_double_advances, Bitboard pawns)
{
	constexpr Piece moving_piece = PAWN;
	while (pawns)
	{
		const auto pawn = pop_bit(pawns);
		const auto piece_index = bit_index(pawn);
		Bitboard curr_pawn_moves = 0ULL;
		curr_pawn_moves |= get_pawn_left_attacks<color>(pawn) & left_pawn_captures;
		curr_pawn_moves |= get_pawn_right_attacks<color>(pawn) & right_pawn_captures;
		curr_pawn_moves |= get_pawn_advances<color>(pawn) & legal_pawn_advances;
		curr_pawn_moves |= get_double_advance_target<color>(pawn) & pawn_double_advances;
		while (curr_pawn_moves)
		{
			const Bitboard move = pop_bit(curr_pawn_moves);
			const uint32_t move_target = bit_index(move);

			if (move & promotion_rank<color>())
			{
				for (const auto promotion_piece : promotion_pieces)
				{
					moves.push_move({ piece_index, move_target, moving_piece, promotion_piece });
				}
			}
			else
				moves.push_move({ piece_index, move_target, moving_piece });
		}
			
	}
}  

template<Color color, bool check, bool unblockable_check>
forceinline void write_slider_moves(MoveList& move_list, Bitboard movable_bishops, Bitboard movable_rooks, Bitboard queens, const Bitboard occupied, const Bitboard allies,
	const Bitboard bishop_pinmask, const Bitboard rook_pinmask, const Bitboard checkers, const Bitboard bishop_checkmask, const Bitboard rook_checkmask)
{
	while (movable_bishops)
	{
		const Bitboard curr_bishop = pop_bit(movable_bishops);
		Bitboard curr_bishop_moves = get_single_bishop_attacks(curr_bishop, occupied);
		if constexpr (check && unblockable_check)
		{
			curr_bishop_moves &= checkers;
		}
		else if constexpr (check && !unblockable_check)
		{
			curr_bishop_moves &= (rook_checkmask | bishop_checkmask);
		}
		if(curr_bishop & bishop_pinmask)
			curr_bishop_moves &= bishop_pinmask;
		curr_bishop_moves &= ~allies;
		if(curr_bishop_moves)
			write_moves<BISHOP, color>(move_list, curr_bishop_moves, bit_index(curr_bishop));
	}
	while (movable_rooks)
	{
		const Bitboard curr_rook = pop_bit(movable_rooks);
		Bitboard curr_rook_moves = get_single_rook_attacks(curr_rook, occupied);
		if constexpr (check && unblockable_check)
		{
			curr_rook_moves &= checkers;
		}
		else if constexpr (check && !unblockable_check)
		{
			curr_rook_moves &= (rook_checkmask | bishop_checkmask);
		}
		if (curr_rook & rook_pinmask)
			curr_rook_moves &= rook_pinmask;
		curr_rook_moves &= ~allies;
		if(curr_rook_moves)
			write_moves<ROOK, color>(move_list, curr_rook_moves, bit_index(curr_rook));
	}
	while (queens)
	{
		const Bitboard curr_queen = pop_bit(queens);
		const Bitboard curr_queen_rook_moves = get_single_rook_attacks(curr_queen, occupied);
		const Bitboard curr_queen_bishop_moves = get_single_bishop_attacks(curr_queen, occupied);
		Bitboard curr_queen_moves = curr_queen_rook_moves | curr_queen_bishop_moves;

		if(curr_queen & rook_pinmask)
			curr_queen_moves &= (rook_pinmask & curr_queen_rook_moves);
		if (curr_queen & bishop_pinmask)
			curr_queen_moves &= (bishop_pinmask & curr_queen_bishop_moves);
		curr_queen_moves &= ~allies;
		if constexpr (check && unblockable_check)
		{
			curr_queen_moves &= checkers;
		}
		else if constexpr (check && !unblockable_check)
		{
			curr_queen_moves &= (rook_checkmask | bishop_checkmask);
		}
		if (curr_queen_moves)
			write_moves<QUEEN, color>(move_list, curr_queen_moves, bit_index(curr_queen));
	}
}

template<Color color, bool check, bool unblockable_check>
forceinline void write_knight_moves(MoveList& move_list, Bitboard movable_knights, const Bitboard allies,
	const Bitboard checkers, const Bitboard bishop_checkmask, const Bitboard rook_checkmask)
{
	constexpr Piece moving_piece = KNIGHT;
	while (movable_knights)
	{
		const Bitboard curr_knight = pop_bit(movable_knights);
		const uint32_t knight_index = bit_index(curr_knight);
		Bitboard curr_knight_moves = knight_moves[knight_index];
		if constexpr (check && unblockable_check)
		{
			curr_knight_moves &= checkers;
		}
		else if constexpr (check && !unblockable_check)
		{
			curr_knight_moves &= (bishop_checkmask | rook_checkmask);
		}
		curr_knight_moves &= ~allies;
		
		while (curr_knight_moves)
		{
			const Bitboard move = pop_bit(curr_knight_moves);
			const uint32_t move_target = bit_index(move);

			move_list.push_move({ knight_index, move_target, moving_piece });
		}
	}
}

template<Color color, size_t castling, bool hasEP>
forceinline MoveList& generate_moves(MoveList& move_list, const Position& Position)
{
	constexpr auto opposite_color = get_opposite_color<color>();
	const auto& curr_pieces = Position.get_side<color>();
	const auto& opposite_pieces = Position.get_side<opposite_color>();
	const auto& king = curr_pieces.king;
	const auto& king_index = bit_index(curr_pieces.king);
	const Bitboard bishop_xray_from_king = bishop_xray_masks[king_index];
	const Bitboard rook_xray_from_king = rook_xray_masks[king_index];
	// king can't move to these squares
	const Bitboard attacked_squares = get_all_attacks<opposite_color>(opposite_pieces, Position.occupied ^ king);

	const bool in_check = curr_pieces.king & attacked_squares;

	Bitboard bishop_checkers = 0ULL;
	Bitboard bishop_pinners = 0ULL;
	Bitboard bishop_pinmask = 0ULL;
	Bitboard bishop_checkmask = 0ULL;

	Bitboard rook_checkers = 0ULL;
	Bitboard rook_pinners = 0ULL;
	Bitboard rook_pinmask = 0ULL;
	Bitboard rook_checkmask = 0ULL;

	const Bitboard knight_checkers = knight_moves[king_index] & opposite_pieces.knights;
	const Bitboard pawn_checkers = get_pawn_attacks<color>(king) & opposite_pieces.pawns;
	const Bitboard unblockable_checkers = knight_checkers | pawn_checkers;

	const Bitboard enemy_rook_queen = opposite_pieces.rooks | opposite_pieces.queens;
	const Bitboard enemy_bishop_queen = opposite_pieces.bishops | opposite_pieces.queens;

	// quick check to save computation in most cases
	// a bishop can't pin a piece if it's not on the same diagonal with king etc.
	if (bishop_xray_from_king & enemy_bishop_queen)
	{
		// bitboard consisting of all pieces attacked by the king if it was a bishop
		const Bitboard bishop_king_attacks = get_single_bishop_attacks(king, Position.occupied);
		bishop_checkers |= enemy_bishop_queen & bishop_king_attacks;
		fill_checkmask(king_index, bishop_checkmask, bishop_checkers);
		const Bitboard attacked_pieces = Position.occupied & bishop_king_attacks;
		const Bitboard removed_attacked_pieces = Position.occupied ^ attacked_pieces;
		const Bitboard attacked_pieces_behind = get_single_bishop_attacks(king, removed_attacked_pieces) & removed_attacked_pieces;
		bishop_pinners |= attacked_pieces_behind & enemy_bishop_queen;
		fill_pinmask(king_index, bishop_pinmask, bishop_pinners);
	}
	if (rook_xray_from_king & enemy_rook_queen)
	{
		const Bitboard rook_king_attacks = get_single_rook_attacks(king, Position.occupied);
		rook_checkers |= enemy_rook_queen & rook_king_attacks;
		fill_checkmask(king_index, rook_checkmask, rook_checkers);
		const Bitboard attacked_pieces = Position.occupied & rook_king_attacks;
		const Bitboard removed_attacked_pieces = Position.occupied ^ attacked_pieces;
		const Bitboard attacked_pieces_behind = get_single_rook_attacks(king, removed_attacked_pieces) & removed_attacked_pieces;
		rook_pinners |= attacked_pieces_behind & enemy_rook_queen;
		fill_pinmask(king_index, rook_pinmask, rook_pinners);
	}

	const uint32_t num_checkers = popcnt(rook_checkers | bishop_checkers | knight_checkers | pawn_checkers);

	// king can always move, unless she can't
	const Bitboard legal_king_moves = get_king_moves(king_index, attacked_squares) & ~curr_pieces.pieces;
	write_moves<KING, color>(move_list, legal_king_moves, king_index);
	if (num_checkers > 1)
	{
		// double check. The only legal moves are king moves to run away from check, can't block it 
		return move_list;
	}
	// single or no checks
	// pinned pieces can't move in directions they can't move in
	// in check by sliders they can only capture the checking piece or block it 
	// in check by knights or pawns they can only capture the piece 
	Bitboard pawn_left_captures = get_legal_left_pawn_captures<color>(curr_pieces.pawns, bishop_pinmask, rook_pinmask, opposite_pieces.pieces);
	Bitboard pawn_right_captures = get_legal_right_pawn_captures<color>(curr_pieces.pawns, bishop_pinmask, rook_pinmask, opposite_pieces.pieces);
	Bitboard legal_pawn_advances = get_legal_pawn_advances<color>(curr_pieces.pawns, bishop_pinmask, rook_pinmask, Position.occupied);
	Bitboard legal_pawn_double_advances = legal_pawn_advances & get_push_candidates_bitmask<color>();
	legal_pawn_double_advances = get_pawn_advances<color>(legal_pawn_double_advances) & ~Position.occupied;

	// bishops pinned by a rook can't move 
	const Bitboard movable_bishops = curr_pieces.bishops & ~rook_pinmask;
	// likewise, rooks pinned by 
	const Bitboard movable_rooks = curr_pieces.rooks & ~bishop_pinmask;
	const Bitboard movable_knights = curr_pieces.knights & ~(bishop_pinmask | rook_pinmask);
	// pinned queens can move always
	
	// no checks
	if (!(knight_checkers | pawn_checkers | rook_checkers | bishop_checkers))
	{
		write_slider_moves<color, false, false>(move_list, movable_bishops, movable_rooks,
			curr_pieces.queens, Position.occupied, curr_pieces.pieces, bishop_pinmask,
			rook_pinmask, 0, 0, 0);
		write_knight_moves<color, false, false>(move_list, movable_knights, curr_pieces.pieces, 0,
			0, 0);
	}
	// blockable checks
	else if (!(knight_checkers | pawn_checkers) && (rook_checkers | bishop_checkers))
	{
		write_slider_moves<color, true, false>(move_list, movable_bishops, movable_rooks,
			curr_pieces.queens, Position.occupied, curr_pieces.pieces, bishop_pinmask,
			rook_pinmask, rook_checkers | bishop_checkers, bishop_checkmask, rook_checkmask);
		pawn_left_captures &= (bishop_checkmask|rook_checkmask);
		pawn_right_captures &= (bishop_checkmask | rook_checkmask);
		legal_pawn_advances &= (bishop_checkmask | rook_checkmask);
		legal_pawn_double_advances &= (bishop_checkmask | rook_checkmask);
		write_knight_moves<color, true, false>(move_list, movable_knights, curr_pieces.pieces, rook_checkers | bishop_checkers,
			bishop_checkmask, rook_checkmask);
	}
	// unblockable checks
	else
	{
		write_slider_moves<color, true, true>(move_list, movable_bishops, movable_rooks,
			curr_pieces.queens, Position.occupied, curr_pieces.pieces, bishop_pinmask,
			rook_pinmask, knight_checkers | pawn_checkers, 0, 0);
		pawn_left_captures &= knight_checkers | pawn_checkers;
		pawn_right_captures &= knight_checkers | pawn_checkers;
		legal_pawn_advances = 0;
		legal_pawn_double_advances = 0;
		write_knight_moves<color, true, true>(move_list, movable_knights, curr_pieces.pieces, knight_checkers | pawn_checkers,
			0, 0);
	}
	// all bad moves have been pruned
	write_pawn_moves<color>(move_list, pawn_left_captures, pawn_right_captures, legal_pawn_advances, legal_pawn_double_advances, curr_pieces.pawns);


	if constexpr (hasEP)
	{
		const uint32_t EP_index = bit_index(Position.EP_square);
		const Bitboard victim = EP_victims_lookup[EP_index];
		// move the EP pawn and remove the target, see if king is attacked by a slider
		const Bitboard victim_removed = Position.occupied ^ victim;
		// pawns that are pinned by rooks can't EP
		Bitboard EP_candidates = EP_candidates_lookup[EP_index] & (curr_pieces.pawns & ~rook_pinmask);
		while (EP_candidates)
		{
			const Bitboard EP_candidate = pop_bit(EP_candidates);
			const Bitboard occupied_removed = victim_removed ^ (EP_candidate | Position.EP_square);
			const Bitboard king_bishop_attacks = get_single_bishop_attacks(king, occupied_removed);
			const Bitboard king_rook_attacks = get_single_rook_attacks(king, occupied_removed);
			if (
				!(king_bishop_attacks & enemy_bishop_queen)
				&& 
				!(king_rook_attacks & enemy_rook_queen)
				)
			{
				write_moves<PAWN, color>(move_list, Position.EP_square, bit_index(EP_candidate));
			}
		}
	}

	constexpr auto castling_perms = castling;
	// kingside castling
	if constexpr (castling_perms & 0b1)
	{
		const bool can_castle = !((attacked_squares & kingside_castling_castling_king_path<color>()) ||
			(Position.occupied & (kingside_castling_castling_rook_path<color>() & ~king)));
		if (can_castle)
		{
			write_moves<KING, color>(move_list, kingside_castling_rook<color>(), king_index);
		}
	}
	// queenside castling
	if constexpr (castling_perms & 0b10)
	{
		const bool can_castle = !((attacked_squares & queenside_castling_king_path<color>()) ||
			(Position.occupied & (queenside_castling_rook_path<color>() & ~king)));
		if (can_castle)
		{
			write_moves<KING, color>(move_list, queenside_castling_rook<color>(), king_index);
		}
	}
	return move_list;
}

forceinline MoveList generate_moves(const Position& position)
{
	MoveList move_list;
	const bool EP = position.EP_square;
	const Color color = position.side_to_move;
	const CastlingType castling = position.get_curr_castling();

		 if (color == WHITE && castling == 0b11 && !EP) generate_moves<WHITE, 0b11, false>(move_list, position);
	else if (color == BLACK && castling == 0b11 && !EP) generate_moves<BLACK, 0b11, false>(move_list, position);
	else if (color == WHITE && castling == 0b00 && !EP) generate_moves<WHITE, 0b00, false>(move_list, position);
	else if (color == BLACK && castling == 0b00 && !EP) generate_moves<BLACK, 0b00, false>(move_list, position);
	else if (color == WHITE && castling == 0b01 && !EP) generate_moves<WHITE, 0b01, false>(move_list, position);
	else if (color == BLACK && castling == 0b01 && !EP) generate_moves<BLACK, 0b01, false>(move_list, position);
	else if (color == WHITE && castling == 0b10 && !EP) generate_moves<WHITE, 0b10, false>(move_list, position);
	else if (color == BLACK && castling == 0b10 && !EP) generate_moves<BLACK, 0b10, false>(move_list, position);
	else if (color == WHITE && castling == 0b11 &&  EP) generate_moves<WHITE, 0b11, true >(move_list, position);
	else if (color == BLACK && castling == 0b11 &&  EP) generate_moves<BLACK, 0b11, true >(move_list, position);
	else if (color == WHITE && castling == 0b00 &&  EP) generate_moves<WHITE, 0b00, true >(move_list, position);
	else if (color == BLACK && castling == 0b00 &&  EP) generate_moves<BLACK, 0b00, true >(move_list, position);
	else if (color == WHITE && castling == 0b01 &&  EP) generate_moves<WHITE, 0b01, true >(move_list, position);
	else if (color == BLACK && castling == 0b01 &&  EP) generate_moves<BLACK, 0b01, true >(move_list, position);
	else if (color == WHITE && castling == 0b10 &&  EP) generate_moves<WHITE, 0b10, true >(move_list, position);
	else if (color == BLACK && castling == 0b10 &&  EP) generate_moves<BLACK, 0b10, true >(move_list, position);

	return move_list;
}


