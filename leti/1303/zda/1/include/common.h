#ifndef COMMON_H
#define COMMON_H

#include <vector>
#include <image.h>
#include "tbb/flow_graph.h"
typedef std::pair<int,int> Point;
typedef std::vector< Point > PointsVector;
typedef std::pair<PointsVector, Image*> PointsWithImage;
typedef tbb::flow::tuple< PointsWithImage, PointsWithImage, PointsWithImage > ThreeMessages;

#endif // COMMON_H
