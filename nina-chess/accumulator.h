#pragma once
#include "architecture.h"
#include "utils.h"
#include "weights.h"

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
	forceinline constexpr void AccumulateFeatures(const BitboardFeatureIterator& newFeaturesIterator, const BitboardFeatureIterator& oldFeaturesIterator, const float* previousAccumulatorOutput);

	forceinline constexpr void AccumulateFeatures(const BitboardFeatureIterator& newFeaturesIterator, const BitboardFeatureIterator& oldFeaturesIterator, const float* previousAccumulatorOutput);
	forceinline constexpr const float* GetOutput() const { return m_Output; }
	forceinline constexpr void Reset(const BitboardFeatureIterator& newFeaturesIterator);
	forceinline constexpr void SetWeights(Weights& weightsToSet) { m_Weights = &weightsToSet; }

private:
	forceinline constexpr size_t getWeightsIndex(const size_t bitboard_index, const uint32_t feature_in_bitboard_index);
	forceinline constexpr void validateOutput(const BitboardFeatureIterator& new_features_iterator);

	const Weights* m_Weights;
	alignas(CACHE_LINE_SIZE) float m_Output[outputSize];
};

template<typename BitboardFeatureIterator, size_t outputSize>
template<size_t bitboardIndex>
forceinline constexpr void BitboardFeatureAccumulator<BitboardFeatureIterator, outputSize>::AccumulateFeatures(const BitboardFeatureIterator& newFeaturesIterator, const BitboardFeatureIterator& oldFeaturesIterator, const float* previousAccumulatorOutput)
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

template<typename BitboardFeatureIterator, size_t outputSize>
forceinline constexpr void BitboardFeatureAccumulator<BitboardFeatureIterator, outputSize>::AccumulateFeatures(const BitboardFeatureIterator& newFeaturesIterator, const BitboardFeatureIterator& oldFeaturesIterator, const float* previousAccumulatorOutput)
{
	std::memcpy(m_Output, previousAccumulatorOutput, sizeof(m_Output));

	AccumulateFeatures<0>(newFeaturesIterator, oldFeaturesIterator, previousAccumulatorOutput);

	validateOutput(newFeaturesIterator);
}

template<typename BitboardFeatureIterator, size_t outputSize>
forceinline constexpr void BitboardFeatureAccumulator<BitboardFeatureIterator, outputSize>::Weights::SetWeights(std::ifstream& weightsFile)
{
	if (!weightsFile.is_open())
	{
		throw std::runtime_error("weights file is not open");
	}
	constexpr size_t weightsSize = BitboardFeatureIterator::NumBitboardFeatures() * BITS_IN_BITBOARD;
	float weightsFromFile[weightsSize];
	float biasFromFile = 0;

	// for now we're hardcoding the weights so we don't give a little pik about file reading
	std::memset(weightsFromFile, 0, weightsSize * sizeof(float));
	std::memcpy(weightsFromFile, HARDCODED_PSQT_WEIGHTS, sizeof(HARDCODED_PSQT_WEIGHTS));

	for (auto& value : weightsFromFile)
	{
		std::random_device rd;
		std::mt19937 gen(rd());
		std::uniform_real_distribution<float> dis(value * -0.1f, value * 0.1f);

		value += dis(gen);
	}

	std::memcpy(this->Weights, weightsFromFile, sizeof(this->Weights));
	std::memcpy(this->Bias, &biasFromFile, sizeof(this->Bias));
}

template<typename BitboardFeatureIterator, size_t outputSize>
forceinline constexpr void BitboardFeatureAccumulator<BitboardFeatureIterator, outputSize>::Reset(const BitboardFeatureIterator& newFeaturesIterator)
{
	std::memcpy(m_Output, m_Weights->Bias, sizeof(m_Output));

	for (size_t bitboardIndex = 0; bitboardIndex < BitboardFeatureIterator::NumBitboardFeatures(); bitboardIndex++)
	{
		Bitboard newFeatures = newFeaturesIterator.Get(bitboardIndex);

		while (newFeatures)
		{
			const uint32_t featureIndex = PopBitAndGetIndex(newFeatures);
			const size_t weightsIndex = getWeightsIndex(bitboardIndex, featureIndex);

			for (size_t outputIndex = 0; outputIndex < outputSize; outputIndex++)
			{
				m_Output[outputIndex] += m_Weights->Weights[weightsIndex][outputIndex];
			}
		}
	}
}

template<typename BitboardFeatureIterator, size_t outputSize>
forceinline constexpr size_t BitboardFeatureAccumulator<BitboardFeatureIterator, outputSize>::getWeightsIndex(const size_t bitboardIndex, const uint32_t featureInBitboardIndex)
{
	const size_t weightsIndex = bitboardIndex * BITS_IN_BITBOARD + featureInBitboardIndex;
	DEBUG_ASSERT(weightsIndex < BitboardFeatureIterator::NumBitboardFeatures() * BITS_IN_BITBOARD);
	return weightsIndex;
}

template<typename BitboardFeatureIterator, size_t outputSize>
forceinline constexpr void BitboardFeatureAccumulator<BitboardFeatureIterator, outputSize>::validateOutput(const BitboardFeatureIterator& newFeaturesIterator)
{
	DEBUG_IF(true)
	{
		float outputCopy[outputSize];
		std::memcpy(outputCopy, m_Output, sizeof(outputCopy));
		Reset(newFeaturesIterator);
		for (size_t outputIndex = 0; outputIndex < outputSize; outputIndex++)
		{
			const float val = m_Output[outputIndex];
			const float valCopy = outputCopy[outputIndex];
			const float diff = val - valCopy;
			if (std::fabs(diff) > 0.001)
			{
				throw std::runtime_error("accumulator is boboken :C");
			}
		}
		std::memcpy(m_Output, outputCopy, sizeof(outputCopy));
	}
}

