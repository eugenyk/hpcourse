#include "ImageLineWorker.h"
class Image;

class AverageBrightnessCounter : ImageLineWorker<tuple<Image*, double>, unsigned long long> {
    unsigned long long summaryBrightness(tuple<unsigned char*, unsigned int, unsigned int> lineInfo);
public:
    tuple<Image*, double> operator()(Image* image);
};
