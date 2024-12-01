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

	template<size_t bitboardIndex>
	forceinline constexpr void AccumulateFeatures(const BitboardFeatureIterator& newFeaturesIterator, const BitboardFeatureIterator& oldFeaturesIterator, const float* previousAccumulatorOutput)
	{
		const Bitboard newFeatures = newFeaturesIterator.Get<bitboardIndex>(0.f);
		const Bitboard oldFeatures = oldFeaturesIterator.Get<bitboardIndex>(0.f);

		Bitboard addedFeatures = newFeatures & ~oldFeatures;
		Bitboard removedFeatures = ~newFeatures & oldFeatures;

		// add weights of added features
		while (addedFeatures)
		{
			const uint32_t featureIndex = PopBitAndGetIndex(addedFeatures);
			const size_t weightsIndex = getWeightsIndex(bitboardIndex, featureIndex);

			for (size_t outputIndex = 0; outputIndex < outputSize; outputIndex++)
			{
				m_Output[outputIndex] += m_Weights->Weights[weightsIndex][outputIndex];
			}
		}

		// subtract weights of removed features
		while (removedFeatures)
		{
			const uint32_t featureIndex = PopBitAndGetIndex(removedFeatures);
			const size_t weightsIndex = getWeightsIndex(bitboardIndex, featureIndex);

			for (size_t outputIndex = 0; outputIndex < outputSize; outputIndex++)
			{
				m_Output[outputIndex] -= m_Weights->Weights[weightsIndex][outputIndex];
			}
		}

		if constexpr (bitboardIndex + 1 < BitboardFeatureIterator::NumBitboardFeatures())
		{
			AccumulateFeatures<bitboardIndex + 1>(newFeaturesIterator, oldFeaturesIterator, previousAccumulatorOutput);
		}
	}

	forceinline constexpr void AccumulateFeatures(const BitboardFeatureIterator& newFeaturesIterator, const BitboardFeatureIterator& oldFeaturesIterator, const float* previousAccumulatorOutput)
	{
		std::memcpy(m_Output, previousAccumulatorOutput, sizeof(m_Output));

		AccumulateFeatures<0>(newFeaturesIterator, oldFeaturesIterator, previousAccumulatorOutput);

		validateOutput(newFeaturesIterator);
	}

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

