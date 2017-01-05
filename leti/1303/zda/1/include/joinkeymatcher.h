#ifndef JOINKEYMATCHER_H
#define JOINKEYMATCHER_H

#include "common.h"

class JoinKeyMatcher
{
public:
    JoinKeyMatcher();
    Image * operator()(const PointsWithImage & data);
    Image * operator()(const ImageWithMean & data);
    Image * operator()(const ImageWithInverted & data);
};

#endif // JOINKEYMATCHER_H
