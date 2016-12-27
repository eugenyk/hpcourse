#ifndef PARMETERS_H
#define PARMETERS_H

#include <string>
#include <sstream>
#include <iostream>

struct Parameters
{
	unsigned char reqBrightness;
	int parallelLimit;
	long reqImageCount;
	std::string logFilename;
	bool good;

	Parameters()
	{
		reqImageCount = reqBrightness = -1;
		parallelLimit = 1;
		good = false;
	}

	operator bool() { return good; }
};

#endif