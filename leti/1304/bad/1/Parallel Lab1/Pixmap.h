#ifndef Pixmap_h
#define Pixmap_h

#include <iostream>
#include "Pixel.h"

class Pixmap {
private:
    int width, height;
    std::vector<Pixel*> pixels;
public:
    Pixmap(int w, int h) {
        this->width = w;
        this->height = h;
        for(int i = 0; i < w * h; i++) {
            this->pixels.push_back(new Pixel());
        }
    }
    Pixel* get(int i, int j) {
        return pixels[i * width + j];
    }
    int getWidth() {
        return width;
    }
    int getHeight() {
        return height;
    }
};

#endif /* Pixmap_h */
