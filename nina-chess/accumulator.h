#pragma once
#include "utils.h"

#include "architecture.h"

template<typename BitboardFeatureIterator, size_t output_size>
class BitboardFeatureAccumulator
{
public:
	inline static constexpr size_t bits_in_bitboard = 64;
	struct Weights
	{
		alignas(cache_line_size) float weights[BitboardFeatureIterator::NumBitboardFeatures() * bits_in_bitboard][output_size];
		alignas(cache_line_size) float bias[output_size];

		forceinline constexpr void SetWeights(std::ifstream& weights_file);
	};

	BitboardFeatureAccumulator() = default;

	forceinline constexpr void AccumulateFeatures(const BitboardFeatureIterator& new_features_iterator, const BitboardFeatureIterator& old_features_iterator, const float* previous_accumulator_output);
	forceinline constexpr void Reset(const BitboardFeatureIterator& new_features_iterator);

	forceinline constexpr const float* GetOutput() const { return output; } 
	forceinline constexpr void SetWeights(Weights& weights) { this->weights = &weights; }

private:
	forceinline constexpr size_t getWeightsIndex(const size_t bitboard_index, const uint32_t feature_in_bitboard_index);

	forceinline constexpr void validateOutput(const BitboardFeatureIterator& new_features_iterator);

	const Weights* weights;
	alignas(cache_line_size) float output[output_size];
};

#include "accumulator.inl"


