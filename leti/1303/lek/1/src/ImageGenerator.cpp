#include "ImageGenerator.h"
#include "ImageFiller.h"
#include "Image.h"
#include "ThreadsInfo.h"
#include <functional>
#include <iostream>


ImageGenerator::ImageGenerator(unsigned int minWidth, unsigned int minHeight,
                               unsigned int maxWidth, unsigned int maxHeight) : 
                minWidth(minWidth), minHeight(minHeight), maxWidth(maxWidth),
                maxHeight(maxHeight) {}

continue_msg ImageGenerator::addImage(Image* image) {
    imagesCollection.push_back(image);
    return continue_msg();
}

std::vector<Image*> ImageGenerator::generate(unsigned int count) {
    // Remove old generated images.
    imagesCollection.clear();

    graph imageGeneratorGraph;
    broadcast_node<Image*> input(imageGeneratorGraph);
    // Limit number of created working nodes.
    limiter_node<Image*> limiter(imageGeneratorGraph, ThreadsInfo::THREADS_INC_COEF * ThreadsInfo::THREADS_NUM);
    // Node for filling image.
    function_node<Image*, Image*> imageFiller(imageGeneratorGraph, unlimited, ImageFiller());
    buffer_node<Image*> buffer(imageGeneratorGraph);
    // Save images in one vector.
    function_node<Image*, continue_msg> collector(imageGeneratorGraph, serial,
        std::bind(&ImageGenerator::addImage, this, std::placeholders::_1));

    // Add edges.
    make_edge(input, limiter);
    make_edge(limiter, imageFiller);
    make_edge(imageFiller, buffer);
    make_edge(buffer, collector);
    make_edge(collector, limiter.decrement);

    // Generate images of different size.
    std::random_device rd;
    for (unsigned int i = 0; i < count; i++) {
        Image* newImage = new Image(rd() % (maxWidth - minWidth) + minWidth,
            rd() % (maxHeight - minHeight) + minHeight);
        input.try_put(newImage);
    }

    // Run filling of all images.
    imageGeneratorGraph.wait_for_all();
    return imagesCollection;
}