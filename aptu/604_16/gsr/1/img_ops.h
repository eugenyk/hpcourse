#ifndef TBBHW_IMG_OPS_H
#define TBBHW_IMG_OPS_H

#include <vector>
#include <glob.h>
#include <random>

namespace {
    std::random_device rd;
    std::mt19937 rng(rd());
    std::uniform_int_distribution<int> uni(0, 255);
};

const size_t NUM_IMAGES = 100;
const size_t DEFAULT_HEIGHT = 480;
const size_t DEFAULT_WIDTH = 640;

struct image {
    size_t n;
    size_t m;

    image(){}
    image(size_t n, size_t m) : n(n), m(m), data(n * m) {
        for (auto &elem : data) {
            elem = (unsigned char) uni(rng);
        }
    }

    float mean() const {
        float result = 0.f;
        for (auto &elem : data) {
            result += elem;
        }
        return result / data.size();
    }

    std::vector<size_t> filter(unsigned char el) const {
        std::vector<size_t> res;
        for (auto &elem : data) {
            if (elem == el)
                res.push_back(elem);
        }
        return res;
    }

    void map(std::function<void(unsigned char)> f) const {
        std::vector<size_t> res;
        for (auto elem : data) {
            f(elem);
        }
    }

    void invert(unsigned char max_val) {
        for (auto &elem : data){
            elem = max_val - elem;
        }
    }

    void highlight(std::vector<size_t> positions) {
        for (auto &pos: positions) {
            mark_rectangle(pos);
        }
    }

private:
    std::vector<unsigned char> data;

    void mark_rectangle(size_t pos) {
        mark(pos - 1);
        mark(pos - 1 - n);
        mark(pos - 1 + n);

        mark(pos + 1);
        mark(pos + 1 - n);
        mark(pos + 1 + n);

        mark(pos - n);
        mark(pos + n);
    }

    void mark(int pos){
        if (pos > 0 && pos < data.size()){
            data[pos] = 255;
        }
    }

};


#endif //TBBHW_IMG_OPS_H
