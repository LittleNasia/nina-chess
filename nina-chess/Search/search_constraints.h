#pragma once
#include "Core/Engine/utils.h"
#include <cstdint>

struct SearchConstraints
{
	int Depth = invalidInt;
	int64_t Time = invalidInt;
	int Movetime = invalidInt;
	int Nodes = invalidInt;
};
