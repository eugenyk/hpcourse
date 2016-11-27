#include "Image.h"
#include <iostream>

unsigned long Image::idCounter = 0;

Image::Image(unsigned int width, unsigned int height) : width(width), height(height) {
    // Set id.
    id = idCounter;
    idCounter++;
    // Create matrix.
    pixelMatrix = new unsigned char *[height];
    for (unsigned int i = 0; i < height; i++) {
        pixelMatrix[i] = new unsigned char[width];
    }
}

Image::Image(const Image & image) : Image(image.getWidth(), image.getHeight()) {
    // Copy matrix.
    for (unsigned int i = 0; i < height; i++) {
        for (unsigned int j = 0; j < width; j++) {
            pixelMatrix[i][j] = image.pixelMatrix[i][j];
        }
    }
}

Image::~Image() {
    for (unsigned int i = 0; i < height; i++) {
        delete[]pixelMatrix[i];
    }
    delete[] pixelMatrix;
}

unsigned int Image::getWidth() const {
    return width;
}

unsigned int Image::getHeight() const {
    return height;
}

unsigned char* Image::getImageLine(unsigned int lineNumber) {
    if (lineNumber < height) {
        return pixelMatrix[lineNumber];
    }
    return nullptr;
}

unsigned long Image::getId() const {
    return id;
}
