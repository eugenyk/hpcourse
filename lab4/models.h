#ifndef MODELS
#define MODELS

#include <cmath>
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <memory>
#include <utility>
#include <vector>

using std::vector;
using std::pair;
using std::ofstream;

struct pixel {
    uint r;
    uint g;
    uint b;
};

typedef vector<vector<pixel>> image;
const uint MAX_COLOR = 256;
const int MARGIN = 2;

class img_generator {
    int w;
    int h;
    int l;
public:
    img_generator(int w_, int h_, int l_): w(w_), h(h_), l(l_) {}

    bool operator()(std::shared_ptr<image>& result) {
        if(l <= 0) {
            std::cout << "Used!" << std::endl;
            return false;
        }
        --l;

        result.reset(new image(w, vector<pixel>(h)));
        image* result_ptr = result.get();

        for(int i = 0; i < w; ++i) {
            for(int j = 0; j < h; ++j) {
                (*result_ptr)[i][j].r = rand() % MAX_COLOR;
                (*result_ptr)[i][j].g = rand() % MAX_COLOR;
                (*result_ptr)[i][j].b = rand() % MAX_COLOR;
            }
        }

        return true;
    }
};

uint brightness(pixel p) {
    return p.r + p.g + p.b;
}

struct find_max {
    vector<pair<int, int>> operator()(std::shared_ptr<image> img) {
        std::cout << "find_max" << std::endl;

        uint max = 0;
        vector<pair<int, int>> result;
        image* img_ptr = img.get();
        int w = img_ptr->size();
        int h = (*img_ptr)[0].size();

        for(int i = 0; i < w; ++i) {
            for(int j = 0; j < h; ++j) {
                uint br = brightness((*img_ptr)[i][j]);
                if(br > max) {
                    max = br;
                }
            }
        }

        for(int i = 0; i < w; ++i) {
            for(int j = 0; j < h; ++j) {
                uint br = brightness((*img_ptr)[i][j]);
                if(br == max) {
                    result.push_back(std::make_pair(i, j));
                }
            }
        }

        for(int i = 0; i < result.size(); ++i) {
            pixel p = (*img_ptr)[result[i].first][result[i].second];
            std::cout << "Max at (" << result[i].first << ", " << result[i].second << "): (" << p.r << ", " << p.g << ", " << p.b << ")"
                      << std::endl;
        }

        return result;
    }
};

struct find_min {
    vector<pair<int, int>> operator()(std::shared_ptr<image> img) {
        std::cout << "find_min" << std::endl;

        uint min = 3 * MAX_COLOR + 1;
        vector<pair<int, int>> result;
        image* img_ptr = img.get();
        int w = img_ptr->size();
        int h = (*img_ptr)[0].size();

        for(int i = 0; i < w; ++i) {
            for(int j = 0; j < h; ++j) {
                uint br = brightness((*img_ptr)[i][j]);
                if(min > br) {
                    min = br;
                }
            }
        }

        for(int i = 0; i < w; ++i) {
            for(int j = 0; j < h; ++j) {
                uint br = brightness((*img_ptr)[i][j]);
                if(min == br) {
                    result.push_back(std::make_pair(i, j));
                }
            }
        }

        for(int i = 0; i < result.size(); ++i) {
            pixel p = (*img_ptr)[result[i].first][result[i].second];
            std::cout << "Min at (" << result[i].first << ", " << result[i].second << "): (" << p.r << ", " << p.g << ", " << p.b << ")"
                      << std::endl;
        }

        return result;
    }
};

class find_val {
    int b;
public:
    find_val(int a): b(a) {}

    vector<pair<int, int>> operator()(std::shared_ptr<image> img) {
        std::cout << "find_val" << std::endl;

        vector<pair<int, int>> result;
        image* img_ptr = img.get();
        int w = img_ptr->size();
        int h = (*img_ptr)[0].size();

        for(int i = 0; i < w; ++i) {
            for(int j = 0; j < h; ++j) {
                uint br = brightness((*img_ptr)[i][j]);
                if(b == br) {
                    result.push_back(std::make_pair(i, j));
                }
            }
        }

        for(int i = 0; i < result.size(); ++i) {
            pixel p = (*img_ptr)[result[i].first][result[i].second];
            std::cout << "Val " << b << " at (" << result[i].first << ", " << result[i].second << "): ("
                      << p.r << ", " << p.g << ", " << p.b << ")" << std::endl;
        }

        return result;
    }
};

struct invert_img {
private:
    void invert_img_helper(image* img_ptr, const vector<pair<int, int>>& pixels) {
        for(auto p: pixels) {
            int w = img_ptr->size();
            int h = (*img_ptr)[0].size();
            int pi = p.first;
            int pj = p.second;
            for(int i = -MARGIN; i <= MARGIN; ++i) {
                for(int j = -MARGIN; j <= MARGIN; ++j) {
                    if(pi + i >= 0 && pi + i < w && pj + j >= 0 && pj + j < h) {
                        (*img_ptr)[pi + i][pj + j].r = MAX_COLOR - (*img_ptr)[pi + i][pj + j].r;
                        (*img_ptr)[pi + i][pj + j].g = MAX_COLOR - (*img_ptr)[pi + i][pj + j].g;
                        (*img_ptr)[pi + i][pj + j].b = MAX_COLOR - (*img_ptr)[pi + i][pj + j].b;
                    }
                }
            }
        }
    }

public:
    bool operator()(std::tuple<std::shared_ptr<image>, const vector<pair<int, int>>&, const vector<pair<int, int>>&, const vector<pair<int, int>>&> v) {
        std::cout << "invert_img" << std::endl;

        std::shared_ptr<image> img = std::get<0>(v);
        image new_img = *img.get();
        invert_img_helper(&new_img, std::get<1>(v));
        invert_img_helper(&new_img, std::get<2>(v));
        invert_img_helper(&new_img, std::get<3>(v));
        return true;
    }
};

class average_img {
    ofstream& out;

    void average_img_helper(const image* img_ptr, const vector<pair<int, int>>& pixels) {
        for(auto p: pixels) {
            int w = img_ptr->size();
            int h = (*img_ptr)[0].size();
            int pi = p.first;
            int pj = p.second;
            double sumr = 0;
            double sumg = 0;
            double sumb = 0;
            int cnt = 0;

            for(int i = -MARGIN; i <= MARGIN; ++i) {
                for(int j = -MARGIN; j <= MARGIN; ++j) {
                    if(pi + i >= 0 && pi + i < w && pj + j >= 0 && pj + j < h) {
                        sumr += (*img_ptr)[pi + i][pj + j].r;
                        sumg += (*img_ptr)[pi + i][pj + j].g;
                        sumb += (*img_ptr)[pi + i][pj + j].b;
                        ++cnt;
                    }
                }
            }

            sumr /= cnt;
            sumg /= cnt;
            sumb /= cnt;

            out << "Average at (" << pi << ", " << pj << ") is " << "(" << sumr << ", " << sumg << ", " << sumb << ")" << std::endl;
        }
    }

public:
    average_img(ofstream& o): out(o) {}

    bool operator()(std::tuple<std::shared_ptr<image>, const vector<pair<int, int>>&, const vector<pair<int, int>>&, const vector<pair<int, int>>&> v) {
        std::cout << "average_img" << std::endl;

        std::shared_ptr<image> img = std::get<0>(v);
        average_img_helper(img.get(), std::get<1>(v));
        average_img_helper(img.get(), std::get<2>(v));
        average_img_helper(img.get(), std::get<3>(v));

        out << "The end." << std::endl;

        return true;
    }
};

#endif
