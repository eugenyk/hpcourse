#include "ImageFiller.h"
#include "Image.h"
#include "ThreadsInfo.h"
#include <functional>
#include <random>
#include <iostream>

// Structure for generating sequence for sequencer_buffer.
struct Sequencer {
    size_t operator()(unsigned char* line) {
        static unsigned int counter = 0;
        counter++;
        return (counter - 1);
    }
};

unsigned char* ImageFiller::fillImageLine(tuple<unsigned char*, unsigned int> lineInfo) {
    std::random_device rd;
    unsigned char * line  = get<0>(lineInfo);
    // Generate random value of pixel color.
    for (unsigned int i = 0; i < get<1>(lineInfo); i++) {
        line[i] = rd() % 256;
    }
    return line;
}

Image *ImageFiller::operator()(Image* image) {
    graph imageFillerGraph;
    broadcast_node<tuple<unsigned char*, unsigned int>> input(imageFillerGraph);
    // Limit number of created working nodes.
    limiter_node<tuple<unsigned char*, unsigned int>> limiter(imageFillerGraph, 
        ThreadsInfo::THREADS_INC_COEF * ThreadsInfo::THREADS_NUM);
    // Node for filling.
    function_node<tuple<unsigned char*, unsigned int>, unsigned char*> lineFiller(imageFillerGraph, unlimited,
        std::bind(&ImageFiller::fillImageLine, this, std::placeholders::_1));
    sequencer_node<unsigned char*> buffer(imageFillerGraph, Sequencer());
    // Output node.
    function_node<unsigned char*, continue_msg> output(imageFillerGraph, serial, 
        [](unsigned char*)->continue_msg{ return continue_msg(); });

    // Add edges to graph.
    make_edge(input, limiter);
    make_edge(limiter, lineFiller);
    make_edge(lineFiller, buffer);
    make_edge(buffer, output);
    make_edge(output, limiter.decrement);

    // Put image lines to graph input.
    for (unsigned int i = 0; i < image->getHeight(); i++) {
        input.try_put(tuple<unsigned char*, unsigned int>(image->getImageLine(i), image->getWidth()));
    }
    imageFillerGraph.wait_for_all();
    return image;
}