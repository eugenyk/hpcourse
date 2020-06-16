#include "utils.h"
#include "image.h"

#include <utility>
#include <vector>
#include <cmath>
#include <iostream>
#include <iomanip>
#include <fstream>

image * generate_image(int id)
{
    int height = 100;
    int width = 100;

    image *img = new image(id, height, width);
    for (int row = 0; row < height; row++)
        for (int col = 0; col < height; col++)
            img->set_pixel(row, col, rand() % 255);

    return img;
}

std::vector<point_t> find_max_elements(image *img)
{
    int height = img->get_height();
    int width = img->get_width();

    int mx = 0;

    for (int row = 0; row < height; row++) {
        for (int col = 0; col < width; col++) {
            mx = std::max(mx, img->get_pixel(row, col));
        }
    }

    std::vector<std::pair<int, int>> ans;

    for (int row = 0; row < height; row++) {
        for (int col = 0; col < width; col++) {
            if (img->get_pixel(row, col) == mx)
                ans.push_back(std::make_pair(row, col));
        }
    }

    return ans;
}

std::vector<point_t> find_min_elements(image *img)
{
    int height = img->get_height();
    int width = img->get_width();

    int mx = 255;

    for (int row = 0; row < height; row++) {
        for (int col = 0; col < width; col++) {
            mx = std::min(mx, img->get_pixel(row, col));
        }
    }

    std::vector<std::pair<int, int>> ans;

    for (int row = 0; row < height; row++) {
        for (int col = 0; col < width; col++) {
            if (img->get_pixel(row, col) == mx)
                ans.push_back(std::make_pair(row, col));
        }
    }

    return ans;
}

std::vector<point_t> find_equal_elements(image *img, int val)
{
    int height = img->get_height();
    int width = img->get_width();

    std::vector<std::pair<int, int>> ans;

    for (int row = 0; row < height; row++) {
        for (int col = 0; col < width; col++) {
            if (img->get_pixel(row, col) == val)
                ans.push_back(std::make_pair(row, col));
        }
    }

    return ans;
}

static void set_point(point_t p, image *img, image *ans)
{
    int height = img->get_height();
    int width = img->get_width();

    for (int row = -IMAGE_MARK_BOUNDARY; row <= IMAGE_MARK_BOUNDARY; row++) {
        for (int col = -IMAGE_MARK_BOUNDARY; col <= IMAGE_MARK_BOUNDARY; col++) {
            if (p.first + row >= 0 && p.first + row < height && p.second + col >= 0 && p.second + col < width)
                ans->set_pixel(p.first + row, p.second + col, img->get_pixel(p.first + row, p.second + col));
        }
    }
}

image * highlight_found(std::tuple<image *, std::vector<point_t>, std::vector<point_t>, std::vector<point_t>> t)
{
    image *img = std::get<0>(t);
    std::vector<point_t> max_v = std::get<1>(t);
    std::vector<point_t> min_v = std::get<2>(t);
    std::vector<point_t> eq_v = std::get<3>(t);

    int id = img->get_id();
    int height = img->get_height();
    int width = img->get_width();

    image ans(id, height, width);

    for (int row = 0; row < height; row++) {
        for (int col = 0; col < width; col++) {
            ans.set_pixel(row, col, 0);
        }
    }

    for (point_t p : max_v) {
        set_point(p, img, &ans);
    }
    for (point_t p : min_v) {
        set_point(p, img, &ans);
    }
    for (point_t p : eq_v) {
        set_point(p, img, &ans);
    }

    for (int row = 0; row < height; row++) {
        for (int col = 0; col < width; col++) {
            img->set_pixel(row, col, ans.get_pixel(row, col));
        }
    }

    return img;
}

// void since we dont use it's result
void inverse_brightness(image *img)
{
    int id = img->get_id();
    int height = img->get_height();
    int width = img->get_width();

    image ans(id, height, width);

    for (int row = 0; row < height; row++) {
        for (int col = 0; col < width; col++) {
            ans.set_pixel(row, col, 255 - img->get_pixel(row, col));
        }
    }
}

std::pair<image *, double> get_avg_brightness(image *img)
{
    int height = img->get_height();
    int width = img->get_width();

    int ans = 0;

    for (int row = 0; row < height; row++) {
        for (int col = 0; col < width; col++) {
                ans += img->get_pixel(row, col);
        }
    }

    return std::make_pair(img, static_cast<double>(ans) / (height * width));
}

image * print_avg_brightness(std::pair<image *, double> a, std::ostream &out)
{
    out << "for image " << a.first->get_id() << " brightness is ";
    out << std::fixed << std::setprecision(5) << a.second << std::endl;
    return a.first;
}
