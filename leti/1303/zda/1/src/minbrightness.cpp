#include "minbrightness.h"

MinBrightness::MinBrightness()
{

}

PointsWithImage MinBrightness::operator()(Image *img) const {
    PointsVector minBrightnessPoints;
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
    return PointsWithImage (minBrightnessPoints,img);
}
