#pragma once
#include "intrinsics.h"
#include "utils.h"

forceinline Bitboard pop_bit(Bitboard& bb)
{
	const uint64_t lsb = blsi(bb);
	bb ^= lsb;
	return lsb;
}

#pragma warning( push )
#pragma warning( disable:4244 )
forceinline uint32_t bit_index(const Bitboard bb)
{
	return tzcnt(bb);
}
#pragma warning( pop )

forceinline uint32_t pop_bit_and_get_index(Bitboard& bb)
{
	const uint64_t lsb = pop_bit(bb);
	return bit_index(lsb);
}