#include "brightnessinverter.h"

BrightnessInverter::BrightnessInverter()
{

}

ImageWithInverted BrightnessInverter::operator()(Image * img) const
{
    Image * newimg = img->getInvertedImage();
    return  ImageWithInverted(img,newimg);
}
