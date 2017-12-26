#include "elements.h"
#include "common.h"

void ImageElementFinder::check_value(int value) {
    assert(value >= 0 && value <= std::numeric_limits<Image::value_type>::max());
}

PositionsList ImageElementFinder::positions_with_value(ImageConstPtr image, Image::value_type value) {
    PositionsList positions;
    image->map([&](int x, int y) {
        if (image->get(x, y) == value) {
            positions.emplace_back(x, y);
        }
    });
    return positions;
}

ElementsResultType ImageElementFinder::result_from_positions(ImageConstPtr image, const PositionsList &positions) {
    return positions;
}

ImageElementFinder::~ImageElementFinder() {}

ElementsResultType NoneElements::operator()(ImageConstPtr image) const {
    return ElementsResultType();
}

ElementsResultType MaximumElements::operator()(ImageConstPtr image) const {
    LOG("enter maxer");
    int maximum_value = -1;
    image->map([&](int x, int y) {
        auto current_value = image->get(x, y);
        if (maximum_value < current_value) {
            maximum_value = current_value;
        }
    });
    
    check_value(maximum_value);
    auto positions = positions_with_value(image, static_cast<unsigned char>(maximum_value));
    return result_from_positions(image, positions);
}

ElementsResultType MinimumElements::operator()(ImageConstPtr image) const {
    LOG("enter miner");
    int minimum_value = std::numeric_limits<int>::max();
    image->map([&](int x, int y) {
        auto current_value = image->get(x, y);
        if (minimum_value > current_value) {
            minimum_value = current_value;
        }
    });

    check_value(minimum_value);
    auto positions = positions_with_value(image, static_cast<unsigned char>(minimum_value));
    return result_from_positions(image, positions);
}

ElementsResultType ExactElements::operator()(ImageConstPtr image) const {
    LOG("enter exacter");
    auto positions = positions_with_value(image, value_);
    return result_from_positions(image, positions);
}
