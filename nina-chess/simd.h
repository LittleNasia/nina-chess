#pragma once

#include "utils.h"
#include <immintrin.h>
#include <xmmintrin.h>

#ifdef __AVX512F__
using SimdVector = __m512;
#elifdef __AVX2__
using SimdVector = __m256;
#else
using SimdVector = __m128;
#endif

inline constexpr int FLOATS_PER_REGISTER = sizeof(SimdVector) / sizeof(float);

struct SimdPack
{
public:
	forceinline constexpr float& operator[] (const size_t index) { return m_Values[index]; }
	forceinline constexpr float operator[] (const size_t index) const { return m_Values[index]; }

private:
	float m_Values[FLOATS_PER_REGISTER];
};

#ifdef __AVX512F__
forceinline SimdVector SimdLoad(const float* address) { return _mm512_load_ps(address); }
forceinline SimdVector SimdSetZero() { return _mm512_setzero_ps(); }
forceinline SimdVector SimdMax(const SimdVector& a, const SimdVector& b) { return _mm512_max_ps(a, b); }
forceinline SimdVector SimdTanh(const SimdVector& input) { return _mm512_tanh_ps(input); }
forceinline SimdVector SimdFusedMultiplyAdd(const SimdVector& a, const SimdVector& b, const SimdVector& c) { return _mm512_fmadd_ps(a, b, c); }
#elifdef __AVX2__
forceinline SimdVector SimdLoad(const float* address) { return _mm256_load_ps(address); }
forceinline SimdVector SimdSetZero() { return _mm256_setzero_ps(); }
forceinline SimdVector SimdMax(const SimdVector& a, const SimdVector& b) { return _mm256_max_ps(a, b); }
forceinline SimdVector SimdTanh(const SimdVector& input) { return _mm256_tanh_ps(input); }
forceinline SimdVector SimdFusedMultiplyAdd(const SimdVector& a, const SimdVector& b, const SimdVector& c) { return _mm256_fmadd_ps(a, b, c); }
#else
forceinline SimdVector SimdLoad(const float* address) { return _mm_load_ps(address); }
forceinline SimdVector SimdSetZero() { return _mm_setzero_ps(); }
forceinline SimdVector SimdMax(const SimdVector& a, const SimdVector& b) { return _mm_max_ps(a, b); }
forceinline SimdVector SimdTanh(const SimdVector& input) { return _mm_tanh_ps(input); }
forceinline SimdVector SimdFusedMultiplyAdd(const SimdVector& a, const SimdVector& b, const SimdVector& c) { return _mm_add_ps(_mm_mul_ps(a, b), c); }
#endif
