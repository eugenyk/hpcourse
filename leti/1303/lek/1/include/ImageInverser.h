#include "ImageLineWorker.h"

class Image;

/// Class for getting inverse image.
class ImageInverser : ImageLineWorker<tuple<Image *, unsigned long>, unsigned char*> {
    unsigned char* inverseImageLine(tuple<unsigned char*, unsigned int, unsigned int> lineInfo);
public:
    tuple<Image *, unsigned long> operator()(Image* image);
};
