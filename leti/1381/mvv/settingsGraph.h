#ifndef SETTINGS_H
#define SETTINGS_H
#include "image.h"
#include "pixel.h"
#include <tuple>
#include <tbb/concurrent_vector.h>
using namespace std;
template<class T>
struct foundValue {
    T val;
    image<T> imageFV;
};

template<class T>
struct foundPixels {
    tbb::concurrent_vector<Pixel<T> > vals;
    image<T> imageFP;
};

template<class T>
struct avgBrightness {
    double avg;
    image<T> imageAB;
};

typedef image<unsigned char> image8u;
typedef foundValue<unsigned char> foundValue8u;
typedef foundPixels<unsigned char> foundPixels8u;
typedef tuple<image8u, image8u, image8u> jointype;
typedef avgBrightness<unsigned char> avgBrightness8u;

#endif