#ifndef JOINKEYMATCHER_H
#define JOINKEYMATCHER_H

#include "common.h"

class JoinKeyMatcher
{
public:
    JoinKeyMatcher();
    Image * operator()(const PointsWithImage data);
};

#endif // JOINKEYMATCHER_H
