#include "Image.h"

Image::Image(int width, int height)
{
	this->width = width;
	this->height = height;

	matrix = new unsigned char * [width];

	for (int i = 0; i < width; i++)
	{
		matrix[i] = new unsigned char[height];

		for(int j = 0; j < height; j++)
			matrix[i][j] = 0 + rand() % 256;
	}
}

Image::~Image()
{
	for (int i = 0; i < width; i++)
		delete[] matrix[i];
	delete[] matrix;
}