#include "stdafx.h"
#include "picture.h"



Picture::Picture(unsigned m, unsigned n)
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
Picture::Picture(const Picture& im){
	height = im.height;
	wight = im.wight;
	size = im.size;
	matrix = im.matrix;
}

Cells Picture::min_cell() const
{
	Cells min_arr;
	unsigned char min = 255;
	for (auto i = 0; i < size; i++)
	{
		if (matrix[i] < min){
			min_arr.clear();
			min_arr.push_back(Cell((unsigned)(i % wight), (unsigned)(i / height)));
			min = matrix[i];
		}
		else if (matrix[i] == min)
			min_arr.push_back(Cell((unsigned)(i % wight), (unsigned)(i / height)));
	}

	return min_arr;
}

Cells Picture::max_cell() const
{
	Cells max_arr;
	unsigned char max = 0;
	for (auto i = 0; i < size; i++)
	{
		if (matrix[i] > max){
			max_arr.clear();
			max_arr.push_back(Cell((unsigned)(i % wight), (unsigned)(i / height)));
			max = matrix[i];
		}
		else if (matrix[i] == max)
			max_arr.push_back(Cell((unsigned)(i % wight), (unsigned)(i / height)));
	}
	return max_arr;
}
Cells Picture::find_cell(unsigned char val) const
{
	Cells find;
	for (int i = 0; i < size; i++)
	if (matrix[i] == val) find.push_back(Cell((unsigned)(i % wight), (unsigned)(i / height)));
	return find;
}
unsigned char Picture::operator() (unsigned row, unsigned col) const
{
	if (row >= height || col >= wight)
		throw exception("const Matrix subscript out of bounds");
	return matrix[wight*row + col];
}
void Picture::inverse_image(){
	for (auto i = 0; i < size; i++)
		matrix[i] = 255 - matrix[i];
	return;
}
double Picture::mean_brightness() const{
	double mean = 0;
	for (auto i = 0; i < size; i++)
		mean += matrix[i];
	mean = mean / size;
	return mean;
}
void Picture::lead_point(Cells p){
	for (auto i = 0; i < p.size(); i++)
	{
		for (auto x = p[i].x - 1; x <= p[i].x; x++)
		for (auto y = p[i].y - 1; y <= p[i].y; y++)
		if ((x>0) && (x<wight) && (y > 0) && (y < height) && ((x != p[i].x) || (y != p[i].y)))
			matrix[y*wight + x] = 255;
	}
	return;
}
void Picture::print_image() const{
	for (auto i = 0; i < size; i++){
		if (i%wight == 0)
			cout << endl;
		cout << (int)(matrix[i]);
		if (matrix[i]>9) cout << "  ";
		else if (matrix[i] > 99) cout << " ";

	}
	return;
}