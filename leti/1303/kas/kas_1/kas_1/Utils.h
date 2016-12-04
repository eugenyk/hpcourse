#include "Image.h"
#include "stdafx.h"
//#include <vector>

#pragma once
class Utils
{
public:
	Utils();
	~Utils();

	static Image* generateImages(int width, int height, int num);
	static std::vector<Image > generateImages2(int width, int height, int num);
};

