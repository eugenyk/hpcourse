#ifndef MAXBRIGHTNESS_H
#define MAXBRIGHTNESS_H

#include "common.h"

class MaxBrightness
{
public:
    MaxBrightness();
    PointsWithImage operator()( Image * img ) const;
};

#endif // MAXBRIGHTNESS_H
