//
// Created by wimag on 12.12.16.
//

#include <random>
#include <algorithm>
#include <climits>
#include "image.h"
using namespace std;
image::image(size_t n, size_t m) : image(n, m, 10){}

image::image(size_t n, size_t m, size_t window_size) : n(n), m(m), window_size(window_size), data(vector<char>(n*m)) {
    for(size_t i = 0; i < n*m; i++)
        data[i] = (char) (rand() % 256);
}


char &image::get(size_t x, size_t y) {
    return data[y * m + x];
}

void image::set(size_t x, size_t y, char value) {
    data[y*m + x] = value;
}

char &image::get(point p) {
    return get(p.first, p.second);
}

void image::set(point p, char value) {
    set(p.first, p.second, value);
}

vector<point> image::max_points() const{
    char maxv = *max_element(data.begin(), data.end());
    return exact_points(maxv);
}

vector<point> image::min_points() const {
    char minv = *min_element(data.begin(), data.end());
    return exact_points(minv);
}

vector<point> image::exact_points(char v) const {
    vector<point> res;
    for(size_t i = 0; i < n*m; i++){
        if(data[i] == v){
            res.push_back(make_pair(i % m, i / m));
        }
    }
    return res;
}

void image::highlight_point(point p) {
    int x = (int) p.first;
    int y = (int) p.second;
    int w = (int) window_size;
    for(int i = -w; i <= w; i ++){
        tryset(x + i, y - w, CHAR_MAX);
        tryset(x + i, y + w, CHAR_MAX);
        tryset(x - w, y + i, CHAR_MAX);
        tryset(x + w, y + i, CHAR_MAX);
    }
}

void image::tryset(int x, int y, char value) {
    if(x >= 0 && x < (int)m && y >= 0 && y < (int)n){
        set((size_t) x, (size_t) y, value);
    }
}

double image::average() const{
    double sum = 0;
    for(size_t i = 0; i < n*m; i++){
        sum += data[i];
    }
    return sum / (n*m);
}

image::image(const image &other) :n(other.n), m(other.m), window_size(other.window_size), data(other.data) {
}

void image::invert() {
    for(size_t i = 0; i < n*m; i++){
        data[i] = (char) (CHAR_MAX - data[i]);
    }
}

image::image() : image(400, 600){}







