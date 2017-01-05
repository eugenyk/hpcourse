#ifndef BRIGHTNESSINVERTER_H
#define BRIGHTNESSINVERTER_H

#include "common.h"

class BrightnessInverter
{
public:
    BrightnessInverter();
    ImageWithInverted operator()( Image * img ) const;
};

#endif // BRIGHTNESSINVERTER_H
