#include <random>
#include <iostream>
#include <iomanip>
#include "image.h"

Image::Image(size_t w, size_t h) : _width(w), _height(h), _matrix(w * h, 0)
{
    generateBrightness();
}

Image::~Image()
{
}

void Image::setBrightness(size_t row, size_t col, uint8_t value)
{
    _matrix[row * _width + col] = value;
}

uint8_t Image::getBrightness(size_t row, size_t col) const
{
    return _matrix[row * _width + col];
}

size_t Image::getWidth() const
{
    return _width;
}

size_t Image::getHeight() const
{
    return _height;
}

void Image::generateBrightness()
{
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dis(0, 255);
    for (size_t row = 0; row < _height; ++row)
    {
        for (size_t col = 0; col < _width; ++col)
        {
            _matrix[row * _width + col] = dis(gen);
        }
    }
}

std::ostream& operator<<(std::ostream& output, const Image& image)
{
    if (image.getWidth() < 1)
    {
        return output;
    }
    for (size_t row = 0; row < image.getHeight(); ++row)
    {
        output << std::setw(3) << (int)image.getBrightness(row, 0);
        for (size_t col = 1; col < image.getWidth(); ++col)
        {
            output << " " << std::setw(3) << (int)image.getBrightness(row, col);
        }
        output << std::endl;
    }
    return output;
}
