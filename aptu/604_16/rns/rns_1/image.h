#pragma once

#include <vector>
#include <tuple>
#include <cstdint>

namespace lab_flow
{
    typedef std::pair<size_t, size_t> coords;
    typedef std::vector<coords> coords_vec;

    class image
    {
    public:
        typedef std::vector<uint8_t>::iterator iterator;
        typedef std::vector<uint8_t>::const_iterator const_iterator;

    public:
        image(size_t width = 0, size_t height = 0);
        image(size_t width, size_t height, const std::vector<uint8_t>& data);
        image(const image& other);
        image(image&& other);
        ~image();

        image& operator=(image other);

        uint8_t operator[](coords&& c) const;
        uint8_t& operator[](coords&& c);
        uint8_t operator[](const coords& c) const;
        uint8_t& operator[](const coords& c);

        void swap(image& other);

        iterator begin();
        const_iterator begin() const;
        iterator end();
        const_iterator end() const;

        coords_vec find(uint8_t value) const;

        size_t get_width() const;
        size_t get_height() const;

    private:
        size_t _width, _height;
        std::vector<uint8_t> _data;
    };

    image make_random_image(size_t width, size_t height);

}