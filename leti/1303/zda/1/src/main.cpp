#include <cstdio>
#include <image.h>
#include <iostream>
#include <vector>
#include "tbb/flow_graph.h"

using namespace tbb::flow;
typedef std::pair<int,int> Point;

struct maxBrightness {
    std::vector< Point > operator()( Image * img ) const {
        std::vector< Point > maxBrightnessPoints;
        unsigned char localMax = 0;
        for(int i=0; i<img->width(); i++) {
            for(int j=0; j<img->height(); j++) {
                unsigned char cur = img->at(i,j);
                if(cur > localMax) {
                    localMax = cur;
                    maxBrightnessPoints.clear();
                    maxBrightnessPoints.push_back(Point(i,j));
                } else if(cur == localMax) {
                    maxBrightnessPoints.push_back(Point(i,j));
                }
            }
        }
        return maxBrightnessPoints;
    }
};

struct minBrightness {
    std::vector< Point > operator()( Image * img ) const {
        std::vector< Point > minBrightnessPoints;
        unsigned char localMin = 255;
        for(int i=0; i<img->width(); i++) {
            for(int j=0; j<img->height(); j++) {
                unsigned char cur = img->at(i,j);
                if(cur < localMin) {
                    localMin = cur;
                    minBrightnessPoints.clear();
                    minBrightnessPoints.push_back(Point(i,j));
                } else if(cur == localMin) {
                    minBrightnessPoints.push_back(Point(i,j));
                }
            }
        }
        return minBrightnessPoints;
    }
};

int main() {
    graph g;

    broadcast_node< Image * > start(g);

    function_node< Image * , std::vector< Point > > maxBrightnessNode(g,1,maxBrightness());
    function_node< Image * , std::vector< Point > > minBrightnessNode(g,1,minBrightness());

    make_edge( start, maxBrightnessNode );
    make_edge( start, minBrightnessNode );

    for (int i = 0; i < 3; ++i ) {
        start.try_put( new Image());
    }
    g.wait_for_all();

    return 0;
}
