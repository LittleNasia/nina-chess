#pragma once
#include "utils.h"

forceinline size_t Pext(const Bitboard b, const Bitboard mask);
forceinline uint32_t Popcnt(const Bitboard bb);
forceinline uint32_t Tzcnt(const Bitboard bb);
forceinline Bitboard Blsi(const Bitboard bb);


forceinline size_t Pext(const Bitboard b, const Bitboard mask)
{
	return _pext_u64(b, mask);
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
	return _tzcnt_u64(bb);
}
#pragma warning( pop )

forceinline Bitboard Blsi(const Bitboard bb)
{
	return _blsi_u64(bb);
}
