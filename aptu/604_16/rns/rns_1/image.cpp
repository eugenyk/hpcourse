#include "image.h"

#include <random>
#include <algorithm>
#include <limits>

namespace lab_flow
    {
    image::image(size_t width, size_t height)
        : _width(width)
        , _height(height)
    {}

    image::image(size_t width, size_t height, const std::vector<uint8_t>& data)
        : image(width, height)
    {
        _data = std::move(data);
    }

    image::image(image&& other)
        : _width(other._width)
        , _height(other._height)
        , _data(std::move(other._data))
    {}

    image::image(const image& other)
        : _width(other._width)
        , _height(other._height)
        , _data(other._data)
    {}

    image::~image()
    {}

    image& image::operator=(image other)
    {
        this->swap(other);
        return *this;
    }

    uint8_t image::operator[](coords&& c) const
    {
        size_t i, j;
        std::tie(i, j) = std::move(c);
        return _data[i * _width + j];
    }

    uint8_t& image::operator[](coords&& c)
    {
        size_t i, j;
        std::tie(i, j) = std::move(c);
        return _data[i * _width + j];
    }

    uint8_t image::operator[](const coords& c) const
    {
        size_t i, j;
        std::tie(i, j) = c;
        return _data[i * _width + j];
    }

    uint8_t& image::operator[](const coords& c)
    {
        size_t i, j;
        std::tie(i, j) = c;
        return _data[i * _width + j];
    }

    void image::swap(image& other)
    {
        std::swap(_width, other._width);
        std::swap(_height, other._height);
        std::swap(_data, other._data);
    }

    image::iterator image::begin()
    {
        return _data.begin();
    }

    image::const_iterator image::begin() const
    {
        return _data.begin();
    }

    image::iterator image::end()
    {
        return _data.end();
    }

    image::const_iterator image::end() const
    {
        return _data.end();
    }

    coords_vec image::find(uint8_t value) const
    {
        coords_vec result;
        for (size_t i = 0; i < _height; ++i)
        {
            for (size_t j = 0; j < _width; ++j)
            {
                if (this->operator[](coords(i,j)) == value)
                    result.emplace_back(i, j);
            }
        }
        return result;
    }

    size_t image::get_width() const
    {
        return _width;
    }

    size_t image::get_height() const
    {
        return _height;
    }

    image make_random_image(size_t width, size_t height)
    {
        static std::uniform_int_distribution<uint8_t> distribution(
            0, std::numeric_limits<uint8_t>::max());
        static std::default_random_engine generator;

        std::vector<uint8_t> data(width * height);
        std::generate(data.begin(), data.end(), []() { return distribution(generator); });
        return image(width, height, data);
    }

}