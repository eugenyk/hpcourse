#ifndef IMAGE_H
#define IMAGE_H

#include <memory>
#include <iostream>

using namespace std;
using std::shared_ptr;

struct Image
{
	int ** matrix;
	int    M;
	int    N;

	Image(int M, int N);
	~Image();
};

typedef shared_ptr<Image> ImagePtr;

ostream & operator << (ostream & out, ImagePtr image);

#endif