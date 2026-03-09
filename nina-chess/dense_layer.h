#pragma once

#include "activation_function.h"
#include "architecture.h"
#include "avx_utils.h"
#include "rng.h"
#include "simd.h"
#include <cmath>
#include <cstring>
#include <fstream>
#include <memory>
#include <random>

template<int inputNeurons, int outputNeurons, ActivationFunction activationFunction = ActivationFunction::RELU>
class DenseLayer
{
public:
	static constexpr int NUM_INPUT_PACKS = inputNeurons / FLOATS_PER_REGISTER;
	// AVX2 and AVX512 implementation parses a few input vectors at once
	// 8 is a number found through trial and error on my hardware to be correct
	static constexpr size_t INPUT_VECTORS_PARSED = 8;

	alignas(CACHE_LINE_SIZE) SimdPack Weights[NUM_INPUT_PACKS][outputNeurons];
	alignas(CACHE_LINE_SIZE) float Biases[outputNeurons];
	alignas(CACHE_LINE_SIZE) float Output[outputNeurons];

	DenseLayer()
	{
		static_assert((inputNeurons % FLOATS_PER_REGISTER) == 0,
			"Incorrect dense layer input size, must be a multiple of FLOATS_PER_REGISTER (4/8/16 depending on SSE/AVX2/AVX512 architecture)");
	}

	forceinline void ReadWeights(std::ifstream& weightsFile)
	{
		auto weightsFromFile = std::make_unique<float[]>(inputNeurons * outputNeurons);
		weightsFile.read((char*)weightsFromFile.get(), inputNeurons * outputNeurons * sizeof(float));
		for (int inputNeuronIndex = 0; inputNeuronIndex < inputNeurons; inputNeuronIndex++)
		{
			for (int outputNeuronIndex = 0; outputNeuronIndex < outputNeurons; outputNeuronIndex++)
			{
				const int inputPack = inputNeuronIndex / FLOATS_PER_REGISTER;
				const int inputInPack = inputNeuronIndex % FLOATS_PER_REGISTER;

				Weights[inputPack][outputNeuronIndex][inputInPack] =
					weightsFromFile[inputNeuronIndex * outputNeurons + outputNeuronIndex];
			}
		}
		weightsFile.read((char*)Biases, sizeof(Biases));
	}

	forceinline void InitializeWeights();

	forceinline float* Forward(const float* input)
	{
		// no matter what, the output is biases + input * weights
		// input * weights get calculated later depending on architecture
		// but this step is always identical
		std::memcpy(Output, Biases, sizeof(Output));
#ifdef __AVX2__
		return forwardAvx2(input);
#else
		if constexpr (outputNeurons == 1)
		{
			return forwardSingle(input);
		}
		else
		{
			return forwardMulti(input);
		}
#endif
	}

private:
#ifdef __AVX2__
	forceinline float* forwardAvx2(const float* input)
	{
		static constexpr int actualVectorsParsedCount = ((NUM_INPUT_PACKS < INPUT_VECTORS_PARSED) ? NUM_INPUT_PACKS : INPUT_VECTORS_PARSED);
		static constexpr size_t numOuterIterations = NUM_INPUT_PACKS / actualVectorsParsedCount;

		int baseInputIndex = 0;
		for (int outerIteration = 0; outerIteration < numOuterIterations; outerIteration++,
			baseInputIndex += FLOATS_PER_REGISTER * actualVectorsParsedCount)
		{
			SimdVector inputs[actualVectorsParsedCount];
			for (int inputVector = 0, inputIndex = baseInputIndex; inputVector < actualVectorsParsedCount;
				inputVector++, inputIndex += FLOATS_PER_REGISTER)
			{
				inputs[inputVector] = _mm256_load_ps(&input[inputIndex]);

				if constexpr (activationFunction == ActivationFunction::RELU)
				{
					inputs[inputVector] = _mm256_max_ps(inputs[inputVector], _mm256_setzero_ps());
				}
				else if constexpr (activationFunction == ActivationFunction::TANH)
				{
					inputs[inputVector] = _mm256_tanh_ps(inputs[inputVector]);
				}
			}

			for (int outputNode = 0; outputNode < outputNeurons; outputNode++)
			{
				SimdVector outputSum = _mm256_setzero_ps();
				for (int inputVector = 0; inputVector < actualVectorsParsedCount; inputVector++)
				{
					outputSum = _mm256_fmadd_ps(inputs[inputVector],
						_mm256_load_ps((const float*)&Weights[outerIteration * actualVectorsParsedCount + inputVector][outputNode]),
						outputSum);
				}
				// horizontal sum
				// although doing horizontal sums in the innermost loop is not really good
				// the important thing to note is that the iterations of the outermost loop
				// are divided by the num_input_vectors
				// that means the innermost loop executes way less often
				// this outperforms doing it in the outer loop
				// due to the memory accesses we save by reversing the loops
				// I benchmarked that I promise!!!!! this is really better !!!!!! (on my machine)

				Output[outputNode] += simd_horizontal_sum(outputSum);
			}
		}
		return Output;
	}
#else
	// in case the output layer has at least 4 neurons, we can optimize in a neat little way
	forceinline float* forwardMulti(const float* input)
	{
		for (int inputPack = 0, inputNeuronIndex = 0; inputPack < NUM_INPUT_PACKS; inputPack++, inputNeuronIndex += 4)
		{
			// load 4 inputs into one register
			__m128 currInputPack = _mm_load_ps(input + inputNeuronIndex);
			// apply activation function on input first
			if constexpr (activationFunction == ActivationFunction::RELU)
			{
				currInputPack = _mm_max_ps(currInputPack, _mm_setzero_ps());
			}
			else if constexpr (activationFunction == ActivationFunction::TANH)
			{
				currInputPack = _mm_tanh_ps(currInputPack);
			}

			// 4 output values are calculated in one iteration
			static constexpr int numOutputPacks = outputNeurons / 4;
			for (int outputPack = 0, outputNeuronIndex = 0; outputPack < numOutputPacks; outputPack++, outputNeuronIndex += 4)
			{
				// weights for each of the packs
				__m128 weightPacks[4];
				for (int weightPackIndex = 0; weightPackIndex < 4; weightPackIndex++)
				{
					// load the weights
					weightPacks[weightPackIndex] = _mm_load_ps((float*)&Weights[inputPack][outputNeuronIndex + weightPackIndex]);
					// multiply each of the weights by the input
					weightPacks[weightPackIndex] = _mm_mul_ps(weightPacks[weightPackIndex], currInputPack);
				}

				// sum the weights corresponding to the first two output neurons
				__m128 firstTwoSums = _mm_hadd_ps(weightPacks[0], weightPacks[1]);
				firstTwoSums = _mm_hadd_ps(firstTwoSums, firstTwoSums);

				// identical logic for the next two thingies
				__m128 nextTwoSums = _mm_hadd_ps(weightPacks[2], weightPacks[3]);
				nextTwoSums = _mm_hadd_ps(nextTwoSums, nextTwoSums);

				// now the first vector has sums of input*weight for the first two output neurons
				// and the second vector has just that for the next two output neurons
				// if 1 is the value we need to add to the first output neuron, 2 to the second etc.
				// then the vectors look as follows:
				// firstTwoSums = [ 1, 2, 1, 2]
				// nextTwoSums = [ 3, 4, 3, 4]
				// we want a vector in form of:
				// [1, 2, 3, 4]
				// to which we then add current values in output, and store it where it needs to be

				nextTwoSums = _mm_movehl_ps(nextTwoSums, firstTwoSums);

				// now we need to add the values to the ones currently present in output
				__m128 currOutputPart = _mm_load_ps(Output + outputNeuronIndex);
				nextTwoSums = _mm_add_ps(nextTwoSums, currOutputPart);

				// store the result
				_mm_store_ps(Output + outputNeuronIndex, nextTwoSums);
			}
		}
		return Output;
	}

	// in case the output layer has only 1 neuron, we can't optimize it in that neat little way
	forceinline float* forwardSingle(const float* input)
	{
		// there is only one output neuron
		constexpr int outputNeuronIndex = 0;
		float& outputValue = Output[outputNeuronIndex];

		// we load two input packs so that two additions at once can be performed using haddps
		for (int inputPack = 0, inputNeuronIndex = 0; inputPack < NUM_INPUT_PACKS; inputPack += 2, inputNeuronIndex += 8)
		{
			// load 4 inputs into one register twice
			__m128 firstInputPack = _mm_load_ps(input + inputNeuronIndex);
			__m128 secondInputPack = _mm_load_ps(input + inputNeuronIndex + 4);

			// apply activation function on input first
			if constexpr (activationFunction == ActivationFunction::RELU)
			{
				firstInputPack = _mm_max_ps(firstInputPack, _mm_setzero_ps());
				secondInputPack = _mm_max_ps(secondInputPack, _mm_setzero_ps());
			}
			else if constexpr (activationFunction == ActivationFunction::TANH)
			{
				firstInputPack = _mm_tanh_ps(firstInputPack);
				secondInputPack = _mm_tanh_ps(secondInputPack);
			}

			// load weights of these packs
			__m128 firstInputPackWeights = _mm_load_ps((float*)&Weights[inputPack][outputNeuronIndex]);
			__m128 secondInputPackWeights = _mm_load_ps((float*)&Weights[inputPack + 1][outputNeuronIndex]);

			firstInputPackWeights = _mm_mul_ps(firstInputPack, firstInputPackWeights);
			secondInputPackWeights = _mm_mul_ps(secondInputPack, secondInputPackWeights);

			// sum all the values
			firstInputPackWeights = _mm_hadd_ps(firstInputPackWeights, secondInputPackWeights);
			firstInputPackWeights = _mm_hadd_ps(firstInputPackWeights, firstInputPackWeights);
			firstInputPackWeights = _mm_hadd_ps(firstInputPackWeights, firstInputPackWeights);

			outputValue += _mm_cvtss_f32(firstInputPackWeights);
		}
		return Output;
	}
#endif
};

template<int inputNeurons, int outputNeurons, ActivationFunction activationFunction>
forceinline void DenseLayer<inputNeurons, outputNeurons, activationFunction>::InitializeWeights()
{
	Xorshift64 prng(0xBEEF);
	const float bound = 1.f / std::sqrt(static_cast<float>(inputNeurons));
	std::uniform_real_distribution<float> weightDist(-bound, bound);
	std::memset(Biases, 0, sizeof(Biases));
	for (int inputNeuronIndex = 0; inputNeuronIndex < inputNeurons; inputNeuronIndex++)
	{
		const int currInputPack = inputNeuronIndex / FLOATS_PER_REGISTER;
		const int neuronInPackIndex = inputNeuronIndex % FLOATS_PER_REGISTER;
		for (int outputNeuronIndex = 0; outputNeuronIndex < outputNeurons; outputNeuronIndex++)
		{
			float randomVal = weightDist(prng);
			Weights[currInputPack][outputNeuronIndex][neuronInPackIndex] = randomVal;
		}
	}
}
