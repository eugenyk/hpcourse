#include "imagegenerator.h"
#include <cstdlib>

ImageGenerator::ImageGenerator()
{

}

bool ImageGenerator::operator()(Image* & output)
{
    output = new Image(rand()%1000+10,rand()%1000+10);
    return true;
}
