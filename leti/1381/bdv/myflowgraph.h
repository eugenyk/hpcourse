#ifndef MYFLOWGRAPH_H
#define MYFLOWGRAPH_H

#include "structs.h"
#include "myflowgraph_types.h"
#include <fstream>

class MyFlowGraph
{
private:
    std::vector<image> imgs;
    int images_limit;
    unsigned char br;
    bool logging;
    std::string log_file;

    static std::pair<uchar, uchar> find_minmax_value(image img);
    static tbb::concurrent_vector<pixel> find_elements(image img, int value);
    static void extend_pixels(image img, tbb::concurrent_vector<pixel> pixels);
    static void extend_pix(image img, pixel p);
    static double inverse_and_avgbr(image img);
    void write_avgs_to_file(tbb::flow::buffer_node<img_avgbr>& node);

public:
    MyFlowGraph(graph_options opt);
    ~MyFlowGraph();

    void run();
};

#endif // MYFLOWGRAPH_H
