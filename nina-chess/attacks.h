#pragma once
#include "utils.h"

#include "bit_manip.h"
#include "bitmasks.h"
#include "side.h"

template <Color color>
forceinline constexpr Bitboard get_pawn_advances(const Bitboard pawns)
{
	validate_color<color>();
    if constexpr (color == WHITE)
    {
        return pawns << 8;
    }
    else
    {
        return pawns >> 8;
    }
}

template <Color color>
forceinline constexpr Bitboard get_double_advance_target(const Bitboard pawns)
{
    validate_color<color>();
    if constexpr (color == WHITE)
    {
        return pawns << 16;
    }
    else
    {
        return pawns >> 16;
    }
}

template<Color color>
forceinline constexpr Bitboard get_push_candidates_bitmask()
{
    validate_color<color>();
    if constexpr (color == WHITE)
    {
        return row_bitmasks[2];
    }
    else
    {
        return row_bitmasks[5];
    }
}

template <Color color>
forceinline constexpr Bitboard get_pawn_left_attacks(Bitboard pawns)
{
    validate_color<color>();
    pawns &= pawns_that_can_attack_left;
    if constexpr (color == WHITE)
    {
        return pawns << 9;
    }
    else
    {
        return pawns >> 7;
    }
}

template<Color color>
forceinline constexpr Bitboard get_pawn_left_attackers(const Bitboard attackers)
{
    validate_color<color>();
    if constexpr (color == WHITE)
    {
        return attackers >> 9;
    }
    else
    {
        return attackers << 7;
    }
}

template <Color color>
forceinline constexpr Bitboard get_pawn_right_attacks(Bitboard pawns)
{
    validate_color<color>();
    pawns &= pawns_that_can_attack_right;
    if constexpr (color == WHITE)
    {
        return pawns << 7;
    }
    else
    {
        return pawns >> 9;
    }
}

template<Color color>
forceinline constexpr Bitboard get_pawn_right_attackers(const Bitboard attackers)
{
    validate_color<color>();
    if constexpr (color == WHITE)
    {
        return attackers >> 7;
    }
    else
    {
        return attackers << 9;
    }
}

template<Color color>
forceinline constexpr Bitboard get_pawn_attacks(Bitboard pieces)
{
    validate_color<color>();
    return get_pawn_left_attacks<color>(pieces) | get_pawn_right_attacks<color>(pieces);
}

forceinline Bitboard get_knight_attacks(Bitboard pieces)
{
    Bitboard attacks = 0;
    while (pieces)
    {
        Bitboard piece = pop_bit(pieces);
        attacks |= knight_moves[bit_index(piece)];
    }
    return attacks;
}

forceinline Bitboard get_king_attacks(const Bitboard pieces)
{
    return king_moves[bit_index(pieces)];
}

forceinline Bitboard get_single_bishop_attacks(const Bitboard piece, const Bitboard occupied)
{
    const uint32_t index = bit_index(piece);
    const uint32_t offset = bishop_pext_table_offsets[index];
    const Bitboard xray_attacks = bishop_pext_xray_masks[index];
    const size_t pext_value = pext(occupied, xray_attacks);
    return bishop_pext_table[offset + pext_value];
}

forceinline Bitboard get_single_rook_attacks(const Bitboard piece, const Bitboard occupied)
{
    const uint32_t index = bit_index(piece);
    const uint32_t offset = rook_pext_table_offsets[index];
    const Bitboard xray_attacks = rook_pext_xray_masks[index];
    const size_t pext_value = pext(occupied, xray_attacks);
    return rook_pext_table[offset + pext_value];
}

forceinline Bitboard get_all_bishop_attacks(Bitboard pieces, const Bitboard occupied)
{
    Bitboard attacks = 0ULL;
    while (pieces)
    {
        Bitboard piece = pop_bit(pieces);
        attacks |= get_single_bishop_attacks(piece, occupied);
    }
    return attacks;
}

forceinline Bitboard get_all_rook_attacks(Bitboard pieces, const Bitboard occupied)
{
    Bitboard attacks = 0ULL;
    while (pieces)
    {
        Bitboard piece = pop_bit(pieces);
        attacks |= get_single_rook_attacks(piece, occupied);
    }
    return attacks;
}

forceinline Bitboard get_queen_attacks(Bitboard pieces, const Bitboard occupied)
{
    Bitboard attacks = 0ULL;
    while (pieces)
    {
        Bitboard piece = pop_bit(pieces);
        attacks |= get_single_bishop_attacks(piece, occupied);
        attacks |= get_single_rook_attacks(piece, occupied);
    }
    return attacks;
}

template<Color color>
forceinline Bitboard get_all_attacks(const Side& pieces, const Bitboard occupied)
{
    const auto pawn_attacks = get_pawn_attacks<color>(pieces.pawns);
    const auto knight_attacks = get_knight_attacks(pieces.knights);
    const auto bishop_attacks = get_all_bishop_attacks(pieces.bishops, occupied);
    const auto rook_attacks = get_all_rook_attacks(pieces.rooks, occupied);
    const auto queen_attacks = get_queen_attacks(pieces.queens, occupied);
    const auto king_attacks = get_king_attacks(pieces.king);
    return pawn_attacks | knight_attacks | bishop_attacks | rook_attacks | queen_attacks | king_attacks;
}