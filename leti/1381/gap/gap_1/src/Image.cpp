#include "stdafx.h"
#include "Image.h"



Image::Image(unsigned m , unsigned n )
{
	height = m;
	wight = n;
	size = n*m;
	if (m == 0 || n == 0)
		throw exception("Matrix constructor has 0 size");
	matrix.reserve(m * n);
	for (auto i = 0; i < size; i++){
		unsigned char m = abs(rand()) % 256;
		matrix.push_back(m);
	}
}
Image::Image(const Image& im){
	height = im.height;
	wight = im.wight;
	size = im.size;
	matrix = im.matrix;
}

Pixels Image::min_pixel() const
{
	Pixels min_arr;
	unsigned char min = 255;
	//tbb::parallel_for(0, size, 1, [=](int i, byte &min, std::vector<unsigned> &min_arr)
	for (auto i = 0; i < size; i++)
	{
		if (matrix[i] < min){
			min_arr.clear();
			min_arr.push_back(Pixel((unsigned)(i % wight), (unsigned)(i / height)));
			min = matrix[i];
		}
		else if (matrix[i] == min)
			min_arr.push_back(Pixel((unsigned)(i % wight), (unsigned)(i / height)));
	}

	return min_arr;
}

Pixels Image::max_pixel() const
{
	Pixels max_arr;
	unsigned char max = 0;
	//tbb::parallel_for(0, size, 1, [=](int i, byte &max, std::vector<unsigned> &max_arr)
	for (auto i = 0; i < size; i++)
	{
		if (matrix[i] > max){
			max_arr.clear();
			max_arr.push_back(Pixel((unsigned)(i % wight), (unsigned)(i / height)));
			max = matrix[i];
		}
		else if (matrix[i] == max)
			max_arr.push_back(Pixel((unsigned)(i % wight), (unsigned)(i / height)));
	}
	return max_arr;
}
Pixels Image::find_pixel(unsigned char val) const
{
	Pixels find;
	for (int i = 0; i < size; i++)
	if (matrix[i] == val) find.push_back(Pixel((unsigned)(i % wight), (unsigned)(i / height)));
	return find;
}
unsigned char Image::operator() (unsigned row, unsigned col) const
{
	if (row >= height || col >= wight)
		throw exception("const Matrix subscript out of bounds");
	return matrix[wight*row + col];
}
void Image::inverse_image(){ 
	return; 
}
double Image::mean_brightness() const{
	return 2;
}
void Image::lead_point(Pixels p) const{ 
	return; 
}
void Image::print_image() const{
	for (auto i = 0; i < size; i++){
		if (i%wight == 0)
			cout << endl;
		cout << (int)(matrix[i]);
		if (matrix[i]>9) cout <<"  ";
		else if (matrix[i] > 99) cout << " ";
		
	}
	return;
}