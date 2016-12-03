#ifndef FIXEDBRIGHTNESS_H
#define FIXEDBRIGHTNESS_H
#include "common.h"

class FixedBrightness
{
    unsigned char _brightness;
public:
    FixedBrightness(unsigned char value);
    PointsWithImage operator()( Image * img ) const;
};

#endif // FIXEDBRIGHTNESS_H
