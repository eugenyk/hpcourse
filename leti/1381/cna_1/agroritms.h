#ifndef AGRORITMS_H
#define AGRORITMS_H

#include "image.h"

#include <numeric>

template <typename Iterator>
std::vector<typename Iterator::difference_type> findPixels(Iterator b, Iterator e, typename Iterator::value_type value)
{
    std::vector<typename Iterator::difference_type> founded;
    for(Iterator it = b; it != e; it++)
        if(*it == value)
            founded.push_back(std::distance(b, it));
    return founded;
}

template <typename Iterator>
typename Iterator::value_type meanValue(Iterator b, Iterator e, typename Iterator::value_type init)
{
    if(b == e)
        return init;
    typename Iterator::value_type mean = *b;
    for(Iterator it = b; it != e; it++)
        mean = (mean + *it) / 2;
    return mean;
}

template <typename Iterator>
void bitInverseValues(Iterator b, Iterator e)
{
    for(Iterator it = b; it != e; it++)
        *it = ~*it;
}

#endif // AGRORITMS_H
