#pragma once
#include "utils.h"

#include "simd.h"

#ifdef __AVX2__
forceinline float simd_horizontal_sum(const SimdVector& input)
{
	// input = [x0,x1,x2,x3,x4,x5,x6,x7]
	// extractf imm8 argument is either 1 or 0
	// 0 extracts the lower half, 1 extracts the upper half
	// lower_half = [x0,x1,x2,x3]
	// upper_half = [x4,x5,x6,x7]
	__m128 lowerHalf = _mm256_extractf128_ps(input, 0);
	__m128 higherHalf = _mm256_extractf128_ps(input, 1);

	// higher_half = [x0+x4, x1+x5, x2+x6, x3+x7]
	higherHalf = _mm_add_ps(higherHalf, lowerHalf);

	// lower_half = [x2+x6,x3+x7 ...] 
	lowerHalf = _mm_movehl_ps(higherHalf, higherHalf);

	// we do not care about the third and fourth value in the lower half currently
	// we only care that the first two values in each vector get added to each other
	// higher_half = [x2+x6+x0+x4,x1+x5+x3+x7 ...]
	higherHalf = _mm_add_ps(lowerHalf, higherHalf);

	// now we need to add the first and second element in the higher half 
	// the idea is to move the second element of higher half
	// to first element of lower half
	// lower_half = [x1+x5+x3+x7 ...]
	lowerHalf = _mm_shuffle_ps(higherHalf, higherHalf, _MM_SHUFFLE(0, 0, 0, 1));

	// now we can just add the very first elements using a simple scalar add
	higherHalf = _mm_add_ss(higherHalf, lowerHalf);

	// return the scalar value 
	return _mm_cvtss_f32(higherHalf);
}
#endif

#ifndef __AVX2__
forceinline float simd_horizontal_sum(const SimdVector& input)
{
	// input = [x0,x1,x2,x3]
	
	// shuffled = [x1,x1,x3,x3]
	__m128 shuffled = _mm_movehdup_ps(input);

	// sum = [x0+x1,x1+x1,x2+x3,x3+x3]
	__m128 sum = _mm_add_ps(input, shuffled);

	// sum now has both the x0 + x1 and x2+x3 values
	// we need to put x2 + x3 at index 0, we don't care about anything else
	// shuffled = [x2+x3,...]
	shuffled = _mm_shuffle_ps(sum, sum, _MM_SHUFFLE(0, 0, 0, 2));
	
	// sum = [x0+x1+x2+x3,...]
	sum = _mm_add_ss(sum, shuffled);

	return _mm_cvtss_f32(sum);
}
#endif