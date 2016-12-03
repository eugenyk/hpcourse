#ifndef POINTHIGHLIGHTER_H
#define POINTHIGHLIGHTER_H

#include "common.h"

class PointHighlighter
{
public:
    PointHighlighter();
    Image * operator()( ThreeMessages input ) const;
};

#endif // POINTHIGHLIGHTER_H
