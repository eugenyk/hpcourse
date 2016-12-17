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
    this->images.push_back("");
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
