#pragma once
#include "utils.h"

#include <fstream>

struct Serializable
{
	virtual void Serialize(std::ofstream& file) = 0;
	virtual void Deserialize(std::ifstream& file) = 0;
	virtual ~Serializable() = default;
};