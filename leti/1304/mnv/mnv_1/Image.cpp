#include "Image.h"
#include <vector>
#include <cstdlib>
#include <cstring>
#include <stdio.h>

int Image::counter = 0;

Image::Image()
{
    counter++;
}

Image::Image(int width, int height) : width(width), height(height)
{
    counter++;
    id = counter;
    picture = new Pixel[width * height];
    fillPixels();
}

Image::Image(int width, int height, int id) : width(width), height(height), id(id)
{
    picture = new Pixel[width * height];
}

Image::Image(int width, int height, int id, Pixel *map) : width(width), height(height), id(id), picture(map)
{
}

Image::Image(const Image & img)
{
    id = img.id;
    width = img.width;
    height = img.height;
    picture = new Pixel[width * height];
    std::memcpy(picture, img.picture, sizeof(Pixel) * width * height);

}

Image::~Image()
{
    delete[] picture;
}

int Image::getId() const
{
    return id;
}

int Image::getWidth() const
{
    return width;
}

int Image::getHeight() const
{
    return height;
}

unsigned char * Image::getPixelMap() const
{
    return picture;
}

void Image::printPixelMap() const
{
    printf("====Print pixels==== \n");
    for (int i = 0; i < height; ++i) {
        for (int j = 0; j < width; ++j) {
            printf("%03d ", picture[i * height + j]);
        }
        printf("\n");
    }
}

void Image::update(int index, Pixel value)
{
    picture[index] = value;
}


void Image::fillPixels()
{
    for (int i = 0; i < width * height; ++i) {
        picture[i] = rand() % 256;
    }
}
