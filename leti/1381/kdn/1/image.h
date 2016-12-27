#ifndef IMAGE_H
#define IMAGE_H

#include <fstream>

using Brightness = unsigned char;

struct Image
{
    Brightness* pixels;
    int width;
    int height;

    Image();
    Image(int w, int h);
    ~Image();

    void random();
    friend std::ostream& operator<<(std::ostream& out, const Image& image);
};

#endif
