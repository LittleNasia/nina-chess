#pragma once
#include "utils.h"

#include "weights.h"

template<typename BitboardFeatureIterator, size_t output_size>
class BitboardFeatureAccumulator
{
public:
	inline static constexpr size_t bits_in_bitboard = 64;
	struct Weights
	{
		alignas(cache_line_size) float weights[BitboardFeatureIterator::NumBitboardFeatures() * bits_in_bitboard][output_size];
		alignas(cache_line_size) float bias[output_size];

		forceinline constexpr void SetWeights(std::ifstream& weights_file)
		{
			constexpr size_t weights_size = BitboardFeatureIterator::NumBitboardFeatures() * bits_in_bitboard;
			float* weights = new float[weights_size];
			float bias = 0;

			// for now we're hardcoding the weights so we don't give a little pik about file reading
			std::memset(weights, 0, weights_size * sizeof(float));
			std::memcpy(weights, hardcoded_psqt_weights, sizeof(hardcoded_psqt_weights));

			std::memcpy(this->weights, weights, sizeof(this->weights));
			std::memcpy(this->bias, &bias, sizeof(this->bias));

			delete[] weights;
		}
	};

	BitboardFeatureAccumulator() = default;

	forceinline constexpr void AccumulateFeatures(const BitboardFeatureIterator& new_features_iterator, const BitboardFeatureIterator& old_features_iterator, const float* previous_accumulator_output)
	{
		std::memcpy(output, previous_accumulator_output, sizeof(output));

		for (size_t bitboard_index = 0; bitboard_index < BitboardFeatureIterator::NumBitboardFeatures(); bitboard_index++)
		{
			const Bitboard new_features = new_features_iterator.Get(bitboard_index);
			const Bitboard old_features = old_features_iterator.Get(bitboard_index);

			Bitboard added_features = new_features & ~old_features;
			Bitboard removed_features = ~new_features & old_features;

			// add weights of added features
			while (added_features)
			{
				const uint32_t feature_index = bit_index(pop_bit(added_features));
				const size_t weights_index = GetWeightsIndex(bitboard_index, feature_index);

				for (size_t output_index = 0; output_index < output_size; output_index++)
				{
					output[output_index] += weights->weights[weights_index][output_index];
				}
			}

			// subtract weights of removed features
			while (removed_features)
			{
				const uint32_t feature_index = bit_index(pop_bit(removed_features));
				const size_t weights_index = GetWeightsIndex(bitboard_index, feature_index);

				for (size_t output_index = 0; output_index < output_size; output_index++)
				{
					output[output_index] -= weights->weights[weights_index][output_index];
				}
			}
		}
	}

	forceinline constexpr void Reset(const BitboardFeatureIterator& new_features_iterator)
	{
		std::memcpy(output, weights->bias, sizeof(output));

		for (size_t bitboard_index = 0; bitboard_index < BitboardFeatureIterator::NumBitboardFeatures(); bitboard_index++)
		{
			Bitboard new_features = new_features_iterator.Get(bitboard_index);

			while (new_features)
			{
				const uint32_t feature_index = bit_index(pop_bit(new_features));
				const size_t weights_index = GetWeightsIndex(bitboard_index, feature_index);

				for (size_t output_index = 0; output_index < output_size; output_index++)
				{
					output[output_index] += weights->weights[weights_index][output_index];
				}
			}
		}
	}

	forceinline constexpr const float* GetOutput() const { return output; } 

	forceinline constexpr void SetWeights(Weights& weights)
	{
		this->weights = &weights;
	}

private:
	forceinline constexpr size_t GetWeightsIndex(const size_t bitboard_index, const uint32_t feature_in_bitboard_index)
	{
		return bitboard_index * bits_in_bitboard + feature_in_bitboard_index;
	}

	const Weights* weights;
	alignas(cache_line_size) float output[output_size];
};