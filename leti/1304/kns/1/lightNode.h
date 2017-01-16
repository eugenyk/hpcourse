#ifndef TBB_LAB_HighlightNode_H
#define TBB_LAB_HighlightNode_H

#include "utils.h"
#include "tbb/tbb.h"
#include "tbb/parallel_for.h"
#include "tbb/blocked_range.h"
#include "ImageMagick-6/Magick++.h"
#include "utils.h"
#include <vector>
#include <iostream>

using namespace std;
using namespace tbb;
using namespace tbb::flow;
using namespace Magick;

typedef tuple<Image*, PixelsType,PixelsType,PixelsType> HighlightInputType;


/**
 Allows to draw rectangles on a given image.
 red rect for pixel with maximum brightness; green - with minimum brightness; blue - according to given value from command line.
 */
class HighlightNode {
protected:
    CommandLineInput& args;
public:

    HighlightNode(CommandLineInput& args) : args(args) {}

    Image* operator()(HighlightInputType input) {
        int n = task_scheduler_init::default_num_threads();
        auto image = new Image(*get<0>(input));
        PixelsType maxBrightPixels = get<1>(input);
        PixelsType minBrightPixels = get<2>(input);
        PixelsType givenBrightPixels = get<3>(input);


        image->strokeWidth(2);
        auto fillColor = Color("white");
        fillColor.alpha(1.0);
        image->fillColor(fillColor);

        image->strokeColor("red");
        parallel_for(blocked_range<size_t>(0, maxBrightPixels.size(), maxBrightPixels.size()/n), [&image, &maxBrightPixels](const blocked_range<size_t> &r) {
            for (size_t i = r.begin(); i != r.end(); ++i) {
                Utils::highlightPixel(image, maxBrightPixels[i]);
            }
        });

        image->strokeColor("green");
        parallel_for(blocked_range<size_t>(0, minBrightPixels.size(), minBrightPixels.size()/n), [&image, &minBrightPixels](const blocked_range<size_t> &r) {
            for (size_t i = r.begin(); i != r.end(); ++i) {
                Utils::highlightPixel(image, minBrightPixels[i]);
            }
        });

        image->strokeColor("blue");
        parallel_for(blocked_range<size_t>(0, givenBrightPixels.size(), givenBrightPixels.size()/n), [&image, &givenBrightPixels](const blocked_range<size_t> &r) {
            for (size_t i = r.begin(); i != r.end(); ++i) {
                Utils::highlightPixel(image, givenBrightPixels[i]);
            }
        });

        auto fileName = "highlight_" + Utils::imageName(image);
        image->write(args.output + fileName);
        delete image;

        if (args.verbose)
            cout<<"Image " << fileName << " has been written to output dir" <<endl;


        return get<0>(input);
    }
};

#endif