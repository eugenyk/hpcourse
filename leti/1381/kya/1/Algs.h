#ifndef IMAGE_ALGS_H
#define IMAGE_ALGS_H

#include <memory>
#include <vector>
#include <tuple>
#include <fstream>

#include "Image.h"

class MinBrightness
{
public:
	std::vector<std::tuple<unsigned char, unsigned char>> operator() (std::shared_ptr<Image> image)
	{
		std::vector<std::tuple<unsigned char, unsigned char>> result;

		int min = 255;

		for (int i = 0; i < image->width; i++)
		{
			for (int j = 0; j < image->height; j++)
			{
				if (image->matrix[i][j] == min)
				{
					result.push_back({ i, j });
					continue;
				}

				if (image.get()->matrix[i][j] < min)
				{
					result.clear();
					min = image->matrix[i][j];
					result.push_back({ i, j });
				}
			}
		}

		return result;
	}
};

class MaxBrightness
{
public:
	std::vector<std::tuple<unsigned char, unsigned char>> operator() (std::shared_ptr<Image> image)
	{
		std::vector<std::tuple<unsigned char, unsigned char>> result;

		int max = 0;

		for (int i = 0; i < image->width; i++)
		{
			for (int j = 0; j < image->height; j++)
			{
				if (image.get()->matrix[i][j] == max)
				{
					result.push_back({ i, j });
					continue;
				}

				if (image.get()->matrix[i][j] > max)
				{
					result.clear();
					max = image.get()->matrix[i][j];
					result.push_back({ i, j });
				}
			}
		}

		return result;
	}
};

class EquBrightness
{
	unsigned char reqBrightness;
public:
	EquBrightness(unsigned char brightness)
	{
		reqBrightness = brightness;
	}

	std::vector<std::tuple<unsigned char, unsigned char>> operator() (std::shared_ptr<Image> image)
	{
		std::vector<std::tuple<unsigned char, unsigned char>> result;

		for (int i = 0; i < image->width; i++)
			for (int j = 0; j < image->height; j++)
				if (image->matrix[i][j] == reqBrightness)
					result.push_back({ i, j });

		return result;
	}
};

class Inverse
{
public:
	std::shared_ptr<Image> operator() (std::shared_ptr<Image> image)
	{
		std::shared_ptr<Image> result(new Image(image->width, image->height));

		for (int i = 0; i < image->width; i++)
			for (int j = 0; j < image->height; j++)
				result->matrix[i][j] = 255 - image->matrix[i][j];

		return result;
	}
};

class Average
{
public:
	unsigned char operator() (std::tuple<unsigned char, unsigned char> pair)
	{
		unsigned char min = std::get<0>(pair);
		unsigned char max = std::get<1>(pair);
		return min + (max - min) / 2;
	}
};

class ToImageConverter
{
public:
	std::shared_ptr<Image> operator() (
		std::tuple<
			std::vector<std::tuple<unsigned char, unsigned char>>,
			std::vector<std::tuple<unsigned char, unsigned char>>,
			std::vector<std::tuple<unsigned char, unsigned char>>,
			std::shared_ptr<Image>
		> joinOutput
	)
	{
		return std::get<3>(joinOutput);
	}
};

class ToMinMaxConverter
{
public:
	std::tuple<unsigned char, unsigned char> operator() (
		std::tuple<
			std::vector<std::tuple<unsigned char, unsigned char>>,
			std::vector<std::tuple<unsigned char, unsigned char>>,
			std::vector<std::tuple<unsigned char, unsigned char>>,
			std::shared_ptr<Image>
		> joinOutput
	)
	{
		std::shared_ptr<Image> image = std::get<3>(joinOutput);

		std::tuple<unsigned char, unsigned char> minPixel = std::get<0>(joinOutput)[0];
		std::tuple<unsigned char, unsigned char> maxPixel = std::get<1>(joinOutput)[0];

		int min = image->matrix[std::get<0>(minPixel)][std::get<1>(minPixel)];
		int max = image->matrix[std::get<0>(maxPixel)][std::get<1>(maxPixel)];

		return{ min, max };
	}
};

class LogWriter
{
	static std::ofstream fout;
public:
	LogWriter::LogWriter(const std::string & filename = "")
	{
		fout.open(filename.c_str());
	}

	bool LogWriter::operator() (unsigned char avgBrightness)
	{
		if (!fout)
			return false;

		fout << (int)avgBrightness << std::endl;
		return true;
	}
};

#endif 