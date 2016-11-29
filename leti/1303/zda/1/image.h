#ifndef IMAGE_H
#define IMAGE_H

#include <vector>

class Image
{
    int _width;
    int _height;
    std::vector< std::vector<unsigned char> > _data;

    void fillRandom();
public:
    Image();
    unsigned char at(int x, int y) const;
    int width() const;
    int height() const;
};

#endif // IMAGE_H
