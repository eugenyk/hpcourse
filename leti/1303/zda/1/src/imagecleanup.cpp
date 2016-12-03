#include "imagecleanup.h"

ImageCleanup::ImageCleanup()
{

}

double ImageCleanup::operator()(MeanAndInvertedResults results)
{
    ImageWithMean imageWithMean = results.get<0>();
    ImageWithInverted imageWithInverted= results.get<1>();
    double mean = imageWithMean.second;
    delete imageWithInverted.first;
    delete imageWithInverted.second;
    return mean;
}
