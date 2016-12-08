#ifndef BODY_OBJECTS_H
#define BODY_OBJECTS_H

#include "tbb/flow_graph.h"
#include "Image.h"
#include <iostream>
#include "ImageGenerator.h"
#include "ImageAlgorithms.h"
#include <fstream>
#include "ParmParser.h"

using namespace std;

struct MinBrightness
{
	vector<Pixel> operator() (ImagePtr image)
	{
		cout << "MIN" << endl;
		return getMinBrightness(image);
	}
};

struct MaxBrightness
{
	vector<Pixel> operator() (ImagePtr image)
	{
		cout << "MAX" << endl;
		return getMaxBrightness(image);
	}
};

struct EquBrightness
{
	int b;

	EquBrightness(int b) : b(b) {}

	vector<Pixel> operator() (ImagePtr image)
	{
		cout << "EQU" << endl;
		return getEquBrightness(image, b);
	}
};

struct Inverser
{
	ImagePtr operator() (ImagePtr image)
	{
		cout << "INV" << endl;
		return getInversedImage(image);
	}
};

struct Average
{
	int operator() (tuple<int, int> v)
	{
		int min = get<0>(v);
		int max = get<1>(v);
		int avg = getAverageBrightness(min, max);
		cout << "AVG: " << "avg(" << min << ", " << max << ") = " << avg << endl;
		return avg;
	}
};

struct InvConverter
{
	ImagePtr operator() (tuple<vector<Pixel>, vector<Pixel>, vector<Pixel>, ImagePtr> v)
	{
		cout << "INV converter" << endl;
		return get<3>(v);
	}
};

struct AvgConverter
{
	tuple<int, int> operator() (tuple<vector<Pixel>, vector<Pixel>, vector<Pixel>, ImagePtr> v)
	{
		cout << "AVG converter" << endl;
		ImagePtr image = get<3>(v);

		Pixel minPixel = get<0>(v)[0];
		Pixel maxPixel = get<1>(v)[0];

		int min = image->matrix[minPixel.i][minPixel.j];
		int max = image->matrix[maxPixel.i][maxPixel.j];

		return tuple<int, int>(min, max);
	}
};

extern ofstream fout;

struct LogWriter
{
	LogWriter() {}

	LogWriter(const string filename)
	{
		fout.open(filename.c_str());
		if (!fout)
			cerr << "LogWriter: unable to open a file" << endl;
	}

	bool operator() (int avgBrightness)
	{
		if (!fout)
			return false;

		cout << "LOG: " << avgBrightness << endl;
		fout << avgBrightness << endl;

		return true;
	}
};

#endif 