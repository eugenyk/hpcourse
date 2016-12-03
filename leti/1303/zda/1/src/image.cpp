#include "image.h"
#include <stdlib.h>

Image::Image(int width, int height)
{
    _width = width;
    _height = height;
    fillRandom();
}

void Image::fillRandom()
{
    for(int i=0; i<_height; i++) {
        std::vector<unsigned char> row;
        for(int j=0; j<_width; j++) {
            row.push_back(static_cast<unsigned char>(rand()%255));
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

Image * Image::getInvertedImage()
{
    Image * img = new Image(width(),height());
    for(int i=0; i<_height; i++) {
        for(int j=0; j<_width; j++) {
            img->set(j,i,255 - at(j,i));
        }
    }
    return img;
}

double Image::getMeanBrightness()
{
    double mean = 0;
    int n = 0;
    for(int i=0; i<_height; i++) {
        for(int j=0; j<_width; j++) {
            double delta = at(j,i) - mean;
            mean += delta/++n;
        }
    }
    return mean;
}


