#include "ImageAlgorithms.h"

vector<Pixel> getMaxBrightness(ImagePtr image)
{
	vector<Pixel> result;
	result.resize(image.get()->N * image.get()->M);

	int localMaximum = 0;

	for (int i = 0; i < image.get()->M; i++)
	{
		for (int j = 0; j < image.get()->N; j++)
		{
			if (image.get()->matrix[i][j] == localMaximum)
			{
				result.push_back(Pixel(i, j));
				continue;
			}

			if (image.get()->matrix[i][j] > localMaximum)
			{
				result.clear();
				localMaximum = image.get()->matrix[i][j];
				result.push_back(Pixel(i, j));
			}
		}
	}

	return result;
}

vector<Pixel> getMinBrightness(ImagePtr image)
{
	vector<Pixel> result;
	result.resize(image.get()->N * image.get()->M);

	int localMinimum = 255;

	for (int i = 0; i < image.get()->M; i++)
	{
		for (int j = 0; j < image.get()->N; j++)
		{
			if (image.get()->matrix[i][j] == localMinimum)
			{
				result.push_back(Pixel(i, j));
				continue;
			}

			if (image.get()->matrix[i][j] < localMinimum)
			{
				result.clear();
				localMinimum = image.get()->matrix[i][j];
				result.push_back(Pixel(i, j));
			}
		}
	}

	return result;
}

vector<Pixel> getEquBrightness(ImagePtr image, int brightness)
{
	vector<Pixel> result;

	for (int i = 0; i < image.get()->M; i++)
		for (int j = 0; j < image.get()->N; j++)
			if (image.get()->matrix[i][j] == brightness)
				result.push_back(Pixel(i, j));

	return result;
}

int getAverageBrightness(int min, int max)
{
	return max - (max - min) / 2;
}

ImagePtr getInversedImage(ImagePtr image)
{
	ImagePtr result(new Image(image.get()->M, image.get()->N));

	for (int i = 0; i < image.get()->M; i++)
		for (int j = 0; j < image.get()->N; j++)
			(*result).matrix[i][j] = 255 - image.get()->matrix[i][j];

	return result;
}