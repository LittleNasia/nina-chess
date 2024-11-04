#pragma once
#include "utils.h"

#include "bit_manip.h"
#include "bitmasks.h"
#include "side.h"

template <Color color>
forceinline constexpr Bitboard GetPawnAdvances(const Bitboard pawns)
{
	ValidateColor<color>();
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
forceinline constexpr Bitboard GetDoubleAdvances(const Bitboard pawns)
{
    ValidateColor<color>();
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
forceinline constexpr Bitboard GetDoubleAdvancesCandidates()
{
    ValidateColor<color>();
    if constexpr (color == WHITE)
    {
        return ROW_BITMASKS[2];
    }
    else
    {
        return ROW_BITMASKS[5];
    }
}

template <Color color>
forceinline constexpr Bitboard GetPawnsLeftAttacks(Bitboard pawns)
{
    ValidateColor<color>();
    pawns &= PAWNS_THAT_CAN_ATTACK_LEFT;
    if constexpr (color == WHITE)
    {
        return pawns << 9;
    }
    else
    {
        return pawns >> 7;
    }
}

template <Color color>
forceinline constexpr Bitboard GetPawnsRightAttacks(Bitboard pawns)
{
    ValidateColor<color>();
    pawns &= PAWNS_THAT_CAN_ATTACK_RIGHT;
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
forceinline constexpr Bitboard GetAllPawnAttacks(Bitboard pieces)
{
    ValidateColor<color>();
    return GetPawnsLeftAttacks<color>(pieces) | GetPawnsRightAttacks<color>(pieces);
}

forceinline Bitboard GetAllKnightAttacks(Bitboard knights)
{
    Bitboard attacks = 0;
    while (knights)
    {
        Bitboard knightBitmask = PopBit(knights);
        attacks |= KNIGHT_MOVE_BITMASKS[BitIndex(knightBitmask)];
    }
    return attacks;
}

forceinline Bitboard GetKingAttacks(const Bitboard kingBitmask)
{
    return KING_MOVE_BITMASKS[BitIndex(kingBitmask)];
}

forceinline Bitboard GetSingleBishopAttacks(const Bitboard bishopBitmask, const Bitboard occupiedBitmask)
{
    DEBUG_ASSERT(Popcnt(bishopBitmask) == 1);

    const uint32_t index = BitIndex(bishopBitmask);
    const uint32_t offset = BISHOP_PEXT_TABLE_OFFSETS[index];
    const Bitboard xrayAttacks = BISHOP_PEXT_XRAY_BITMASKS[index];
    const size_t pextValue = Pext(occupiedBitmask, xrayAttacks);
    return BISHOP_PEXT_TABLE[offset + pextValue];
}

forceinline Bitboard GetSingleRookAttacks(const Bitboard rookBitmask, const Bitboard occupiedBitmask)
{
    DEBUG_ASSERT(Popcnt(rookBitmask) == 1);

    const uint32_t index = BitIndex(rookBitmask);
    const uint32_t offset = ROOK_PEXT_TABLE_OFFSETS[index];
    const Bitboard xrayAttacks = ROOK_PEXT_XRAY_BITMASKS[index];
    const size_t pextValue = Pext(occupiedBitmask, xrayAttacks);
    return ROOK_PEXT_TABLE[offset + pextValue];
}

forceinline Bitboard GetAllBishopAttacks(Bitboard bishopsBitmask, const Bitboard occupiedBitmask)
{
    Bitboard attacks = 0ULL;
    while (bishopsBitmask)
    {
        Bitboard pieceBitmask = PopBit(bishopsBitmask);
        attacks |= GetSingleBishopAttacks(pieceBitmask, occupiedBitmask);
    }
    return attacks;
}

forceinline Bitboard GetAllRookAttacks(Bitboard rooksBitmask, const Bitboard occupiedBitmask)
{
    Bitboard attacks = 0ULL;
    while (rooksBitmask)
    {
        Bitboard pieceBitmask = PopBit(rooksBitmask);
        attacks |= GetSingleRookAttacks(pieceBitmask, occupiedBitmask);
    }
    return attacks;
}

forceinline Bitboard GetAllQueenAttacks(Bitboard queensBitmask, const Bitboard occupiedBitmask)
{
    Bitboard attacks = 0ULL;
    while (queensBitmask)
    {
        Bitboard pieceBitmask = PopBit(queensBitmask);
        attacks |= GetSingleBishopAttacks(pieceBitmask, occupiedBitmask);
        attacks |= GetSingleRookAttacks(pieceBitmask, occupiedBitmask);
    }
    return attacks;
}

template<Color color>
forceinline Bitboard GetAllAttacks(const Side& pieces, const Bitboard occupiedBitmask)
{
    const auto pawnAttacks = GetAllPawnAttacks<color>(pieces.Pawns);
    const auto knightAttacks = GetAllKnightAttacks(pieces.Knights);
    const auto bishopAttacks = GetAllBishopAttacks(pieces.Bishops, occupiedBitmask);
    const auto rookAttacks = GetAllRookAttacks(pieces.Rooks, occupiedBitmask);
    const auto queenAttacks = GetAllQueenAttacks(pieces.Queens, occupiedBitmask);
    const auto kingAttacks = GetKingAttacks(pieces.King);
    return pawnAttacks | knightAttacks | bishopAttacks | rookAttacks | queenAttacks | kingAttacks;
}