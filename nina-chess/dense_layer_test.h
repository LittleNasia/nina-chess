#pragma once

#include "activation_function.h"
#include "architecture.h"
#include "dense_layer.h"
#include "rng.h"
#include "simd.h"
#include <algorithm>
#include <cstring>
#include <iostream>
#include <random>

template<int inputNeurons, int outputNeurons, ActivationFunction activationFunction>
bool ScalarForward(DenseLayer<inputNeurons, outputNeurons, activationFunction>& layer,
	const float* input, float* scalarOutput)
{
	std::memcpy(scalarOutput, layer.Biases, outputNeurons * sizeof(float));

	for (size_t inputIndex = 0; inputIndex < inputNeurons; inputIndex++)
	{
		float activated;
		if constexpr (activationFunction == ActivationFunction::RELU)
			activated = std::max(0.f, input[inputIndex]);
		else if constexpr (activationFunction == ActivationFunction::TANH)
			activated = std::tanh(input[inputIndex]);
		else
			activated = input[inputIndex];

		const size_t pack = inputIndex / FLOATS_PER_REGISTER;
		const size_t inPack = inputIndex % FLOATS_PER_REGISTER;

		for (int outputIndex = 0; outputIndex < outputNeurons; outputIndex++)
		{
			scalarOutput[outputIndex] += activated * layer.Weights[pack][outputIndex][inPack];
		}
	}
	return true;
}

template<int inputNeurons, int outputNeurons, ActivationFunction activationFunction>
bool TestDenseLayerShape(const char* name)
{
	constexpr float EPSILON = 1e-4f;

	DenseLayer<inputNeurons, outputNeurons, activationFunction> layer;
	layer.InitializeWeights();

	// Generate deterministic input data
	Xorshift64 prng(0xCAFE);
	std::uniform_real_distribution<float> inputDist(-1.f, 1.f);
	alignas(CACHE_LINE_SIZE) float input[inputNeurons];
	for (int inputIndex = 0; inputIndex < inputNeurons; inputIndex++)
		input[inputIndex] = inputDist(prng);

	// AVX forward pass
	float* avxOutput = layer.Forward(input);

	// Copy AVX result before scalar overwrites Output
	float avxResult[outputNeurons];
	std::memcpy(avxResult, avxOutput, sizeof(avxResult));

	// Scalar forward pass
	float scalarResult[outputNeurons];
	ScalarForward(layer, input, scalarResult);

	bool passed = true;
	for (int outputIndex = 0; outputIndex < outputNeurons; outputIndex++)
	{
		const float delta = std::abs(avxResult[outputIndex] - scalarResult[outputIndex]);
		if (delta >= EPSILON)
		{
			std::cout << "  FAIL " << name << " output[" << outputIndex << "]: "
				<< "avx=" << avxResult[outputIndex]
				<< " scalar=" << scalarResult[outputIndex]
				<< " delta=" << delta << "\n";
			passed = false;
		}
	}

	if (passed)
		std::cout << "  PASS " << name << "\n";

	return passed;
}

inline bool TestDenseLayer()
{
	std::cout << "Running DenseLayer AVX vs scalar tests\n";

	bool allPassed = true;
	allPassed &= TestDenseLayerShape<16, 4, ActivationFunction::RELU>("DenseLayer<16,4,RELU>");
	allPassed &= TestDenseLayerShape<64, 8, ActivationFunction::RELU>("DenseLayer<64,8,RELU>");
	allPassed &= TestDenseLayerShape<32, 1, ActivationFunction::TANH>("DenseLayer<32,1,TANH>");

	if (!allPassed)
		std::cout << "DenseLayer test FAILED\n";
	else
		std::cout << "DenseLayer test passed\n";

	return allPassed;
}
