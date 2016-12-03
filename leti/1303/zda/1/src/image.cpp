#include "image.h"
#include <stdlib.h>

Image::Image()
{
    _width = 1000;
    _height = 1000;
    fillRandom();
}

void Image::fillRandom()
{
    for(int i=0; i<_height; i++) {
        std::vector<unsigned char> row;
        for(int j=0; j<_width; j++) {
            row.push_back(static_cast<unsigned char>(rand()%256));
        }
        _data.push_back(row);
    }
}

int Image::width() const
{
    return _width;
}

int Image::height() const
{
    return _height;
}

void Image::highlightPoints(std::vector<std::pair<int, int> > points)
{
    for(int i=0; i<points.size(); i++) {
        std::pair<int, int> cur = points[i];
        int x = cur.first;
        int y = cur.second;
        set(x-1,y+1,255);
        set(x-1,y-1,255);
        set(x,y+1,255);
        set(x,y-1,255);
        set(x+1,y+1,255);
        set(x+1,y-1,255);
        set(x+1,y,255);
        set(x-1,y,255);
    }
}


