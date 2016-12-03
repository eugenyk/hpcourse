#ifndef MINBRIGHTNESS_H
#define MINBRIGHTNESS_H
#include "common.h"

class MinBrightness
{
public:
    MinBrightness();
    PointsWithImage operator()( Image * img ) const;
};

#endif // MINBRIGHTNESS_H
