#ifndef LAB03_ELEMENTS_H
#define LAB03_ELEMENTS_H

#include <vector>
#include <limits>
#include "image.h"

using Position = std::pair<int, int>;


// TODO avoid capturing image in map by 'function' as function<void (int x, int y, value_type value)>.

struct ImageElementFinder {
    static void check_value(int value);
    
    static std::vector<Position> positions_with_value(const Image &image, Image::value_type value);
};

struct MaximumElements : public ImageElementFinder {
    std::vector<Position> operator()(const Image &image) const;
};

struct MinimumElements : public ImageElementFinder {
    std::vector<Position> operator()(const Image &image) const;
};

struct ExactElements : public ImageElementFinder {
    explicit ExactElements(Image::value_type value): value_(value) {}
    
    std::vector<Position> operator()(const Image &image) const;

private:
    Image::value_type value_;
};

#endif //LAB03_ELEMENTS_H
