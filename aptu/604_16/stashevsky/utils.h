#pragma once

#include <cstdint>
#include <functional>
#include <vector>

struct Image {
    typedef uint8_t pixel;
    std::size_t width;
    std::size_t height;

    std::vector<std::vector<pixel>> data;
};

Image GenerateRandomMatrix(std::size_t height, std::size_t width);

template <class T>
inline void foreach(Image &image, T action) {
    for (size_t i = 0; i < image.height; ++i) {
        for (size_t j = 0; j < image.width; ++j) {
            action(image.data[i][j]);
        }
    }
}

template <class A, class R>
inline R fold(Image const& image, R init, A action) {
    for (size_t i = 0; i < image.height; ++i) {
        for (size_t j = 0; j < image.width; ++j) {
            init = action(init, image.data[i][j]);
        }
    }

    return init;
}
