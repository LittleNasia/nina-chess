#pragma once
#include "Core/Engine/utils.h"

struct SearchConstraints
{
	int Depth = invalidInt;
	int64_t Time = invalidInt;
	int Movetime = invalidInt;
	int Nodes = invalidInt;
};
