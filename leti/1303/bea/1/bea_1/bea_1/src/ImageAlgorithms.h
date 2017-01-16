#ifndef IMAGE_ALGORITHMS
#define IMAGE_ALGORITHMS

#include <vector>
#include <memory>
#include "Image.h"

using std::vector;

struct Pixel
{
	int i;
	int j;
	Pixel(int i = 0, int j = 0) : i(i), j(j) { }
};

vector<Pixel> getMaxBrightness(ImagePtr image);
vector<Pixel> getMinBrightness(ImagePtr image);
vector<Pixel> getEquBrightness(ImagePtr image, int brightness);
int getAverageBrightness(int min, int max);
ImagePtr getInversedImage(ImagePtr image);

#endif