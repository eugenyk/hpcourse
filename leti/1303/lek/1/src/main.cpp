#include "ImageGenerator.h"
#include <iostream>
#include "ExtraInfo.h"
#include "ImageTransformer.h"
unsigned char ExtraInfo::currentBrightness = 0;

int main(int argc, char **argv) {
    unsigned char brightnessValue = 0;
    size_t imagesLimit = ExtraInfo::THREADS_NUM; // default threads number
    char *brightnessFile = nullptr;

    // Read command line options.
    for (int i = 1; i < argc; i+=2) {
        if (i == argc - 1) {
            if (strcmp(argv[i], "-l") != 0 && strcmp(argv[i], "-b") != 0 &&
                strcmp(argv[i], "-f") != 0)
                std::cout << "Uknown option " << argv[i];
            else
                std::cout << "Option " << argv[i]  << " should have value.";
            return 1;
        }
        if (strcmp(argv[i], "-l") == 0) {
            char *pEnd;
            imagesLimit = std::strtoull(argv[i + 1], &pEnd, 10);
        } else if (strcmp(argv[i], "-b") == 0) {
            brightnessValue = atoi(argv[i + 1]);
        } else if (strcmp(argv[i], "-f") == 0) {
            brightnessFile = argv[i + 1];
        }
    }

    ExtraInfo::currentBrightness = brightnessValue;
    
    // Constants for size of generated images.
    const unsigned int minWidth = 50;
    const unsigned int minHeight = 50;
    const unsigned int maxWidth = 256;
    const unsigned int maxHeight = 256;

    const unsigned int generatedImageNumber = 100;

    // Generate images.
    ImageGenerator generator(minWidth, minHeight, maxWidth, maxHeight);
    std::vector<Image*> generatedImages = generator.generate(generatedImageNumber);
    ImageTransformer transformer;

    // Transform images.
    transformer.transform(generatedImages, imagesLimit, brightnessFile);
    for (Image* image : generatedImages) {
        delete image;
    }
    return 0;
}