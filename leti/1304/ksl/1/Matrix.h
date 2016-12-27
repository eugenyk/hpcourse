#pragma once

#include <functional>

template <typename T>
class Matrix
{
	Matrix(std::pair<uint32_t, uint32_t> _size)
	{
		size = _size;
		data = new T[size.first * size.second];
		memset(data, 0, sizeof(T)* size.first * size.second);
	}

	Matrix(T* _data, std::pair<uint32_t, uint32_t> _size)
	{
		data = _data;
		size = _size;
	}


	T* getData()
	{
		return data;
	}


protected:
	T* data;
	std::pair<uint32_t, uint32_t> size;
};

class CharMatrix
{
public:
	CharMatrix(unsigned int _w = 320, unsigned int _h = 240)
	{
		data = nullptr;
		setSize(_w, _h);
	}

	CharMatrix(const CharMatrix& another)
	{
		w = another.w;
		h = another.h;
		data = new unsigned char[w * h];
		memcpy(data, another.data, sizeof(unsigned char)* w * h);
	}

	void setSize(unsigned int _w, unsigned int _h)
	{
		if (data) {
			delete data;
		}
		w = _w;
		h = _h;
		data = new unsigned char[w * h];
		memset(data, 0, sizeof(char) * w * h);
	}

	void randomFilling()
	{
		for (unsigned int i = 0; i < w * h; ++i) {
			data[i] = rand() % 256;
		}
	}

	unsigned long long getSize()
	{
		return w * h;
	}

	void pixel(int pos, unsigned char nVal)
	{
		data[pos] = nVal;
	}

	void processingCell(std::function<void(unsigned char&, int)> fun)
	{
		for (unsigned int i = 0; i < w * h; ++i)
		{
			fun(data[i], i);
		}
	}

protected:
	unsigned int w;
	unsigned int h;
	unsigned char* data;
};