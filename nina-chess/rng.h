#pragma once
#include <cstdint>
#include <limits>
#include "Core/Engine/utils.h"

struct Xorshift64
{
	using result_type = uint64_t;

	uint64_t State;

	forceinline constexpr Xorshift64(const uint64_t seed) : State(seed ? seed : 1) {}

	forceinline constexpr uint64_t operator()()
	{
		State ^= State << 13;
		State ^= State >> 7;
		State ^= State << 17;
		return State;
	}

	static constexpr uint64_t min() { return 1; }
	static constexpr uint64_t max() { return std::numeric_limits<uint64_t>::max(); }
};
