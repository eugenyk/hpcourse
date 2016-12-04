#include "stdafx.h"
#include "Image.h"

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


//private methods.

void Image::fillPixels()
{
	for (int i = 0; i < width * height; ++i)
	{
		map[i] = rand() % 256;
	}
}
