#pragma once
#include "utils.h"

forceinline size_t pext(const Bitboard b, const Bitboard mask)
{
	return _pext_u64(b, mask);
}

#pragma warning( push )
#pragma warning( disable:4244) 
forceinline uint32_t popcnt(const Bitboard bb)
{
	return __popcnt64(bb);
}
#pragma warning( pop ) 

forceinline Bitboard pop_bit(Bitboard& bb)
{
	uint64_t lsb = _blsi_u64(bb);
	bb ^= lsb;
	return lsb;
}

#pragma warning( push )
#pragma warning( disable:4244 )
forceinline uint32_t bit_index(const Bitboard bb)
{
	return _tzcnt_u64(bb);
}
#pragma warning( pop ) 