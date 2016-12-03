#include "pointhighlighter.h"

PointHighlighter::PointHighlighter()
{

}

Image *PointHighlighter::operator()(ThreeMessages input) const
{
    PointsWithImage maxBrightnessMessage = input.get<0>();
    PointsWithImage minBrightnessMessage = input.get<1>();
    PointsWithImage fixedBrightnessMessage = input.get<2>();

    Image * img = maxBrightnessMessage.second;

    img->highlightPoints(maxBrightnessMessage.first);
    img->highlightPoints(minBrightnessMessage.first);
    img->highlightPoints(fixedBrightnessMessage.first);

    return img;
}
