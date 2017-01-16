#ifndef ADDFUNCTION_H
#define ADDFUNCTION_H
#include "image.h"
#include "pixel.h"
#include "base.h"
#include <tbb/tbb.h>
#include <tbb/flow_graph.h>
#include <tbb/concurrent_vector.h>
#include "settingsGraph.h"
template<class T>
T findMinMean(image<T> srcImage)
{
	T* buffer = new T[srcImage.height];
	tbb::parallel_for(0, srcImage.height, 1, [buffer, srcImage](size_t i) 
	{
		T* arr = srcImage.data + i*srcImage.width;
		T min = arr[0];
		for (int j = 1; j < srcImage.width; j++)
			if (min > arr[j])
				min = arr[j];
		buffer[i] = min;
	});
	T min = buffer[0];
	for (int i = 1; i < srcImage.height; i++)
		if (min > buffer[i])
			min = buffer[i];
	delete[] buffer;
	return min;
}
template<class T>
T findMaxValue(image<T> srcImage)
{
	T* buffer = new T[srcImage.height];
	tbb::parallel_for(0, srcImage.height, 1, [buffer, srcImage](size_t i) 
	{
		T* arr = srcImage.data + i*srcImage.width;
		T max = arr[0];
		for (int j = 1; j < srcImage.width; j++)
			if (max < arr[j])
				max = arr[j];
		buffer[i] = max;
	}
	);
	T max = buffer[0];
	for (int i = 1; i < srcImage.height; i++)
		if (max < buffer[i])
			max = buffer[i];
	delete[] buffer;
	return max;
}

template<class T>
tbb::concurrent_vector<Pixel<T> > findValue(image<T> srcImage, T value)
{
	tbb::concurrent_vector<Pixel<T> > result;
	tbb::parallel_for(0, srcImage.height, 1, [srcImage, value, &result](size_t i) 
	{
		T* arr = srcImage.data + i*srcImage.width;
		Pixel<T> pix;
		pix.value = value;
		pix.indexHeight = i;
		for (int j = 0; j < srcImage.width; j++)
			if (arr[j] == value) {
				pix.indexWidht = j;
				result.push_back(pix);
			}
	}
	);
	return result;
}

template<class T>
void markElement(image<T> srcImage, Pixel<T> p)
{
	for (int ii = -1; ii <= 1; ii++)
		for (int jj = -1; jj <= 1; jj++) 
		{
			T* pix_ptr = srcImage.getPixel(p.indexWidht + jj, p.indexHeight + ii);
			if (pix_ptr != 0)
				pix_ptr[0] = p.value;
		}
}

void logAvgBrightness(std::vector<avgBrightness8u> b, std::string filename)
{
	FILE* f = fopen(filename.c_str(), "w+");
	for (int i = 0; i < b.size(); i++)
		fprintf(f, "The Brighness mean %d is %lf\n", b[i].imageAB.id, b[i].avg);
	fclose(fp);
}



#endif