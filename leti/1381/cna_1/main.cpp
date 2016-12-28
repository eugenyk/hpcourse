#include "image.h"
#include "agroritms.h"
#include "comporator.h"

#include <tbb/task_scheduler_init.h>
#include <tbb/parallel_for.h>
#include <tbb/blocked_range.h>
#include <tbb/flow_graph.h>
#include <random>
#include <ctime>
#include <cstring>
#include <algorithm>
#include <iostream>
#include <tuple>

//using namespace std;
using namespace tbb;
using namespace tbb::flow;

#define TEST std::cout << __PRETTY_FUNCTION__ << "\t GOOD" << std::endl;

int main(int argc, char *argv[])
{
    unsigned char userValue = 100;
    size_t repeated = 5000;
    size_t l = 5;
    char *fileName = 0;

    for(int i = 1; i < argc; i++) {
        if(!strcmp("-b", argv[i]) && i + 1 < argc)
            userValue = atoi(argv[++i]);
        else if(!strcmp("-l", argv[i]) && i + 1 < argc)
            l = atoi(argv[++i]);
        else if(!strcmp("-f", argv[i]) && i + 1 < argc)
            fileName = argv[++i];
    }

    std::cout << "User light value: " << static_cast<int>(userValue) << std::endl;
    std::cout << "Images count:     " << repeated << std::endl;
    std::cout << "Log file name:    " << (fileName != 0 ? fileName : "(empty)") << std::endl;

    srand(time(0));
    tbb::flow::graph graph;

    auto source = [repeated](Image& img) {
        static size_t cntr = 0;
        img = Image(800, 600);
        img.fill();
        cntr++;
        std::cout << "Source: " << cntr << "/" << repeated << "\t" << (cntr < repeated) << std::endl;
        return cntr < repeated;
    };

    auto limiter = [l]() {
        return static_cast<unsigned>(l);
    };

    auto find_minMax = [](const Image &img) {
        std::pair<Image::Container::const_iterator, Image::Container::const_iterator> minMax =
                minmax_element(img.cpixels().begin(), img.cpixels().end());
        unsigned char minValue = minMax.first != img.cpixels().end() ? *minMax.first : 1;
        unsigned char maxValue = minMax.second != img.cpixels().end() ? *minMax.second : UCHAR_MAX;
        return make_tuple(img, std::make_pair(minValue, maxValue));
    };

    auto fmax = [](tuple<Image, std::pair<unsigned char,unsigned char> > arg) {
        auto img = get<0>(arg);
        return make_tuple(img, findPixels(img.cpixels().begin(), img.cpixels().end(), get<1>(arg).first));
    };

    auto fmin = [](tuple<Image, std::pair<unsigned char,unsigned char> > arg) {
        auto img = get<0>(arg);
        return make_tuple(img, findPixels(img.cpixels().begin(), img.cpixels().end(), get<1>(arg).second));
    };

    auto fuser = [userValue](tuple<Image, std::pair<unsigned char,unsigned char> > arg) {
        auto img = get<0>(arg);
        return make_tuple(img, findPixels(img.cpixels().begin(), img.cpixels().end(), userValue));
    };

    auto marked = [](tuple<Image, std::vector<ptrdiff_t> > arg) {
        auto img = get<0>(arg);
        mark(get<1>(arg), img.pixels(), img.width(), img.height(), UCHAR_MAX);
        return img;
    };

    auto inverse_image = [](Image img) {
        bitInverseValues(img.pixels().begin(), img.pixels().end());
        return img;
    };

    auto find_mean_brightness = [](const Image &img) {
        std::cout << "Mean brightness: " << static_cast<int>(meanValue(img.cpixels().begin(), img.cpixels().end(), 0)) << std::endl;
    };

    tbb::flow::source_node<Image> input(graph, source);
    tbb::flow::limiter_node<Image> limit_node(graph, limiter());
    tbb::flow::function_node<Image, tuple<Image, std::pair<unsigned char,unsigned char> > > f_find_minmax(graph, tbb::flow::unlimited, find_minMax);
    tbb::flow::broadcast_node<tuple<Image, std::pair<unsigned char,unsigned char> > > broad_node(graph);
    tbb::flow::function_node<tuple<Image, std::pair<unsigned char,unsigned char> > > f_fmax(graph, tbb::flow::unlimited, fmax);
    tbb::flow::function_node<tuple<Image, std::pair<unsigned char,unsigned char> > > f_fmin(graph, tbb::flow::unlimited, fmin);
    tbb::flow::function_node<tuple<Image, std::pair<unsigned char,unsigned char> > > f_fuser(graph, tbb::flow::unlimited, fuser);
    tbb::flow::join_node<tbb::flow::tuple<std::tuple<Image, std::vector<ptrdiff_t> > > > found_join(graph);
    tbb::flow::function_node<tuple<Image,std::vector<ptrdiff_t> >, Image> f_dist_pixel(graph, tbb::flow::unlimited, marked);
    tbb::flow::function_node<Image> f_invers_image(graph, tbb::flow::unlimited, inverse_image);
    tbb::flow::function_node<Image> f_find_mean(graph, tbb::flow::unlimited, find_mean_brightness);
    tbb::flow::join_node<tbb::flow::tuple<std::vector<ptrdiff_t> > > finish_join(graph);
    tbb::flow::function_node<tbb::flow::tuple<std::vector<ptrdiff_t> > > finish_image(graph, tbb::flow::serial, []() {
        std::cout << "End image" << std::endl;
    });

    tbb::flow::make_edge(input, limit_node);
    tbb::flow::make_edge(limit_node, f_find_minmax);
    tbb::flow::make_edge(f_find_minmax, broad_node);
    tbb::flow::make_edge(broad_node, f_fmax);
    tbb::flow::make_edge(broad_node, f_fmin);
    tbb::flow::make_edge(broad_node, f_fuser);
    tbb::flow::make_edge(f_fmax, input_port<1>(found_join));
    tbb::flow::make_edge(f_fmin, tbb::flow::input_port<2>(found_join));
    tbb::flow::make_edge(f_fuser, tbb::flow::input_port<3>(found_join));
    tbb::flow::make_edge(broad_node, tbb::flow::input_port<0>(found_join));
    tbb::flow::make_edge(found_join, f_dist_pixel);
    tbb::flow::make_edge(f_dist_pixel, f_invers_image);
    tbb::flow::make_edge(f_dist_pixel, f_find_mean);
    tbb::flow::make_edge(f_invers_image, tbb::flow::input_port<0>(finish_join));
    tbb::flow::make_edge(f_find_mean, tbb::flow::input_port<1>(finish_join));
    tbb::flow::make_edge(finish_join, finish_image);
    tbb::flow::make_edge(finish_image, limit_node.decrement);

    input.activate();
    graph.wait_for_all();
    std::cout << "DONE" << std::endl;
    return 0;
}
