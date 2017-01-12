//
// Created by dsavvinov on 09.12.16.
//

#include <cstdio>
#include <cstdlib>
#include "Generator.h"

static size_t ROWS_COUNT = 500;
static size_t COLS_COUNT = 600;

void Generator::fillImageRandomly(Image & img) {
    img.resize(ROWS_COUNT);

    for (size_t i = 0; i < img.size(); ++i) {
        img[i].resize(COLS_COUNT);

        for (size_t j = 0; j < img[i].size(); ++j) {
            img[i][j] = (uint8_t) abs(rand());
        }
    }
}

std::vector<Image> Generator::generateImages(size_t size, uint32_t seed) {
    std::vector<Image> imgs = std::vector<Image>(size);
    srand(seed);
    for (size_t i = 0; i < imgs.size(); ++i) {
        fillImageRandomly(imgs[i]);
    }

    return imgs;
}


