#include "accumulator.h"

#include <random>

#include "weights.h"

template<typename BitboardFeatureIterator, size_t output_size>
forceinline constexpr void BitboardFeatureAccumulator<BitboardFeatureIterator, output_size>::Weights::SetWeights(std::ifstream& weights_file)
{
	if (!weights_file.is_open())
	{
		throw std::runtime_error("weights file is not open");
	}
	constexpr size_t weights_size = BitboardFeatureIterator::NumBitboardFeatures() * bits_in_bitboard;
	float weights_from_file[weights_size];
	float bias_from_file = 0;

	// for now we're hardcoding the weights so we don't give a little pik about file reading
	std::memset(weights_from_file, 0, weights_size * sizeof(float));
	std::memcpy(weights_from_file, hardcoded_psqt_weights, sizeof(hardcoded_psqt_weights));

	for (auto& value : weights_from_file)
	{
		std::random_device rd;
		std::mt19937 gen(rd());
		std::uniform_real_distribution<float> dis(value * -0.1f, value * 0.1f);

		value += dis(gen);
	}

	std::memcpy(this->weights, weights_from_file, sizeof(this->weights));
	std::memcpy(this->bias, &bias_from_file, sizeof(this->bias));
}

template<typename BitboardFeatureIterator, size_t output_size>
forceinline constexpr void BitboardFeatureAccumulator<BitboardFeatureIterator, output_size>::AccumulateFeatures(const BitboardFeatureIterator& new_features_iterator, const BitboardFeatureIterator& old_features_iterator, const float* previous_accumulator_output)
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
			const size_t weights_index = getWeightsIndex(bitboard_index, feature_index);

			for (size_t output_index = 0; output_index < output_size; output_index++)
			{
				output[output_index] += weights->weights[weights_index][output_index];
			}
		}

		// subtract weights of removed features
		while (removed_features)
		{
			const uint32_t feature_index = bit_index(pop_bit(removed_features));
			const size_t weights_index = getWeightsIndex(bitboard_index, feature_index);

			for (size_t output_index = 0; output_index < output_size; output_index++)
			{
				output[output_index] -= weights->weights[weights_index][output_index];
			}
		}
	}

	validateOutput(new_features_iterator);
}

template<typename BitboardFeatureIterator, size_t output_size>
forceinline constexpr void BitboardFeatureAccumulator<BitboardFeatureIterator, output_size>::Reset(const BitboardFeatureIterator& new_features_iterator)
{
	std::memcpy(output, weights->bias, sizeof(output));

	for (size_t bitboard_index = 0; bitboard_index < BitboardFeatureIterator::NumBitboardFeatures(); bitboard_index++)
	{
		Bitboard new_features = new_features_iterator.Get(bitboard_index);

		while (new_features)
		{
			const uint32_t feature_index = bit_index(pop_bit(new_features));
			const size_t weights_index = getWeightsIndex(bitboard_index, feature_index);

			for (size_t output_index = 0; output_index < output_size; output_index++)
			{
				output[output_index] += weights->weights[weights_index][output_index];
			}
		}
	}
}

template<typename BitboardFeatureIterator, size_t output_size>
forceinline constexpr size_t BitboardFeatureAccumulator<BitboardFeatureIterator, output_size>::getWeightsIndex(const size_t bitboard_index, const uint32_t feature_in_bitboard_index)
{
	const size_t weights_index = bitboard_index * bits_in_bitboard + feature_in_bitboard_index;
	DEBUG_ASSERT(weights_index < BitboardFeatureIterator::NumBitboardFeatures() * bits_in_bitboard);
	return weights_index;
}

template<typename BitboardFeatureIterator, size_t output_size>
forceinline constexpr void BitboardFeatureAccumulator<BitboardFeatureIterator, output_size>::validateOutput(const BitboardFeatureIterator& new_features_iterator)
{
	DEBUG_IF(true)
	{
		float output_copy[output_size];
		std::memcpy(output_copy, output, sizeof(output_copy));
		Reset(new_features_iterator);
		for (int output_index = 0; output_index < output_size; output_index++)
		{
			const float val = output[output_index];
			const float val_copy = output_copy[output_index];
			const float diff = val - val_copy;
			if (std::fabs(diff) > 0.001)
			{
				throw std::runtime_error("accumulator is boboken :C");
			}
		}
		std::memcpy(output, output_copy, sizeof(output_copy));
	}
}
