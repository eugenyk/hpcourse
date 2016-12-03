#include "maxbrightness.h"

MaxBrightness::MaxBrightness()
{

}

PointsWithImage MaxBrightness::operator()(Image *img) const {
    PointsVector maxBrightnessPoints;
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
    return PointsWithImage (maxBrightnessPoints,img);
}
