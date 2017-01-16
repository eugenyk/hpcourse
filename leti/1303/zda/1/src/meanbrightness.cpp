#include "meanbrightness.h"

MeanBrightness::MeanBrightness()
{

}

ImageWithMean MeanBrightness::operator()(Image *img) const
{
    return ImageWithMean(img,img->getMeanBrightness());
}
