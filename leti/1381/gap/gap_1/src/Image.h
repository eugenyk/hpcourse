#pragma once
#include <vector>
#include <iostream>

using namespace std;


struct Pixel
{
	Pixel(unsigned _x, unsigned _y){ x = _x; y = _y; }
	unsigned x;
	unsigned y;

};
typedef std::vector<Pixel> Pixels;
class Image
{
public:
	Image(unsigned m = M_size, unsigned n = N_size);
	Image(const Image& im);
	~Image(){}

	void inverse_image();
	double mean_brightness() const;
	void lead_point(Pixels p) const;
	Pixels min_pixel() const;
	Pixels max_pixel() const;
	Pixels find_pixel(unsigned char val) const;
	void print_image() const;
	unsigned char Image::operator() (unsigned row, unsigned col) const;
private:
	Pixel get_pixel(unsigned i){ return Pixel((unsigned)(i % wight), (unsigned)(i / height)); }
	vector<unsigned char> matrix;
	unsigned height;
	unsigned wight;
	unsigned size;

	static const auto M_size = 5;
	static const auto N_size = 5;
};
