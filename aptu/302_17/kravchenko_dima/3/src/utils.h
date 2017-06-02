#ifndef __UTILS_H__
#define __UTILS_H__

#include "image.h"
#include <vector>
#include <iostream>

image * generate_image(int id);
std::vector<point_t> find_max_elements(image *img);
std::vector<point_t> find_min_elements(image *img);
std::vector<point_t> find_equal_elements(image *img, int val);
image * highlight_found(std::tuple<image *, std::vector<point_t>, std::vector<point_t>, std::vector<point_t>> t);
void inverse_brightness(image *img);
std::pair<image *, double> get_avg_brightness(image *img);
image * print_avg_brightness(std::pair<image *, double> a, std::ostream &out);

#endif // __UTILS_H__
