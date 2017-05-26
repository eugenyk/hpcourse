#include "ImageGenerator.h"
#include "ImageFiller.h"
#include "Image.h"
#include "ExtraInfo.h"
#include <functional>
#include <chrono>
#include <iostream>


ImageGenerator::ImageGenerator(unsigned int minWidth, unsigned int minHeight,
                               unsigned int maxWidth, unsigned int maxHeight) : 
                minWidth(minWidth), minHeight(minHeight), maxWidth(maxWidth),
                maxHeight(maxHeight) {}

Image* ImageGenerator::generate() const {
    unsigned seed = std::chrono::system_clock::now().time_since_epoch().count();
    std::default_random_engine generator (seed);
    std::uniform_int_distribution<unsigned long> widthDistribution(minWidth,maxWidth);
    std::uniform_int_distribution<unsigned long> heightDistribution(minHeight,maxHeight);
    // Generate images of different size.
    unsigned long width = widthDistribution(generator);
    unsigned long height = heightDistribution(generator);
    Image* newImage = new Image(width, height);
    ImageFiller filler;
    filler(newImage);
    // Run filling of all images.
    return newImage;
}