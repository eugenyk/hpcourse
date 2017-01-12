//
// Created by dsavvinov on 09.12.16.
//

#ifndef TBB_GENERATOR_H
#define TBB_GENERATOR_H

#include <cstdint>
#include <vector>
#include "typedefs.h"

namespace Generator {
    std::vector<Image> generateImages(size_t size, uint32_t seed);
    void fillImageRandomly(Image & img);
}
#endif //TBB_GENERATOR_H
