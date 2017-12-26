#include "elements.h"

void ImageElementFinder::check_value(int value) {
    assert(value >= 0 && value <= std::numeric_limits<Image::value_type>::max());
}

std::vector<Position> ImageElementFinder::positions_with_value(const Image &image, Image::value_type value) {
    std::vector<Position> positions;
    image.map([&](int x, int y) {
        if (image.get(x, y) == value) {
            positions.emplace_back(x, y);
        }
    });
    return positions;
}

std::vector<Position> MaximumElements::operator()(const Image &image) const {
    int maximum_value = -1;
    image.map([&](int x, int y) {
        auto current_value = image.get(x, y);
        if (maximum_value < current_value) {
            maximum_value = current_value;
        }
    });
    
    check_value(maximum_value);
    return positions_with_value(image, static_cast<unsigned char>(maximum_value));
}

std::vector<Position> MinimumElements::operator()(const Image &image) const {
    int minimum_value = std::numeric_limits<int>::max();
    image.map([&](int x, int y) {
        auto current_value = image.get(x, y);
        if (minimum_value > current_value) {
            minimum_value = current_value;
        }
    });
    
    check_value(minimum_value);
    return positions_with_value(image, static_cast<unsigned char>(minimum_value));
}

std::vector<Position> ExactElements::operator()(const Image &image) const {
    return positions_with_value(image, value_);
}
