#ifndef IMAGEGENERATOR_H
#define IMAGEGENERATOR_H

#include "common.h"

class ImageGenerator
{
public:
    ImageGenerator();
    bool operator()(Image* & output);
};

#endif // IMAGEGENERATOR_H
