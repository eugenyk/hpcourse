#ifndef POINTHIGHLIGHTER_H
#define POINTHIGHLIGHTER_H

#include "common.h"

class PointHighlighter
{
public:
    PointHighlighter();
    Image * operator()( MinMaxFixedResults input ) const;
};

#endif // POINTHIGHLIGHTER_H
