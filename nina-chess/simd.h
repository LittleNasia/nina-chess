#pragma once
#include "utils.h"

#include <immintrin.h>
#include <xmmintrin.h>

#ifdef __AVX2__
using SimdVector = __m256;
#else
using SimdVector = __m128;
#endif

inline constexpr int FLOATS_PER_REGISTER = sizeof(SimdVector) / sizeof(float);

struct simd_pack
{
public:
	forceinline constexpr float& operator[] (const size_t index) { return m_Values[index]; }
	forceinline constexpr float operator[] (const size_t index) const { return m_Values[index]; }
private:
	float m_Values[FLOATS_PER_REGISTER];
};