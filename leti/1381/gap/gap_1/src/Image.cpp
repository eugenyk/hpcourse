#include "stdafx.h"
#include "Image.h"



Image::Image(int count, unsigned m , unsigned n )
{
	id = count;
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
	id = im.id;
}

int Image::get_id() const
{ return id; }
Pixels Image::min_pixel() const
{
	Pixels min_arr;
	unsigned char min = 255;
	min_arr.first = id;
	for (auto i = 0; i < size; i++)
	{
		if (matrix[i] < min){
			min_arr.second.clear();
			min_arr.second.push_back(Pixel((unsigned)(i % wight), (unsigned)(i / height)));
			min = matrix[i];
		}
		else if (matrix[i] == min)
			min_arr.second.push_back(Pixel((unsigned)(i % wight), (unsigned)(i / height)));
	}

	return min_arr;
}

Pixels Image::max_pixel() const
{
	Pixels max_arr;
	unsigned char max = 0;
	max_arr.first = id;
	for (auto i = 0; i < size; i++)
	{
		if (matrix[i] > max){
			max_arr.second.clear();
			max_arr.second.push_back(Pixel((unsigned)(i % wight), (unsigned)(i / height)));
			max = matrix[i];
		}
		else if (matrix[i] == max)
			max_arr.second.push_back(Pixel((unsigned)(i % wight), (unsigned)(i / height)));
	}
	return max_arr;
}
Pixels Image::find_pixel(unsigned char val) const
{
	Pixels find;
	find.first = id;
	for (int i = 0; i < size; i++)
	if (matrix[i] == val) find.second.push_back(Pixel((unsigned)(i % wight), (unsigned)(i / height)));
	return find;
}
unsigned char Image::operator() (unsigned row, unsigned col) const
{
	if (row >= height || col >= wight)
		throw exception("const Matrix subscript out of bounds");
	return matrix[wight*row + col];
}
void Image::inverse_image(){ 
	for (auto i = 0; i < size; i++)
		matrix[i] = 255 - matrix[i];
	return; 
}
double Image::mean_brightness() const{
	double mean = 0;
	for (auto i = 0; i < size; i++)
		mean += matrix[i];
	mean = mean / size;
	return mean;
}
void Image::lead_point(Pixels p){ 
	for (auto i = 0; i < p.second.size(); i++)
	{
		for (auto x = p.second[i].x - 1; x <= p.second[i].x; x++)
		for (auto y = p.second[i].y - 1; y <= p.second[i].y; y++)
		if ((x>0) && (x<wight) && (y > 0) && (y < height) && ((x != p.second[i].x) || (y != p.second[i].y)))
			matrix[y*wight + x] = 255;
	}
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