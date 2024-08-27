#pragma once
#include "utils.h"

#include <immintrin.h>
#include <xmmintrin.h>

#ifdef __AVX2__
using simd_vector = __m256;
#else
using simd_vector = __m128;
#endif

inline constexpr int floats_per_register = sizeof(simd_vector) / sizeof(float);

struct simd_pack
{
public:
	forceinline constexpr float& operator[] (const size_t index) { return values[index]; }
	forceinline constexpr const float& operator[] (const size_t index) const { return values[index]; }
private:
	float values[floats_per_register];
};