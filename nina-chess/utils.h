#pragma once
#include <cstdint>
#include <intrin.h>
#include <ostream>
#include <stdexcept>

using Bitboard = std::uint64_t;

#ifdef _DEBUG
inline constexpr bool is_debug = true;
#else
inline constexpr bool is_debug = false;
#endif

#define DEBUG_IF(x) if constexpr(is_debug) if (x)
#define DEBUG_ASSERT(x) DEBUG_IF(!(x)) throw std::runtime_error("Assertion failed: " #x)

// TODO determine which functions should be forceinlined and which shouldnt
// forceinlining everything doesn't seem to give performance benefits anymore
// maybe not anymore, forceinlining everything seems to be the way to go now for some reason ? ? ?
// there are functions that probably still shouldn't be inlined but forceinline as a default seems fine now
#define forceinline inline

forceinline constexpr uint32_t fast_modulo(const size_t input, const size_t ceil)
{
	return ((input >> 32) * (ceil)) >> 32;
}