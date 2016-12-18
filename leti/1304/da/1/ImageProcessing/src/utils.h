//
//  utils.h
//  ImageProcessing
//
//  Created by Anton Davydov on 17/12/2016.
//
//

#ifndef utils_h
#define utils_h
#include <vector>
#include <string>
#include "ImageMagick-6/Magick++.h"
using namespace std;

typedef uint8_t BrightnessType;
typedef tuple<int, int> PixelType;
typedef vector<PixelType> PixelsType;

struct CommandLineInput {
    // Mandatory fields
    vector<string> images;
    BrightnessType inputBrightness;
    string output;
    
    // Optional fields
    bool verbose;
    
    CommandLineInput();
    CommandLineInput(vector<string> images, BrightnessType inputBrightness, string output);
};

class Utils {
public:
    static vector<string> readDirectory(const string&) throw(string);
    
    static CommandLineInput parseArgs(int argc, const char * argv[]) throw(string);
    
    static BrightnessType brightnessOfPixelColor(const Magick::Color& color);
    
    static void highlightPixel(Magick::Image*, PixelType);
    
    static void inverseBrightnessOfPixel(Magick::PixelPacket*, int);
    
    static string imageName(Magick::Image*);
};

#endif /* utils_h */
