#pragma once
#include "intrinsics.h"
#include "utils.h"

forceinline Bitboard PopBit(Bitboard& bitboard);
forceinline uint32_t BitIndex(const Bitboard bitboard);
forceinline uint32_t PopBitAndGetIndex(Bitboard& bitboard);

template<Bitboard bitmask>
forceinline uint32_t consteval BitIndex();


forceinline Bitboard PopBit(Bitboard& bitboard)
{
	const uint64_t lsb = Blsi(bitboard);
	bitboard ^= lsb;
	return lsb;
}

#pragma warning( push )
#pragma warning( disable:4244 )
forceinline uint32_t BitIndex(const Bitboard bitboard)
{
	return Tzcnt(bitboard);
}
#pragma warning( pop )

forceinline uint32_t PopBitAndGetIndex(Bitboard& bitboard)
{
	const uint64_t lsb = PopBit(bitboard);
	return BitIndex(lsb);
}

template<Bitboard bitmask>
forceinline uint32_t consteval BitIndex()
{
	for (uint32_t bitIndex = 0; bitIndex < 64; ++bitIndex)
	{
		if (bitmask & (1ULL << bitIndex))
		{
			return bitIndex;
		}
	}
	return 64;
}
