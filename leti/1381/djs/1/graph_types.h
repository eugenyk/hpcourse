#ifndef GRAPH_TYPES_H
#define GRAPH_TYPES_H

#include "image.h"
#include "pixel.h"
#include <tuple>
#include <tbb/concurrent_vector.h>

template<class T>
struct foundValue {
    T val;
    image<T> img;
};

template<class T>
struct foundPixels {
    tbb::concurrent_vector<pixel<T> > vals;
    image<T> img;
};

template<class T>
struct avgBrightness {
    double avg;
    image<T> img;
};
typedef image<unsigned char> image8u;
typedef foundValue<unsigned char> foundValue8u;
typedef foundPixels<unsigned char> foundPixels8u;
typedef std::tuple<image8u, image8u, image8u> jointype;
typedef avgBrightness<unsigned char> avgBrightness8u;

#endif // GRAPH_TYPES_H
