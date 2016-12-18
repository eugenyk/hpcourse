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

    static minmax find_minmax_value(image img);
    static selected_pixels find_elements(minmax val, int br);
    static image extend_min(selected_pixels pixs);
    static image extend_max(selected_pixels pixs);
    static image extend_br(selected_pixels pixs);
    static void extend_pix(image img, pixel p);
    static double inverse_and_avgbr(image img);
    void write_avgs_to_file(tbb::flow::buffer_node<img_avgbr>& node);

public:
    MyFlowGraph(graph_options opt);
    ~MyFlowGraph();

    void run();
};

#endif // MYFLOWGRAPH_H
