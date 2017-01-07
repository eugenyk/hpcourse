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
typedef std::pair<int, std::vector<Pixel>> Pixels;
class Image
{
public:
	Image(int count = 0,unsigned m = M_size, unsigned n = N_size);
	Image(const Image& im);
	~Image(){}
	int get_id() const;
	void inverse_image();
	double mean_brightness() const;
	void lead_point(Pixels p);
	Pixels min_pixel() const;
	Pixels max_pixel() const;
	Pixels find_pixel(unsigned char val) const;
	void print_image() const;
	unsigned char Image::operator() (unsigned row, unsigned col) const;
private:
	int id;
	Pixel get_pixel(unsigned i){ return Pixel((unsigned)(i % wight), (unsigned)(i / height)); }
	vector<unsigned char> matrix;
	unsigned height;
	unsigned wight;
	unsigned size;

	static const auto M_size = 1000;
	static const auto N_size = 1000;
};
