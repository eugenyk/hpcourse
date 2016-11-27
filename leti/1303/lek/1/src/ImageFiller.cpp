#include "ImageFiller.h"
#include "Image.h"
#include "ExtraInfo.h"
#include <functional>
#include <random>
#include <chrono>
#include <iostream>

unsigned char* ImageFiller::fillImageLine(tuple<unsigned char*, unsigned int, unsigned int> lineInfo) {
    unsigned seed = std::chrono::system_clock::now().time_since_epoch().count();
    std::default_random_engine generator (seed);
    std::uniform_int_distribution<unsigned char> distribution(0,255);
    unsigned char * line  = get<0>(lineInfo);
    // Generate random value of pixel color.
    for (unsigned int i = 0; i < get<1>(lineInfo); i++) {
        line[i] = distribution(generator);
    }
    return line;
}

Image *ImageFiller::operator()(Image* image) {
    // Set colors to pixels with graph.
    runTaskOnGraph(image, std::bind(&ImageFiller::fillImageLine, 
                                    this, std::placeholders::_1), 
                   [](unsigned char*)->continue_msg { return continue_msg(); });
    return image;
}