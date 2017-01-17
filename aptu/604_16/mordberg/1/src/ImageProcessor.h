//
// Created by antonpp on 16.01.17.
//

#ifndef AU_PARALLEL_COMPUTING_IMAGEPROCESSOR_H
#define AU_PARALLEL_COMPUTING_IMAGEPROCESSOR_H

#include <tbb/flow_graph.h>

#include <string>
#include <vector>
#include <numeric>
#include <algorithm>
#include <fstream>
#include <ostream>
#include <iostream>
#include <memory>

#include "Image.h"

class ImageProcessor {
public:
    ImageProcessor(const std::vector<Image> &images, pixel_t pixel_value, size_t image_parallel,
                   std::string log_fname);

    void process();

private:
    size_t generated_images = 0;

    pixel_t pixel_to_search;
    std::vector<Image> images;
    tbb::flow::graph flow_graph;
    std::ofstream average_pixel_log;
    std::shared_ptr<tbb::flow::source_node<Image>> source_generation_node;

    // we do not need those pointers, but we want to delay destructors call of graph nodes...
    std::vector<std::shared_ptr<tbb::flow::graph_node>> nodes;
};


#endif //AU_PARALLEL_COMPUTING_IMAGEPROCESSOR_H
