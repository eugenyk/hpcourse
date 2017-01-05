#include "joinkeymatcher.h"

JoinKeyMatcher::JoinKeyMatcher()
{

}

Image *JoinKeyMatcher::operator()(const PointsWithImage & data)
{
    return data.second;
}

Image *JoinKeyMatcher::operator()(const ImageWithMean & data)
{
    return data.first;
}

Image *JoinKeyMatcher::operator()(const ImageWithInverted & data)
{
    return data.first;
}
