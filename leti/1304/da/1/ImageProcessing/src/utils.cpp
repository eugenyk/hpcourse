//
//  utils.cpp
//  ImageProcessing
//
//  Created by Anton Davydov on 17/12/2016.
//
//

#include "utils.h"
using namespace Magick;
ConsoleInput::ConsoleInput() {
    this->images = vector<string>();
    this->images.push_back("/Volumes/FlashDrive/Projects/hpcourse/leti/1304/da/1/ImageProcessing/src/3.jpg");
    this->inputBrightness = 100;
    this->output = "";
}

ConsoleInput::ConsoleInput(vector<string> images, BrightnessType inputBrightness, string output) {
    this->images = images;
    this->inputBrightness = inputBrightness;
    this->output = output;
}

Output::Output() {
    
}

ConsoleInput Utils::parseArgs(int argc, const char * argv[]) {
    return ConsoleInput();
}

BrightnessType Utils::brightnessOfPixelColor(const Color& color) {
    auto rgbColor = ColorRGB(color);
    return 255 * ((rgbColor.red() + rgbColor.green() + rgbColor.blue()) / 3.0);
}

void Utils::highlightPixel(Image* image, PixelType p, Color c) {
    auto edge = 10 / 2;
    auto upX = get<0>(p)-edge;
    auto upY = get<1>(p)-edge;
    auto downX = get<0>(p)+edge;
    auto downY = get<1>(p)+edge;
    image->strokeColor(c);
    image->strokeWidth(2);
    auto fillColor = Color("white");
    fillColor.alpha(1.0);
    
    image->fillColor(fillColor);
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
