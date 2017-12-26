#ifndef LAB03_PROCESS_H
#define LAB03_PROCESS_H

#include <string>
#include "image.h"
#include "elements.h"

struct ImageHighlighter {
    explicit ImageHighlighter(size_t radius = 3);
    typedef std::tuple<ImagePtr, ElementsResultType, ElementsResultType, ElementsResultType> input_type;
    // TODO avoid copying vector of positions.
    ImagePtr operator()(input_type image_and_positions);
    
    size_t radius_;
};

struct ImageInverser {
    ImagePtr operator()(const ImageConstPtr &image);
};

struct MeanBrightnessCalculator {
    typedef double result_type;
    // TODO query_node for writing to file.
    
    explicit MeanBrightnessCalculator(const std::string &filename);
    
    result_type operator()(ImageConstPtr image);
    
    void write(result_type s);
private:
    const std::string &filename_;
};


#endif //LAB03_PROCESS_H
