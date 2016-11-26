#include "ImageLineWorker.h"
class Image;


/// Class for filling one image with random colors.
class ImageFiller : ImageLineWorker<Image*, unsigned char*> {
    /// Fill one line of image.
    unsigned char* fillImageLine(tuple<unsigned char*, unsigned int, unsigned int> lineInfo);
public:
    /// Main action - fill image with different colors.
    Image *operator()(Image* image);
};
