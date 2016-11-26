#include "ImageLineWorker.h"
class Image;

class ImageInverser : ImageLineWorker<Image*, unsigned char*> {
    unsigned char* inverseImageLine(tuple<unsigned char*, unsigned int, unsigned int> lineInfo);
public:
    Image *operator()(Image* image);
};
