#pragma once
#include <atomic>
#include <chrono>
#include <cstdint>
#include <fstream>
#include <intrin.h>
#include <iostream>
#include <limits>
#include <memory>
#include <ostream>
#include <random>
#include <sstream>
#include <stdexcept>
#include <string_view>

using Bitboard = std::uint64_t;

#ifdef _DEBUG
inline constexpr bool IS_DEBUG = true;
#else
inline constexpr bool IS_DEBUG = ADD_DEBUG_CODE;
#endif

#define DEBUG_IF(x) if constexpr(IS_DEBUG) if (x)
#define DEBUG_ASSERT(x) DEBUG_IF(!(x)) throw std::runtime_error("Assertion failed: " #x)

// TODO determine which functions should be forceinlined and which shouldnt
// forceinlining everything doesn't seem to give performance benefits anymore
// maybe not anymore, forceinlining everything seems to be the way to go now for some reason ? ? ?
// there are functions that probably still shouldn't be inlined but forceinline as a default seems fine now
#define forceinline __forceinline

forceinline constexpr uint32_t FastModulo(const size_t input, const size_t ceil)
{
	return ((input >> 32) * (ceil)) >> 32;
}
