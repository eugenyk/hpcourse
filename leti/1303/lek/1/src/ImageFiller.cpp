#include "ImageFiller.h"
#include "Image.h"
#include "ThreadsInfo.h"
#include <functional>
#include <random>
#include <iostream>

unsigned char* ImageFiller::fillImageLine(tuple<unsigned char*, unsigned int, unsigned int> lineInfo) {
    std::random_device rd;
    unsigned char * line  = get<0>(lineInfo);
    // Generate random value of pixel color.
    for (unsigned int i = 0; i < get<1>(lineInfo); i++) {
        line[i] = rd() % 256;
    }
    return line;
}

Image *ImageFiller::operator()(Image* image) {
    runTaskOnGraph(image, std::bind(&ImageFiller::fillImageLine, this, std::placeholders::_1), 
                   [](unsigned char*)->continue_msg { return continue_msg(); });
    return image;
}