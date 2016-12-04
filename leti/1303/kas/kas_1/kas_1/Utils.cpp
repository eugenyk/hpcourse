#include "stdafx.h"
#include "Utils.h"
#include <vector>

Utils::Utils()
{
}


Utils::~Utils()
{
}

Image * Utils::generateImages(int width, int height, int num)
{
	Image *imgs = new Image[num];
	for (int i = 0; i < num; i++)
	{
		imgs[i] = Image(width, height);
	}
	return imgs;
}

std::vector<Image > Utils::generateImages2(int width, int height, int num)
{
	std::vector<Image> imgs;
	for (int i = 0; i < num; i++)
	{
		imgs.push_back(Image(width, height));
	}
	return imgs;
}
