#pragma once
#include "Core/Engine/utils.h"
#include <cstdint>
#include <immintrin.h>
#include <intrin0.inl.h>

forceinline size_t Pext(const Bitboard src, Bitboard mask);
forceinline uint32_t Popcnt(const Bitboard bb);
forceinline uint32_t Tzcnt(const Bitboard bb);
forceinline Bitboard Blsi(const Bitboard bb);


forceinline size_t Pext(const Bitboard src, Bitboard mask)
{
#ifdef __AVX2__
	return _pext_u64(src, mask);
#else
	Bitboard result = 0;
	for (Bitboard resultBit = 1; mask; resultBit += resultBit)
	{
		const Bitboard lowestMaskBit = Blsi(mask);
		if (src & lowestMaskBit)
			result |= resultBit;
		mask ^= lowestMaskBit;
	}
	return result;
#endif
}

#pragma warning( push )
#pragma warning( disable:4244) 
forceinline uint32_t Popcnt(const Bitboard bb)
{
	return __popcnt64(bb);
}
#pragma warning( pop )

#pragma warning( push )
#pragma warning( disable:4244)
forceinline uint32_t Tzcnt(const Bitboard bb)
{
#ifdef __AVX2__
	return _tzcnt_u64(bb);
#else
	if (bb == 0) return 64;
	unsigned long index;
	_BitScanForward64(&index, bb);
	return index;
#endif
}
#pragma warning( pop )

forceinline Bitboard Blsi(const Bitboard bb)
{
#ifdef __AVX2__
	return _blsi_u64(bb);
#else
	return bb & (0 - bb);
#endif
}
