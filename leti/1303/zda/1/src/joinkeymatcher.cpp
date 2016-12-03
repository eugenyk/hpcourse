#include "include/joinkeymatcher.h"

JoinKeyMatcher::JoinKeyMatcher()
{

}

Image * JoinKeyMatcher::operator()(const PointsWithImage data) {
    return data.second;
}
