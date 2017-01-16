#ifndef MYFLOWGRAPH_H
#define MYFLOWGRAPH_H

#include "structs.h"
#include "myflowgraph_types.h"
#include <fstream>

class MyFlowGraph
{
private:
    int imgs_h;
    int imgs_w;
    int images_limit;
    unsigned char br;
    bool logging;
    std::string log_file;

    static std::pair<uchar, uchar> find_minmax_value(image img);
    static tbb::concurrent_vector<pixel> find_elements(image img, int value);
    static void extend_pixels(image img, tbb::concurrent_vector<pixel> pixels);
    static void extend_pix(image img, pixel p);
    static image inverse_brightness(image img);
    static double calc_avg_br(image img);
    void write_avgs_to_file(tbb::flow::buffer_node<img_avgbr>& node, int num_images);

public:
    MyFlowGraph(graph_options opt);
    ~MyFlowGraph();

    void run();
};

#endif // MYFLOWGRAPH_H
