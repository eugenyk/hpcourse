#include "Image.h"
#include <ctime>
#include<iostream>
int Image::countForId = 0;
Image::Image(const int size)
{
	
	img = new int[size*size];
	id = countForId++;
	this->size = size;
	
	for (int i = 0; i < size*size; i++)
	{
		img[i] = rand() % 256;

	}
	
}
int Image::getId()
{
	return id;
}
void  Image::highliting(int index)
{
	
	img[index] = 0;
	if(index+size<=size*size) img[index + size] = 0;
	if (index  > size) img[index - size] = 0;
	if (index%size != 0) img[index + 1] = 0;
	if (index%size != 1) img[index - 1] = 0;
	if ((index + size <= size*size)&&(index%size != 0)) img[index + size+1] = 0;
	if ((index + size <= size*size) && (index%size != 1)) img[index + size - 1] = 0;
	if ((index  > size) && (index%size != 0)) img[index -size + 1] = 0;
	if ((index  > size) && (index%size != 1)) img[index - size - 1] = 0;

}
int Image::getV(int i, int j)
{
	return img[i + j*size];
}

int Image::getI(int i, int j)
{
	return j + i*size;
}
void Image::set(int val, int i, int j)
{
	 img[getI(i, j)]=val;
}
Image::~Image()
{
	
	delete[] img; 
}
