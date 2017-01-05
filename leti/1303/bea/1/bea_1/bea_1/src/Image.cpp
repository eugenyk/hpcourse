#include "Image.h"

Image::Image(int M, int N)
{
	this->N = N;
	this->M = M;

	matrix = new int * [M];

	for (int i = 0; i < M; i++)
		matrix[i] = new int[N];
}

Image::~Image()
{
	for (int i = 0; i < M; i++)
		delete[] matrix[i];
	delete[] matrix;
}


ostream & operator << (ostream & out, ImagePtr image)
{
	for (int i = 0; i < image->M; i++)
	{
		for (int j = 0; j < image->N; j++)
			out << image->matrix[i][j] << '\t';
		out << endl;
	}
	out << endl << endl;

	return out;
}