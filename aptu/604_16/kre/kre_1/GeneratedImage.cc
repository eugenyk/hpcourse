#include "GeneratedImage.h"

double GeneratedImage::mean() const {
    float result = 0.f;
    for (auto &elem : data) {
        result += elem;
    }
    return result / data.size();
}

std::vector<size_t> GeneratedImage::filter(unsigned char el) const {
    std::vector<size_t> res;
    for (auto &elem : data) {
        if (elem == el)
            res.push_back(elem);
    }
    return res;
}

void GeneratedImage::map(std::function<void(unsigned char)> f) const {
    std::vector<size_t> res;
    for (auto elem : data) {
        f(elem);
    }
}

void GeneratedImage::invert(unsigned char max_val) {
    for (auto &elem : data){
        elem = max_val - elem;
    }
}

void GeneratedImage::highlight(std::vector<size_t> positions) {
    for (auto &pos: positions) {
        highlight_rectangle(pos);
    }
}

void GeneratedImage::highlight_rectangle(size_t pos) {
    make_white(pos - 1);
    make_white(pos - 1 - width);
    make_white(pos - 1 + width);

    make_white(pos + 1);
    make_white(pos + 1 - width);
    make_white(pos + 1 + width);

    make_white(pos - width);
    make_white(pos + width);
}

void GeneratedImage::make_white(size_t pos){
    if (pos > 0 && pos < data.size()){
        data[pos] = 255;
    }
}
