#include "Image.h"
#include <iostream>

Image::Image(unsigned int width, unsigned int height) : width(width), height(height) {
    pixelMatrix = new unsigned char *[height];
    for (unsigned int i = 0; i < height; i++) {
        pixelMatrix[i] = new unsigned char[width];
    }
}

Image::~Image() {
    for (unsigned int i = 0; i < height; i++) {
        delete[]pixelMatrix[i];
    }
    delete[] pixelMatrix;
}

unsigned int Image::getWidth() {
    return width;
}

unsigned int Image::getHeight()
{
    return height;
}

unsigned char* Image::getImageLine(unsigned int lineNumber)
{
    if (lineNumber < height) {
        return pixelMatrix[lineNumber];
    }
    return nullptr;
}
