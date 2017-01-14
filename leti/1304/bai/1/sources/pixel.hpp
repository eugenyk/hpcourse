#ifndef __HPCOURSE_LAB1_PIXEL_HPP__
#define __HPCOURSE_LAB1_PIXEL_HPP__

#include <stdint.h>

namespace hpcourse {

struct Pixel {

    Pixel(const ::size_t row, const ::size_t column, uint8_t value):
        row(row),
        column(column),
        value(value)
    {}

    ::size_t row;
    ::size_t column;
    ::uint8_t value;

};

} // namespace hpcourse

#endif /* __HPCOURSE_LAB1_PIXEL_HPP__ */
