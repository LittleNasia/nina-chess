#pragma once
#include "utils.h"

struct SearchConstraints
{
	int depth = -1;
	int64_t time = -1;
	int movetime = -1;
	int nodes = -1;
};