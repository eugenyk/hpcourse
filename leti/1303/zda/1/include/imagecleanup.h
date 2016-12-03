#ifndef IMAGECLEANUP_H
#define IMAGECLEANUP_H
#include "common.h"

class ImageCleanup
{
public:
    ImageCleanup();
    double operator()( MeanAndInvertedResults results );
};

#endif // IMAGECLEANUP_H
