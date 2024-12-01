#include "accumulator.h"

#include <random>

#include "weights.h"

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
