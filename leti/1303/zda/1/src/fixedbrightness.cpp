#include "fixedbrightness.h"
#include <iostream>

FixedBrightness::FixedBrightness(unsigned char value)
{
    _brightness = value;
}

PointsWithImage FixedBrightness::operator()(Image *img) const
{
    PointsVector selectedPoints;
    for(int i=0; i<img->width(); i++) {
        for(int j=0; j<img->height(); j++) {
            if(img->at(i,j) == _brightness) {
                selectedPoints.push_back(Point(i,j));
            }
        }
    }
    return PointsWithImage (selectedPoints,img);
}
