#ifndef IMAGE_H
#define IMAGE_H

#include <cstdlib>
#include <ctime>

struct Image
{
	unsigned char ** matrix;
	int    width;
	int    height;

	Image(int width = 128, int height = 128);
	~Image();
};

#endif