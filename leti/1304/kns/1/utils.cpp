//
//  utils.cpp
//  ImageProcessing
//
//  Created by Anton Davydov on 17/12/2016.
//
//

#include "utils.h"
#include <dirent.h>
#include <iostream>
using namespace Magick;

CommandLineInput::CommandLineInput() {
    this->images = {};
    this->inputBrightness = 0;
    this->output = "";
    this->verbose = false;
    this->limit = 1;
}

BrightnessType Utils::brightnessOfPixelColor(const Color& color) {
    auto rgbColor = ColorRGB(color);
    return 255 * ((rgbColor.red() + rgbColor.green() + rgbColor.blue()) / 3.0);
}

void Utils::highlightPixel(Image* image, PixelType p) {
    auto edge = 6 / 2;
    auto upX = get<0>(p)-edge;
    auto upY = get<1>(p)-edge;
    auto downX = get<0>(p)+edge;
    auto downY = get<1>(p)+edge;

    image->draw(DrawableRectangle(upX<0?0:upX,
                                  upY<0?0:upY,
                                  downX>=image->columns()?image->columns()-1:downX,
                                  downY>=image->rows()?image->rows()-1:downY));
}

void Utils::inverseBrightnessOfPixel(Magick::PixelPacket* image, int index) {
    image[index].red = 1.0 - image[index].red;
    image[index].green = 1.0 - image[index].green;
    image[index].blue = 1.0 - image[index].blue;
}

string Utils::imageName(Image* im) {
    return im->fileName().substr(im->fileName().find_last_of('/',  string::npos)+1, string::npos);
}