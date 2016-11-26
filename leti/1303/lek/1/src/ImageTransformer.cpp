#include "ImageTransformer.h"
#include "ImageHighlighter.h"
#include "Image.h"
#include "ImageInverser.h"
#include "AverageBrightnessCounter.h"
#include <iostream>
#include <fstream>

class FileWriter {
    std::ostream& out;
public:
    FileWriter(std::ostream& out) : out(out) {}
    continue_msg operator()(tuple<Image*, double> imageBrightness) {
        out << "Image " << get<0>(imageBrightness)->getId() << " : " << get<1>(imageBrightness) << std::endl;
        return continue_msg();
    }

};
void ImageTransformer::setBordersColor(const ElementSet& elementSet, unsigned char color,
    ColorMultiMap& pixelColors, Image* image) {
    for (const auto& element : elementSet) {
        if (element.first > 0) {
            pixelColors.emplace(std::pair<unsigned int, unsigned int>(element.first - 1, element.second), color);
            if (element.second > 0) {
                pixelColors.emplace(std::pair<unsigned int, unsigned int>(element.first - 1, element.second - 1), color);
            }
            if (element.second < image->getWidth() - 1) {
                pixelColors.emplace(std::pair<unsigned int, unsigned int>(element.first - 1, element.second + 1), color);
            }
        }
        if (element.first < image->getHeight() - 1) {
            pixelColors.emplace(std::pair<unsigned int, unsigned int>(element.first + 1, element.second), color);
            if (element.second > 0) {
                pixelColors.emplace(std::pair<unsigned int, unsigned int>(element.first + 1, element.second - 1), color);
            }
            if (element.second < image->getWidth() - 1) {
                pixelColors.emplace(std::pair<unsigned int, unsigned int>(element.first + 1, element.second + 1), color);
            }
        }
        if (element.second > 0) {
            pixelColors.emplace(std::pair<unsigned int, unsigned int>(element.first, element.second - 1), color);
        }
        if (element.second < image->getWidth() - 1) {
            pixelColors.emplace(std::pair<unsigned int, unsigned int>(element.first, element.second + 1), color);
        }
    }
}

tuple<ColorMap, Image*> ImageTransformer::countColor(FoundElementsTuple elementsSet) {
    ColorMultiMap pixelColors;
    ColorMap resultColors;
    
    ElementSet max = get<0>(get<0>(elementsSet));
    ElementSet min = get<0>(get<1>(elementsSet));
    ElementSet current = get<0>(get<2>(elementsSet));
    Image* image = get<3>(elementsSet);

    if (get<1>(get<0>(elementsSet)) != image->getId() || get<1>(get<1>(elementsSet)) != image->getId() ||
        get<1>(get<2>(elementsSet)) != image->getId()) {
        return tuple<ColorMap, Image*>();
    }
    setBordersColor(max, 255, pixelColors, image);
    setBordersColor(min, 0, pixelColors, image);
    setBordersColor(min, 127, pixelColors, image);
    for (auto it = pixelColors.begin(), end = pixelColors.end(); it != end;
         it = pixelColors.upper_bound(it->first)) {
        unsigned int colorSum = 0;
        for (auto itValue = pixelColors.equal_range(it->first).first; itValue != pixelColors.equal_range(it->first).second; ++itValue) {
            colorSum += itValue->second;
        }
        resultColors.emplace(it->first, colorSum / pixelColors.count(it->first));
    }
    return tuple<ColorMap, Image*>(resultColors, image);
}

unsigned long ImageTransformer::getId(ElementSetTuple setTuple) {
    return get<1>(setTuple);
}

void ImageTransformer::transform(std::vector<Image*> images, unsigned int imagesLimit, const char* fileName)
{
    std::ostream& out = std::cout;
    std::ofstream fileStream;
    if (fileName != nullptr) {
         fileStream.open(fileName);
    }

    std::function<unsigned char(Image*)> maximum = [](Image* image)->unsigned char {
        unsigned char max;
        unsigned char *maxInLine = new unsigned char[image->getHeight()];
        for (unsigned int i = 0; i < image->getHeight(); i++) {
            maxInLine[i] = *std::max_element(image->getImageLine(i), image->getImageLine(i) + image->getWidth());
        }
        max = *std::max_element(maxInLine, maxInLine + image->getHeight());
        delete[] maxInLine;
        return max;
    };

    std::function<unsigned char(Image*)> minimum = [](Image* image)->unsigned char {
        unsigned char min;
        unsigned char *minInLine = new unsigned char[image->getHeight()];
        for (unsigned int i = 0; i < image->getHeight(); i++) {
            minInLine[i] = *std::min_element(image->getImageLine(i), image->getImageLine(i) + image->getWidth());
        }
        min = *std::min_element(minInLine, minInLine + image->getHeight());
        delete[] minInLine;
        return min;
    };

    std::function<unsigned char(Image*)> curVal = [](Image* image)->unsigned char {
        return ThreadsInfo::currentBrightness;
    };
    graph imageTransformGraph;
    broadcast_node<Image*> input(imageTransformGraph);
    limiter_node<Image*> limiter(imageTransformGraph, imagesLimit);
    function_node<Image*, ElementSetTuple> maxBrightnessFinder(imageTransformGraph, unlimited, FinderOnImage(maximum));
    queue_node<ElementSetTuple> maxSetQueue(imageTransformGraph);
    function_node<Image*, ElementSetTuple> minBrightnessFinder(imageTransformGraph, unlimited, FinderOnImage(minimum));
    queue_node<ElementSetTuple> minSetQueue(imageTransformGraph);
    function_node<Image*, ElementSetTuple> currentBrightnessFinder(imageTransformGraph, unlimited, FinderOnImage(curVal));
    queue_node<ElementSetTuple> curValSetQueue(imageTransformGraph);
    queue_node<Image*> imagesQueue(imageTransformGraph);
    join_node<FoundElementsTuple, key_matching<unsigned long>> join(imageTransformGraph, 
        std::bind(&ImageTransformer::getId, this, std::placeholders::_1),
        std::bind(&ImageTransformer::getId, this, std::placeholders::_1),
        std::bind(&ImageTransformer::getId, this, std::placeholders::_1),
        [](Image* image)->unsigned long { return image->getId(); });
    function_node<FoundElementsTuple, tuple<ColorMap, Image*>> colorCounter(
        imageTransformGraph, unlimited, std::bind(&ImageTransformer::countColor, this, std::placeholders::_1));
    function_node<tuple<ColorMap, Image*>, Image*> highlighter(imageTransformGraph,
        unlimited, ImageHighlighter());
    function_node<Image*, Image*> inverseNode(imageTransformGraph, unlimited, ImageInverser());
    function_node<Image*, tuple<Image*, double>> averageBrightness(imageTransformGraph, unlimited, AverageBrightnessCounter());
    function_node<tuple<Image*, double>, continue_msg> fileOutput(imageTransformGraph, serial, FileWriter(fileName ? fileStream : out));
    function_node<Image*, continue_msg> output(
        imageTransformGraph, serial, [](Image*)->continue_msg { std::cout << "aaa\n"; return continue_msg(); });
    make_edge(input, limiter);
    make_edge(limiter, maxBrightnessFinder);
    make_edge(limiter, minBrightnessFinder);
    make_edge(limiter, currentBrightnessFinder);
    make_edge(maxBrightnessFinder, maxSetQueue);
    make_edge(minBrightnessFinder, minSetQueue);
    make_edge(currentBrightnessFinder, curValSetQueue);
    make_edge(limiter, imagesQueue);
    make_edge(maxSetQueue, input_port<0>(join));
    make_edge(minSetQueue, input_port<1>(join));
    make_edge(curValSetQueue, input_port<2>(join));
    make_edge(imagesQueue, input_port<3>(join));
    make_edge(join, colorCounter);
    make_edge(colorCounter, highlighter);
    make_edge(highlighter, inverseNode);
    make_edge(highlighter, averageBrightness);
    make_edge(averageBrightness, fileOutput);
    //make_edge(inverseNode, output);
    make_edge(fileOutput, limiter.decrement);
    for (auto image : images) {
        input.try_put(image);
    }
    imageTransformGraph.wait_for_all();
    fileStream.close();
    return;
}
