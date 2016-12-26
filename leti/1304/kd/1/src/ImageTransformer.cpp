#include "ImageTransformer.h"
#include "ImageHighlighter.h"
#include "Image.h"
#include "ImageInverser.h"
#include "AverageBrightnessCounter.h"
#include "ImageGenerator.h"
#include <iostream>
#include <fstream>
#include <assert.h>

/// Class for writing output to stream.
class FileWriter {
    /// Output stream.
    std::ostream& out;
public:
    FileWriter(std::ostream& out) : out(out) {}

    /// Write image brightness.
    Image* operator()(tuple<Image*, double> imageBrightness) {
        out << "Image " << get<0>(imageBrightness)->getId() << " : " 
            << get<1>(imageBrightness) << std::endl;
        return get<0>(imageBrightness);
    }

};

void ImageTransformer::setBordersColor(const ElementSet& elementSet, unsigned char color,
                                       ColorMultiMap& pixelColors, const Image* image) {
    // For all found elements find borders and set color to them.
    for (const auto& element : elementSet) {
        // Color upper border.
        if (element.first > 0) {
            pixelColors.emplace(std::pair<unsigned int, unsigned int>
                                    (element.first - 1, element.second), color);
            if (element.second > 0) {
                pixelColors.emplace(std::pair<unsigned int, unsigned int>
                                        (element.first - 1, element.second - 1), color);
            }
            if (element.second < image->getWidth() - 1) {
                pixelColors.emplace(std::pair<unsigned int, unsigned int>
                                        (element.first - 1, element.second + 1), color);
            }
        }
        // Color bottom border.
        if (element.first < image->getHeight() - 1) {
            pixelColors.emplace(std::pair<unsigned int, unsigned int>
                                    (element.first + 1, element.second), color);
            if (element.second > 0) {
                pixelColors.emplace(std::pair<unsigned int, unsigned int>
                                        (element.first + 1, element.second - 1), color);
            }
            if (element.second < image->getWidth() - 1) {
                pixelColors.emplace(std::pair<unsigned int, unsigned int>
                                        (element.first + 1, element.second + 1), color);
            }
        }

        // Color left border.
        if (element.second > 0) {
            pixelColors.emplace(std::pair<unsigned int, unsigned int>
                                    (element.first, element.second - 1), color);
        }

        // Color right border.
        if (element.second < image->getWidth() - 1) {
            pixelColors.emplace(std::pair<unsigned int, unsigned int>
                                    (element.first, element.second + 1), color);
        }
    }
}

tuple<ColorMap, Image*> ImageTransformer::countColor(FoundElementsTuple elementsSet) {
    ColorMultiMap pixelColors;
    ColorMap resultColors;
    
    // Separate tuple.
    ElementSet max = get<0>(get<0>(elementsSet));
    ElementSet min = get<0>(get<1>(elementsSet));
    ElementSet current = get<0>(get<2>(elementsSet));
    Image* image = get<3>(elementsSet);

    // Check ids of all getting data.
    assert (get<1>(get<0>(elementsSet)) == image->getId() && 
            get<1>(get<1>(elementsSet)) == image->getId() &&
            get<1>(get<2>(elementsSet)) == image->getId());

    // Set borders for each found set.
    setBordersColor(max, 255, pixelColors, image);
    setBordersColor(min, 0, pixelColors, image);
    setBordersColor(current, 127, pixelColors, image);

    // Fins result color of pixels as average of all colors.
    for (auto it = pixelColors.begin(), end = pixelColors.end(); it != end;
         it = pixelColors.upper_bound(it->first)) {
        unsigned int colorSum = 0;
        for (auto itValue = pixelColors.equal_range(it->first).first; 
             itValue != pixelColors.equal_range(it->first).second; ++itValue) {
            colorSum += itValue->second;
        }
        resultColors.emplace(it->first, colorSum / pixelColors.count(it->first));
    }

    return tuple<ColorMap, Image*>(resultColors, image);
}

unsigned long ImageTransformer::getId(ElementSetTuple setTuple) {
    return get<1>(setTuple);
}

void ImageTransformer::transform(const ImageGenerator& generator, size_t imagesLimit,
                                 const char* fileName, unsigned int count) {
    // Set output stream.
    std::ostream& out = std::cout;
    std::ofstream fileStream;
    if (fileName != nullptr) {
         fileStream.open(fileName);
    }

    // Function for find maximum pixel brightness on image.
    std::function<unsigned char(Image*)> maximum = [](Image* image)->unsigned char {
        unsigned char max;
        unsigned char *maxInLine = new unsigned char[image->getHeight()];
        // Find maximum for each line.
        for (unsigned int i = 0; i < image->getHeight(); i++) {
            maxInLine[i] = *std::max_element(image->getImageLine(i), 
                                             image->getImageLine(i) + image->getWidth());
        }
        // Find result maximum.
        max = *std::max_element(maxInLine, maxInLine + image->getHeight());
        delete[] maxInLine;
        return max;
    };

    // Function for find minimum pixel brightness on image.
    std::function<unsigned char(Image*)> minimum = [](Image* image)->unsigned char {
        unsigned char min;
        unsigned char *minInLine = new unsigned char[image->getHeight()];
        // Find mimimum for each line.
        for (unsigned int i = 0; i < image->getHeight(); i++) {
            minInLine[i] = *std::min_element(image->getImageLine(i), image->getImageLine(i) + image->getWidth());
        }
        // Find result mimimum.
        min = *std::min_element(minInLine, minInLine + image->getHeight());
        delete[] minInLine;
        return min;
    };

    /// Function for getting user set current brightness.
    std::function<unsigned char(Image*)> curVal = [](Image* image)->unsigned char {
        return ExtraInfo::currentBrightness;
    };

    // Create flow graph.
    graph imageTransformGraph;

    broadcast_node<Image*> input(imageTransformGraph);
    buffer_node<Image*> buffer(imageTransformGraph);
    // Limit number of images.
    limiter_node<Image*> limiter(imageTransformGraph, imagesLimit);
    function_node<Image*, Image*> logger(imageTransformGraph,
        serial, [](Image* image)->Image* {
            std::ofstream out("Image" + std::to_string(image->getId()) + ".log");
            image->log(out);
            out.close();
            return image;
    });

    // Nodes for finding sets of special elements.
    function_node<Image*, ElementSetTuple> maxBrightnessFinder(imageTransformGraph, unlimited,
                                                               FinderOnImage(maximum));
    queue_node<ElementSetTuple> maxSetQueue(imageTransformGraph);

    function_node<Image*, ElementSetTuple> minBrightnessFinder(imageTransformGraph, unlimited,
                                                               FinderOnImage(minimum));
    queue_node<ElementSetTuple> minSetQueue(imageTransformGraph);

    function_node<Image*, ElementSetTuple> currentBrightnessFinder(imageTransformGraph, unlimited,
                                                                   FinderOnImage(curVal));
    queue_node<ElementSetTuple> curValSetQueue(imageTransformGraph);

    // Queue for saving images.
    queue_node<Image*> imagesQueue(imageTransformGraph);

    // Join all getted sets and image.
    join_node<FoundElementsTuple, key_matching<unsigned long>> join(imageTransformGraph, 
        std::bind(&ImageTransformer::getId, this, std::placeholders::_1),
        std::bind(&ImageTransformer::getId, this, std::placeholders::_1),
        std::bind(&ImageTransformer::getId, this, std::placeholders::_1),
        [](Image* image)->unsigned long { return image->getId(); });

    function_node<FoundElementsTuple, FoundElementsTuple> findResultLogger(imageTransformGraph,
        serial, [](FoundElementsTuple setTuple)->FoundElementsTuple {
            auto id = get<3>(setTuple)->getId();
            std::cout << "Image " << id << std::endl;
            
            std::function<void(ElementSet)> logSet = [](ElementSet set)->void {
                for (const auto &element : set) {
                    std::cout << " { " << element.first << " , " << element.second << " } ";
                }
                std::cout << "]" << std::endl;
            };
            std::cout << "Maximums : [";
            logSet(get<0>(get<0>(setTuple)));
            std::cout << "Minimums : [";
            logSet(get<0>(get<1>(setTuple)));
            std::cout << "Current value = " << (int) ExtraInfo::currentBrightness << " : [";
            logSet(get<0>(get<2>(setTuple)));
            return setTuple;
    });

    // Count result colors for image.
    function_node<FoundElementsTuple, tuple<ColorMap, Image*>> colorCounter(
        imageTransformGraph, unlimited, std::bind(&ImageTransformer::countColor,
                                                  this, std::placeholders::_1));

    // Higlight with counted colors found pixels borders.
    function_node<tuple<ColorMap, Image*>, Image*> highlighter(imageTransformGraph,
                                                               unlimited, ImageHighlighter());

    // Get inverse image.
    function_node<Image*, tuple<Image*, unsigned long>> inverseNode(imageTransformGraph, unlimited, ImageInverser());

    join_node<tuple<tuple<Image*, unsigned long>, Image*>, key_matching<unsigned long>> outputJoin(imageTransformGraph,
        [](tuple<Image*, unsigned long> image)->unsigned long { return get<1>(image); },
        [](Image* image)->unsigned long { return image->getId(); });

    function_node<tuple<tuple<Image*, unsigned long>, Image*>, continue_msg> output(imageTransformGraph, serial,
        [](tuple<tuple<Image*, unsigned long>, Image*> image)->continue_msg {
        if (ExtraInfo::debugMode) {
            std::ofstream out("ResultImages" + std::to_string(get<1>(image)->getId()) + ".log");
            out << "Higlight ";
            get<1>(image)->log(out);
            out << "Inverse ";
            get<0>(get<0>(image))->log(out);
            out.close();
        }
        delete get<0>(get<0>(image));
        delete get<1>(image);
        return continue_msg();
    });

    // Calculate average brightness of image.
    function_node<Image*, tuple<Image*, double>> averageBrightness(imageTransformGraph, unlimited,
                                                                   AverageBrightnessCounter());

    // Output calculated brightness.
    function_node<tuple<Image*, double>, Image*> fileOutput(imageTransformGraph, serial,
                                                       FileWriter(fileName ? fileStream : out));

    // Create edges.
    if (ExtraInfo::debugMode) {
        make_edge(input, logger);
        make_edge(logger, buffer);
    } else {
        make_edge(input, buffer);
    }
    
    make_edge(buffer, limiter);
    
    make_edge(limiter, maxBrightnessFinder);
    make_edge(limiter, minBrightnessFinder);
    make_edge(limiter, currentBrightnessFinder);
    make_edge(limiter, imagesQueue);

    make_edge(maxBrightnessFinder, maxSetQueue);
    make_edge(minBrightnessFinder, minSetQueue);
    make_edge(currentBrightnessFinder, curValSetQueue);
    
    make_edge(maxSetQueue, input_port<0>(join));
    make_edge(minSetQueue, input_port<1>(join));
    make_edge(curValSetQueue, input_port<2>(join));
    make_edge(imagesQueue, input_port<3>(join));

    make_edge(join, colorCounter);
    if (ExtraInfo::debugMode) {
        make_edge(join, findResultLogger);
    }
    make_edge(colorCounter, highlighter);

    make_edge(highlighter, inverseNode);
    make_edge(inverseNode, input_port<0>(outputJoin));
    make_edge(highlighter, averageBrightness);

    make_edge(averageBrightness, fileOutput);
    make_edge(fileOutput, input_port<1>(outputJoin));
    make_edge(outputJoin, output);
    make_edge(output, limiter.decrement);

    // Put images to graph.
    for (unsigned int i = 0; i < count; i++) {
        Image *image = generator.generate();
        input.try_put(image);
    }

    imageTransformGraph.wait_for_all();

    fileStream.close();
}
