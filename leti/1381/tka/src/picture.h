#pragma once
#include <vector>
#include <iostream>

using namespace std;


struct Cell
{
	Cell(unsigned _x, unsigned _y){ x = _x; y = _y; }
	unsigned x;
	unsigned y;

};
typedef std::vector<Cell> Cells;
class Picture
{
public:
	Picture(unsigned m = M_size, unsigned n = N_size);
	Picture(const Picture& im);
	~Picture(){}

	void inverse_image();
	double mean_brightness() const;
	void lead_point(Cells p);
	Cells min_cell() const;
	Cells max_cell() const;
	Cells find_cell(unsigned char val) const;
	void print_image() const;
	unsigned char Picture::operator() (unsigned row, unsigned col) const;
private:
	Cell get_pixel(unsigned i){ return Cell((unsigned)(i % wight), (unsigned)(i / height)); }
	vector<unsigned char> matrix;
	unsigned height;
	unsigned wight;
	unsigned size;

	static const auto M_size = 1000;
	static const auto N_size = 1000;
};
