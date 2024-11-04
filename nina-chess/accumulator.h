#pragma once
#include "utils.h"

#include "architecture.h"

template<typename BitboardFeatureIterator, size_t outputSize>
class BitboardFeatureAccumulator
{
public:
	inline static constexpr size_t BITS_IN_BITBOARD = 64;
	struct Weights
	{
		alignas(CACHE_LINE_SIZE) float Weights[BitboardFeatureIterator::NumBitboardFeatures() * BITS_IN_BITBOARD][outputSize];
		alignas(CACHE_LINE_SIZE) float Bias[outputSize];

		forceinline constexpr void SetWeights(std::ifstream& weightsFile);
	};

	BitboardFeatureAccumulator() = default;

	forceinline constexpr void AccumulateFeatures(const BitboardFeatureIterator& newFeaturesIterator, const BitboardFeatureIterator& oldFeaturesIterator, const float* previousAccumulatorOutput);
	forceinline constexpr void Reset(const BitboardFeatureIterator& newFeaturesIterator);

	forceinline constexpr const float* GetOutput() const { return m_Output; } 
	forceinline constexpr void SetWeights(Weights& weightsToSet) { m_Weights = &weightsToSet; }

private:
	forceinline constexpr size_t getWeightsIndex(const size_t bitboard_index, const uint32_t feature_in_bitboard_index);

	forceinline constexpr void validateOutput(const BitboardFeatureIterator& new_features_iterator);

	const Weights* m_Weights;
	alignas(CACHE_LINE_SIZE) float m_Output[outputSize];
};

#include "accumulator.inl"