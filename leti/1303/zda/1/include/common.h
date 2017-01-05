#ifndef COMMON_H
#define COMMON_H

#include <vector>
#include <image.h>
#include <iostream>
#include "tbb/flow_graph.h"
typedef std::pair<int,int> Point;
typedef std::vector< Point > PointsVector;
typedef std::pair<PointsVector, Image *> PointsWithImage;
typedef std::pair<Image *, double> ImageWithMean;
typedef std::pair<Image *, Image *> ImageWithInverted;
typedef tbb::flow::tuple< ImageWithMean, ImageWithInverted> MeanAndInvertedResults;
typedef tbb::flow::tuple< PointsWithImage, PointsWithImage, PointsWithImage > MinMaxFixedResults;

#endif // COMMON_H
