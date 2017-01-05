#include "utils.h"

#include <cstdlib>
#include <ctime>

using namespace std;

Image GenerateRandomMatrix(size_t height, size_t width) {
    srand(time(0));

    vector<vector<Image::pixel>> result;
    result.resize(height);
    for (auto &row : result) {
        row.resize(width);
        for (auto &item : row) {
            item = rand() % 256;
        }
    }

    return {width, height, result};
}
