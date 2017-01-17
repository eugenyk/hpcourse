//
// Created by antonpp on 16.01.17.
//

#ifndef AU_PARALLEL_COMPUTING_IMAGE_H
#define AU_PARALLEL_COMPUTING_IMAGE_H

#include <vector>
#include <cstdlib>
#include <algorithm>
#include <iterator>
#include <limits>

typedef unsigned char pixel_t;

class Image {
public:
    typedef std::pair<int, int> pos_t;

    Image(size_t w = 0, size_t h = 0);

    std::vector<pos_t> get_border(size_t pixel_index) const;

    const std::vector<pixel_t> &get_pixels() const;

    pixel_t get_pixel(pos_t pos) const;

    pixel_t get_pixel(size_t pos) const;

    void generate(size_t w, size_t h);

    static pixel_t invert_pixel(pixel_t c);

    size_t get_id() const;

private:
    size_t id;

    size_t width;
    size_t height;
    std::vector<pixel_t> pixels;

};


#endif //AU_PARALLEL_COMPUTING_IMAGE_H
