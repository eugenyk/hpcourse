#include "imagegenerator.h"

ImageGenerator::ImageGenerator()
{

}

bool ImageGenerator::operator()(Image* & output)
{
    output = new Image();
    return true;
}
