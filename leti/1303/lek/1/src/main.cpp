#include "ImageGenerator.h"
#include <iostream>
#include "ExtraInfo.h"
#include "ImageTransformer.h"

unsigned char ExtraInfo::currentBrightness = 0;
bool ExtraInfo::debugMode = false;

int main(int argc, char **argv) {
    unsigned char brightnessValue = 0;
    size_t imagesLimit = ExtraInfo::THREADS_NUM; // default threads number
    char *brightnessFile = nullptr;

    // Default values for size of generated images.
    unsigned int minWidth = 50;
    unsigned int minHeight = 50;
    unsigned int maxWidth = 256;
    unsigned int maxHeight = 256;
    unsigned int generatedImageNumber = 100;
    char *pEnd;
    // Read command line options.
    for (int i = 1; i < argc; i+=2) {
        if (i == argc - 1) {
            if (strcmp(argv[i], "-l") != 0 && strcmp(argv[i], "-b") != 0 &&
                strcmp(argv[i], "-f") != 0 && strcmp(argv[i], "-d") != 0 &&
                strcmp(argv[i], "-n") != 0 && strcmp(argv[i], "-maxW") != 0 &&
                strcmp(argv[i], "-minW") != 0 && strcmp(argv[i], "-maxH") != 0 &&
                strcmp(argv[i], "-minH") != 0)
                std::cout << "Uknown option " << argv[i];
            else
                std::cout << "Option " << argv[i]  << " should have value.";
            return 1;
        }
        if (strcmp(argv[i], "-l") == 0) {
            imagesLimit = std::strtoull(argv[i + 1], &pEnd, 10);
        } else if (strcmp(argv[i], "-b") == 0) {
            brightnessValue = atoi(argv[i + 1]);
        } else if (strcmp(argv[i], "-f") == 0) {
            brightnessFile = argv[i + 1];
        } else if (strcmp(argv[i], "-d") == 0) {
            if (strcmp(argv[i + 1], "on") == 0) {
                ExtraInfo::debugMode = true;
            }
        } else if (strcmp(argv[i], "-n") == 0) {
            generatedImageNumber = std::strtoul(argv[i + 1], &pEnd, 10);
        } else if (strcmp(argv[i], "-maxW") == 0) {
            maxWidth = std::strtoul(argv[i + 1], &pEnd, 10);
        } else if (strcmp(argv[i], "-minW") == 0) {
            minWidth = std::strtoul(argv[i + 1], &pEnd, 10);
        } else if (strcmp(argv[i], "-maxH") == 0) {
            maxHeight = std::strtoul(argv[i + 1], &pEnd, 10);
        } else if (strcmp(argv[i], "-minH") == 0) {
            minHeight = std::strtoul(argv[i + 1], &pEnd, 10);
        }
    }

    ExtraInfo::currentBrightness = brightnessValue;
    
    // Generate images.
    ImageGenerator generator(minWidth, minHeight, maxWidth, maxHeight);
    ImageTransformer transformer;

    // Transform images.
    transformer.transform(generator, imagesLimit, brightnessFile, generatedImageNumber);
    return 0;
}