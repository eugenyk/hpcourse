#ifndef MEANBRIGHTNESS_H
#define MEANBRIGHTNESS_H

#include "common.h"

class MeanBrightness
{
public:
    MeanBrightness();
    ImageWithMean operator()( Image * img ) const;
};

#endif // MEANBRIGHTNESS_H
