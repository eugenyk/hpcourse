#include "ImageInverser.h"
#include <functional>

unsigned char * ImageInverser::inverseImageLine(tuple<unsigned char*, unsigned int, unsigned int> lineInfo)
{
    unsigned char * line = get<0>(lineInfo);
    for (unsigned int i = 0; i < get<1>(lineInfo); i++) {
        line[i] = 255 - line[i];
    }
    return line;
}

Image * ImageInverser::operator()(Image * image)
{
    Image *inverseImage = new Image(*image);
    runTaskOnGraph(inverseImage, std::bind(&ImageInverser::inverseImageLine, this, std::placeholders::_1),
        [](unsigned char*)->continue_msg { return continue_msg(); });
    return inverseImage;
}
