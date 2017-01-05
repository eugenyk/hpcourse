#include "ImageInverser.h"
#include <functional>
#include <fstream>

unsigned char * ImageInverser::inverseImageLine(tuple<unsigned char*, unsigned int,
                                                      unsigned int> lineInfo) {
    unsigned char * line = get<0>(lineInfo);
    // Invers each pixel in line.
    for (unsigned int i = 0; i < get<1>(lineInfo); i++) {
        line[i] = 255 - line[i];
    }
    return line;
}

tuple<Image *, unsigned long> ImageInverser::operator()(Image * image) {
    // Create new image as copy of getted.
    Image *inverseImage = new Image(*image);

    // Run task using graph.
    runTaskOnGraph(inverseImage, std::bind(&ImageInverser::inverseImageLine, 
                                           this, std::placeholders::_1),
                   [](unsigned char*)->continue_msg { return continue_msg(); });
    
    return tuple<Image *, unsigned long>(inverseImage, image->getId());
}
