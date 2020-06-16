#include "image.h"

#include <stdlib.h>
#include <cmath>
#include <stdexcept>

image::image(int id, int height, int width)
{
    this->buff = (int *)malloc(height * width * sizeof(int));
    this->id = id;
    this->height = height;
    this->width = width;
}

int image::get_height()
{
    return this->height;
}

int image::get_width()
{
    return this->width;
}

int image::get_id()
{
    return this->id;
}

int image::get_pixel(int row, int col)
{
    return this->buff[this->width * row + col];
}

void image::set_pixel(int row, int col, int val)
{
    this->buff[this->width * row + col] = val;
}
