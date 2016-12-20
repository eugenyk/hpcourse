#pragma once

#include <vector>
#include <glob.h>
#include <random>
#include <functional>

class GeneratedImage {
    size_t width;
    size_t height;

    std::vector<unsigned char> data;

public:
    GeneratedImage(){}

    GeneratedImage(size_t n, size_t m) : width(n), height(m), data(n * m) {
        for (auto &elem : data) {
            elem = (unsigned char) rand() % 256;
        }
    }

    double mean() const;

    std::vector<size_t> filter(unsigned char el) const;

    void map(std::function<void(unsigned char)> f) const;

    void invert(unsigned char max_val);

    void highlight(std::vector<size_t> positions);

private:

    void highlight_rectangle(size_t pos);

    void make_white(size_t pos);
};