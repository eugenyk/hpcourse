#include "image.h"

#include <random>

Coord2d fromDiffType(std::ptrdiff_t diff, std::size_t nColumn, std::size_t nRow)
{
    (void) nRow;
    Coord2d c;
    c.column = diff % nColumn;
    c.row = diff / nColumn;
    return c;
}

std::ptrdiff_t toDiffType(const Coord2d &coord, std::size_t nColumn, std::size_t nRow)
{
    (void) nColumn;
    std::ptrdiff_t diff = coord.column + coord.row * nRow;
    return diff;
}

bool isValid(const Coord2d &coord, std::size_t nColumn, std::size_t nRow)
{
    return coord.column >= 0 && coord.row >= 0 && coord.column < nColumn && coord.row < nRow;
}

Image::Image(unsigned int width, unsigned int height):
    m_width(width), m_height(height)
{
    m_pixels.reserve(size());
}

void Image::fill()
{
    for(unsigned int i = 0; i < size(); i++)
        m_pixels.push_back(std::rand() % (UCHAR_MAX + 1));
}
