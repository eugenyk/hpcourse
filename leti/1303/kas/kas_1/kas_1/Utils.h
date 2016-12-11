#include "Image.h"
#include "stdafx.h"
#include <vector>

#pragma once
class Utils
{
public:
	Utils();
	~Utils();

	static std::tuple<int, int, int, std::string > argsProcessing(int argc, char* argv[]);
};

