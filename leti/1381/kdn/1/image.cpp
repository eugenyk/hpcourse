#include <random>
#include "image.h"

Image::Image()
{
    width = 0;
    height = 0;
    pixels = nullptr;
}

Image::Image(int w, int h)
{
    width = w;
    height = h;
    pixels = new Brightness[w * h];
    random();
}

Image::~Image()
{
    if (pixels != nullptr) {
        delete[] pixels;
    }
}

void Image::random()
{
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dis(0, 255);
    for (int i = 0; i < height; i++) {
        for (int j = 0; j < width; j++) {
            pixels[i * width + j] = dis(gen);
        }
    }
}

std::ostream& operator<<(std::ostream& out, const Image& image)
{
    if (image.width * image.height == 0) {
        return out;
    }
    for (int i = 0; i < image.height; i++) {
        for (int j = 0; j < image.width; j++) {
            out << (int)image.pixels[i * image.width + j] << " ";
        }
        out << "\n";
    }
    return out;
}
