#ifndef LAB03_ELEMENTS_H
#define LAB03_ELEMENTS_H

#include <vector>
#include <limits>
#include "image.h"

using Position = std::pair<int, int>;
using PositionsList = std::vector<Position>;

using ElementsResultType = PositionsList;

enum struct ImageElementFinderType {
    MIN = 1,
    MAX = 2,
    EXACT = 3
};

// TODO avoid capturing image in map by 'function' as function<void (int x, int y, value_type value)>.
struct ImageElementFinder {
    static void check_value(int value);
    
    static PositionsList positions_with_value(ImageConstPtr image, Image::value_type value);
    
    static ElementsResultType result_from_positions(ImageConstPtr image, const PositionsList &positions);
    
    virtual ~ImageElementFinder();
};

struct NoneElements : public ImageElementFinder {
    ElementsResultType operator()(ImageConstPtr image) const;
};

struct MaximumElements : public ImageElementFinder {
    ElementsResultType operator()(ImageConstPtr image) const;
};

struct MinimumElements : public ImageElementFinder {
    ElementsResultType operator()(ImageConstPtr image) const;
};

struct ExactElements : public ImageElementFinder {
    explicit ExactElements(Image::value_type value): value_(value) {}
    
    ElementsResultType operator()(ImageConstPtr image) const;

private:
    Image::value_type value_;
};

#endif //LAB03_ELEMENTS_H
