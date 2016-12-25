#ifndef _PIXEL_H_
#define _PIXEL_H_

#include <cstddef>
#include <cstdint>

struct Pixel
{
    size_t row;
    size_t column;
    uint8_t value;

    Pixel(size_t r, size_t c, uint8_t v) : row(r), column(c), value(v)
    {
    }
};

#endif // _PIXEL_H_
