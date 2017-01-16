#ifndef Pixel_hpp
#define Pixel_hpp

#include <iostream>

class Pixel {
private:
    int r, g, b;
public:
    Pixel() {
        r = rand() % 255;
        g = rand() % 255;
        b = rand() % 255;
    }
    int getR() { return r; }
    int getG() { return g; }
    int getB() { return b; }
};

#endif /* Pixel_hpp */
