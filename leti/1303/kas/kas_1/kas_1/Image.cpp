#include "stdafx.h"
#include "Image.h"
#include <vector>

//public methods.

Image::Image()
{
}

Image::Image(int w, int h) : width(w), height(h)
{
	id = 0;
	map = new unsigned char[width * height];
	fillPixels();
}

Image::Image(int w, int h, int id) : width(w), height(h), id(id)
{
	map = new unsigned char[width * height];
}

Image::Image(int w, int h, int id, unsigned char * map) : width(w), height(h), id(id), map(map)
{
}

Image::Image(const Image & img)
{
	//printf("######## CALL THE CONSTRUCTOR COPY OF IMAGE #############\n");
	id = img.id;
	width = img.width;
	height = img.height;
	map = new unsigned char[width * height];
	memcpy(map, img.map, sizeof(unsigned char) * width * height);

}

Image::~Image()
{
	delete[] map;
}

int Image::getWidth() const
{
	return width;
}

int Image::getHeight() const
{
	return height;
}

unsigned char * Image::getMap() const
{
	return map;
}

void Image::printMap() const
{
	printf("*** Print image map *** \n");
	for (int i = 0; i < height; ++i)
	{
		for (int j = 0; j < width; ++j)
		{
			printf("%03d ", map[i * height + j]);
		}
		printf("\n");
	}
}

void Image::updPix(int index, unsigned char value)
{
	map[index] = value;
}

std::vector<Image*> Image::divByParts(int numParts)
{
	std::vector<Image*> imgParts;

	int rows = height / numParts;

	//TODO: check that the last part include all pixs!
	for (int i = 0; i < numParts; i++)
	{
		unsigned char *partMap;
		int begin = i * rows * width;
		int end = (i + 1) * rows * width;
		for (int j = begin; j < end; j++)
		{
			partMap[j - begin] = map[j];
		}
		Image* img = new Image(width, rows, i + 1, partMap);
		imgParts.push_back(img);
	}

	return imgParts;
}


//private methods.

void Image::fillPixels()
{
	for (int i = 0; i < width * height; ++i)
	{
		map[i] = rand() % 256;
	}
}
