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

unsigned char Image::at(int x, int y) const
{
    if(x < _width && y < _height) {
        return _data.at(y).at(x);
    } else {
        return 255;
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

