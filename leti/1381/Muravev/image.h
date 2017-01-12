#pragma once
																																													#include <random>
																																																																													#include <limits>
template <class T>
struct image
{
    T* data;
    int height;
    int width;
    int identifier;
	void createImage(int _widht, int _height) : height(_height)
		, width(_widht)
	{		
		data = new T[width*height];
		for (int i = 0; i < width*height; i++)
            data[i] = rand() % (numeric_limits<T>::max() + 1);
    }
    T* getPixel(int indexWidht, int indexHeight) 
	{
		if (indexWidht < 0 || indexHeight < 0 || indexWidht >= width || indexHeight >= height)
            return 0;
        else
			return &data[indexHeight*width + indexWidht];
	}
};