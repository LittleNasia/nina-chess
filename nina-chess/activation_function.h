#pragma once

#include "simd.h"
#include "utils.h"

enum class ActivationFunction
{
	TANH,
	RELU
};

template<ActivationFunction activationFunction>
forceinline SimdVector ApplyActivation(SimdVector input)
{
	if constexpr (activationFunction == ActivationFunction::RELU)
	{
		return SimdMax(input, SimdSetZero());
	}
	else if constexpr (activationFunction == ActivationFunction::TANH)
	{
		return SimdTanh(input);
	}
}
