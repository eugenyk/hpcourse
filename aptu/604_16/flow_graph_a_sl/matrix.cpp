#include <iostream>
#include <sstream>
#include <map>
#include <string>
#include <ctime>
#include <fstream>
#include <functional>
#include <tbb/flow_graph.h>
#include "random_image.h"


const size_t HEIGHT = 500;
const size_t WIDTH = 500;

typedef std::shared_ptr<RandomImage> ImagePtr;

struct Args {
    size_t brightness;
    size_t images_limit;
    std::shared_ptr<std::ofstream> file;
    size_t check;
};
Args args;

Args parse_args(size_t argc, char* argv[]) {
    Args args = {0, 0, nullptr};
    for (size_t i = 1; i < argc; i += 2) {
        std::string s1(argv[i]);
        std::string s2(argv[i+1]);
        if (s1 == "-b")
        {
            std::stringstream ss;
            ss << s2;
            ss >> args.brightness;
            ++args.check;
        }
        if (s1 == "-l")
        {
            std::stringstream ss;
            ss << s2;
            ss >> args.images_limit;
            ++args.check;
        }
        if (s1 == "-f")
        {
            args.file = std::shared_ptr<std::ofstream>(new std::ofstream(s2));
            ++args.check;
        }
    }
    if (args.check != 3)
    {
        std::cout << "wrong args format\n";
        std::cout << "\tusage: -b [int, brightness] -l [int, images limit] -f [str, output file]" << std::endl;
        exit(1);
    }
    return args;
}


bool source_function(ImagePtr& img_ptr) {
    static size_t images_left = 239;

    if (images_left > 0) {
        --images_left;
        img_ptr = ImagePtr(new RandomImage(HEIGHT, WIDTH));
        return true;
    }
    return false;
}

std::vector<std::pair<size_t, size_t>> find_min(ImagePtr image)
{
    return image->find_min_elements();
}
std::vector<std::pair<size_t, size_t>> find_max(ImagePtr image)
{
    return image->find_max_elements();
}
std::vector<std::pair<size_t, size_t>> eq_function(ImagePtr image)
{
    return image->find_eq_elements(args.brightness);
}

ImagePtr marking_function(std::tuple< std::vector<std::pair<size_t, size_t>>, 
                                      std::vector<std::pair<size_t, size_t>>, 
                                      std::vector<std::pair<size_t, size_t>>, 
                                      ImagePtr > tpl) {
    ImagePtr image = std::get<3>(tpl);
    image->mark_elements(std::get<0>(tpl));
    image->mark_elements(std::get<1>(tpl));
    image->mark_elements(std::get<2>(tpl));
    return image;
}

ImagePtr inverse_function(ImagePtr image) {
    image->inverse();
    return image;
}

double mean_function(ImagePtr image)
{
    return image->mean();
}

double write_function(double mean) {
    *args.file << mean << std::endl;
    return mean;
}

int main(int argc, char* argv[]) {
    using namespace tbb::flow;
    using namespace std::placeholders;

    std::srand(std::time(0));
    args = parse_args(argc, argv);
    
    
    graph g;
    source_node<ImagePtr> source_node(g, source_function);
    limiter_node<ImagePtr> limiter_node_1(g, args.images_limit);
    broadcast_node<ImagePtr> broadcast_node_1(g);

    function_node<ImagePtr, std::vector<std::pair<size_t, size_t>>> min_node(g, unlimited, find_min);
    function_node<ImagePtr, std::vector<std::pair<size_t, size_t>>> max_node(g, unlimited, find_max);
    function_node<ImagePtr, std::vector<std::pair<size_t, size_t>>> eq_node(g, unlimited, eq_function);

    join_node<std::tuple<std::vector<std::pair<size_t, size_t>>,
                         std::vector<std::pair<size_t, size_t>>, 
                         std::vector<std::pair<size_t, size_t>>, 
                         ImagePtr>, queueing> join_node_1(g);

    function_node<std::tuple<std::vector<std::pair<size_t, size_t>>,
                             std::vector<std::pair<size_t, size_t>>,
                             std::vector<std::pair<size_t, size_t>>,
                             ImagePtr>, ImagePtr> marking_node(g, unlimited, marking_function);
    
    broadcast_node<ImagePtr> broadcast_node_2(g);

    function_node<ImagePtr, double> mean_node(g, unlimited, mean_function);

    join_node<std::tuple<double, ImagePtr, ImagePtr>, queueing> join_node_2(g);
    function_node<double, double> write_node(g, 1, write_function);

    function_node<ImagePtr, ImagePtr> inverse_node(g, unlimited, inverse_function);

    function_node<std::tuple<double, ImagePtr, ImagePtr>, continue_msg> deleter_node(
        g, 
        unlimited,
        [](std::tuple<double, ImagePtr, ImagePtr> tpl) {
            return continue_msg();
        });


    make_edge(source_node, limiter_node_1);
    make_edge(limiter_node_1, broadcast_node_1);

    make_edge(broadcast_node_1, min_node);
    make_edge(broadcast_node_1, max_node);
    make_edge(broadcast_node_1, eq_node);

    make_edge(min_node, std::get<0>(join_node_1.input_ports()));
    make_edge(max_node, std::get<1>(join_node_1.input_ports()));
    make_edge(eq_node,  std::get<2>(join_node_1.input_ports()));
    make_edge(broadcast_node_1, std::get<3>(join_node_1.input_ports()));

    make_edge(join_node_1, marking_node);
    make_edge(marking_node, broadcast_node_2);

    make_edge(broadcast_node_2, inverse_node);
    make_edge(broadcast_node_2, mean_node);
    make_edge(mean_node, write_node);

    make_edge(write_node,       std::get<0>(join_node_2.input_ports()));
    make_edge(inverse_node,     std::get<1>(join_node_2.input_ports()));
    make_edge(broadcast_node_2, std::get<2>(join_node_2.input_ports()));

    make_edge(join_node_2, deleter_node);
    make_edge(deleter_node, limiter_node_1.decrement);
    
    g.wait_for_all();

    

    return 0;
}