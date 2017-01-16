#include "image.h"
#include "agroritms.h"
#include "comporator.h"

#include <tbb/flow_graph.h>
#include <random>
#include <ctime>
#include <cstring>
#include <algorithm>
#include <iostream>
#include <tuple>

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

    auto find_minMax = [](const Image &img) {
        auto minMax = minmax_element(img.cpixels().begin(), img.cpixels().end());
        unsigned char minValue = minMax.first != img.cpixels().end() ? *minMax.first : 1;
        unsigned char maxValue = minMax.second != img.cpixels().end() ? *minMax.second : UCHAR_MAX;
        return make_tuple(img, std::make_pair(minValue, maxValue));
    };

    auto fmax = [](std::tuple<Image, std::pair<unsigned char,unsigned char> > arg) {
        auto img = tbb::flow::get<0>(arg);
        return make_tuple(img, findPixels(img.cpixels().begin(), img.cpixels().end(), tbb::flow::get<1>(arg).first));
    };

    auto fmin = [](tbb::flow::tuple<Image, std::pair<unsigned char,unsigned char> > arg) {
        auto img = tbb::flow::get<0>(arg);
        return make_tuple(img, findPixels(img.cpixels().begin(), img.cpixels().end(), tbb::flow::get<1>(arg).second));
    };

    auto fuser = [userValue](tbb::flow::tuple<Image, std::pair<unsigned char,unsigned char> > arg) {
        auto img = tbb::flow::get<0>(arg);
        return make_tuple(img, findPixels(img.cpixels().begin(), img.cpixels().end(), userValue));
    };

    auto merge = [](tbb::flow::tuple<
            std::tuple<Image, std::vector<ptrdiff_t> >,
            std::tuple<Image, std::vector<ptrdiff_t> >,
            std::tuple<Image, std::vector<ptrdiff_t> > > arg) {
        auto img = std::get<0>(tbb::flow::get<0>(arg));
        auto vect = std::get<1>(tbb::flow::get<0>(arg));
        auto vect1 = std::get<1>(tbb::flow::get<1>(arg));
        auto vect2 = std::get<1>(tbb::flow::get<2>(arg));
        vect.insert(vect.end(), vect1.begin(), vect1.end());
        vect.insert(vect.end(), vect2.begin(), vect2.end());
        return make_tuple(img, vect);
    };

    auto marked = [](std::tuple<Image, std::vector<ptrdiff_t> > arg) {
        auto img = std::get<0>(arg);
        auto vect = std::get<1>(arg);
        mark(vect, img.pixels(), img.width(), img.height(), UCHAR_MAX);
        return img;
    };

    auto inverse_image = [](Image img) {
        unsigned char m = 0;
        bitInverseValues(img.pixels().begin(), img.pixels().end());
        return std::make_tuple(img, m);
    };

    auto find_mean_brightness = [](const Image &img) {
        unsigned char m = meanValue(img.cpixels().begin(), img.cpixels().end(), 0);
        std::cout << "Mean brightness: " << static_cast<int>(m) << std::endl;
        return std::make_tuple(img, m);
    };

    auto finish = [](tbb::flow::tuple<
            std::tuple<Image, unsigned char>,
            std::tuple<Image, unsigned char> > arg) {
        (void)arg;
        std::cout << "End image" << std::endl;
    };

    tbb::flow::source_node<Image> input(graph, source);
    tbb::flow::limiter_node<Image> limit_node(graph, l);
    tbb::flow::function_node<Image, tbb::flow::tuple<Image, std::pair<unsigned char,unsigned char> > > f_find_minmax(graph, tbb::flow::unlimited, find_minMax);
    tbb::flow::broadcast_node<tbb::flow::tuple<Image, std::pair<unsigned char,unsigned char> > > broad_node(graph);
    tbb::flow::function_node<std::tuple<Image, std::pair<unsigned char,unsigned char> >,
            std::tuple<Image, std::vector<ptrdiff_t> > > f_fmax(graph, tbb::flow::serial, fmax);

    tbb::flow::function_node<tbb::flow::tuple<Image, std::pair<unsigned char,unsigned char> >,
            std::tuple<Image, std::vector<ptrdiff_t> > > f_fmin(graph, tbb::flow::serial, fmin);

    tbb::flow::function_node<tbb::flow::tuple<Image, std::pair<unsigned char,unsigned char> >,
            std::tuple<Image, std::vector<ptrdiff_t> > > f_fuser(graph, tbb::flow::serial, fuser);

    tbb::flow::join_node<
            tbb::flow::tuple<
                std::tuple<Image, std::vector<ptrdiff_t> >,
                std::tuple<Image, std::vector<ptrdiff_t> >,
                std::tuple<Image, std::vector<ptrdiff_t> >
            > > found_join(graph);

    tbb::flow::function_node<
            tbb::flow::tuple<
                std::tuple<Image, std::vector<ptrdiff_t> >,
                std::tuple<Image, std::vector<ptrdiff_t> >,
                std::tuple<Image, std::vector<ptrdiff_t> > >,
            std::tuple<Image, std::vector<ptrdiff_t> > > f_dist_pixel(graph, tbb::flow::unlimited, merge);

    tbb::flow::function_node<std::tuple<Image, std::vector<ptrdiff_t> >, Image> f_mark(graph, tbb::flow::unlimited, marked);
    tbb::flow::function_node<Image, std::tuple<Image, unsigned char> > f_invers_image(graph, tbb::flow::serial, inverse_image);
    tbb::flow::function_node<Image, std::tuple<Image, unsigned char> > f_find_mean(graph, tbb::flow::serial, find_mean_brightness);
    tbb::flow::join_node<tbb::flow::tuple<
                std::tuple<Image, unsigned char>,
                std::tuple<Image, unsigned char>
            > > finish_join(graph);

    tbb::flow::function_node<tbb::flow::tuple<
                std::tuple<Image, unsigned char>,
                std::tuple<Image, unsigned char>
            > > finish_image(graph, tbb::flow::serial, finish);

    tbb::flow::make_edge(input, limit_node);
    tbb::flow::make_edge(limit_node, f_find_minmax);
    tbb::flow::make_edge(f_find_minmax, broad_node);
    tbb::flow::make_edge(broad_node, f_fmax);
    tbb::flow::make_edge(broad_node, f_fmin);
    tbb::flow::make_edge(broad_node, f_fuser);
    tbb::flow::make_edge(f_fmax, tbb::flow::input_port<0>(found_join));
    tbb::flow::make_edge(f_fmin, tbb::flow::input_port<1>(found_join));
    tbb::flow::make_edge(f_fuser, tbb::flow::input_port<2>(found_join));
    tbb::flow::make_edge(found_join, f_dist_pixel);
    tbb::flow::make_edge(f_dist_pixel, f_mark);
    tbb::flow::make_edge(f_mark, f_invers_image);
    tbb::flow::make_edge(f_mark, f_find_mean);
    tbb::flow::make_edge(f_invers_image, tbb::flow::input_port<0>(finish_join));
    tbb::flow::make_edge(f_find_mean, tbb::flow::input_port<1>(finish_join));
    tbb::flow::make_edge(finish_join, finish_image);
    tbb::flow::make_edge(finish_image, limit_node.decrement);

    graph.wait_for_all();
    std::cout << "DONE" << std::endl;
    return 0;
}
