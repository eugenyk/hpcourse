//
//  utils.cpp
//  ImageProcessing
//
//  Created by Anton Davydov on 17/12/2016.
//
//

#include "utils.h"
#include "commandline.h"
#include <dirent.h>
#include <iostream>
using namespace Magick;

CommandLineInput::CommandLineInput() {
    this->images = {};
    this->inputBrightness = 0;
    this->output = "";
    this->verbose = false;
}

CommandLineInput::CommandLineInput(vector<string> images, BrightnessType inputBrightness, string output) {
    this->images = images;
    this->inputBrightness = inputBrightness;
    this->output = output;
}

/**
 * Read a directory listing into a vector of strings, filtered by file extension.
 * Throws std::exception on error.
 **/
vector<string> Utils::readDirectory(const string &directoryLocation) throw(string)
{
    vector<string> result;
    
    DIR *dir;
    struct dirent *ent;
    
    if ((dir = opendir(directoryLocation.c_str())) == NULL) {
        throw string("Unable to open directory.");
    }
    
    while ((ent = readdir(dir)) != NULL)
    {
        string entry( ent->d_name );
        result.push_back( directoryLocation + entry );
    }
    
    if (closedir(dir) != 0) {
        throw string("Unable to close directory.");
    }
    
    return result;
}


CommandLineInput Utils::parseArgs(int argc, const char * argv[]) throw(string) {
    
    return CommandLineParser<CommandLineInput>()
        .optionalFlag("--verbose", [](auto i){ i.verbose = true; })
        .mandatory<int>("--brightness",
                       [](auto value) throw(string) { auto r = stoi(value);
                           if (r>255||r<0) throw string("--brightness should be [0; 255]"); return r;
                       },
                       [](auto i, auto r){ i.inputBrightness = r; })
        .mandatory<vector<string>>("--input",
                           [](auto value) throw(string) { return Utils::readDirectory(value); },
                           [](auto i, auto r){ i.images = r; })
        .mandatory<string>("--output",
                               [](auto value) throw(string) { return value; },
                               [](auto i, auto r){ i.output = r; })
        .validate(argc, argv);
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
