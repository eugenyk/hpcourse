#include <cstdio>
#include <iostream>
#include <algorithm>
#include <fstream>
#include <ctgmath>
#include "cxxopts.hpp"
#include "tbb/flow_graph.h"

using namespace tbb::flow;

const int HEIGHT = 5;
const int WIDTH = 5;
const int IMG_COUNT = 10;
const int MAX_BRIGHT = 255;

int cnt = 0;
typedef std::vector<std::vector<int>> image;
typedef std::pair<int, int> elem;
typedef std::vector<elem> elems;

struct generate {
    int height;
    int width;
public:
    generate( int h, int w ) : height(h), width(w) {}

    bool operator()(image& img) {
        img = image(height, std::vector<int>(width));
        if (cnt >= IMG_COUNT)
            return false;
        for (auto &row : img)
            std::generate(row.begin(), row.end(), []() { return std::rand() % (MAX_BRIGHT + 1); });
        cnt++;
        return true;
        }
};

elems find(const image &img, int bright) {
    elems res;
    for (int i = 0; i < img.size(); ++i)
        for (int j = 0; j < img[i].size(); ++j)
            if (img[i][j] == bright)
                res.push_back(std::make_pair(i, j));
    return res;
}

struct max_elems {
    elems operator()(const image& img) {
        int max = 0;
        for (auto row : img)
            max = std::max(max, *std::max_element(row.begin(), row.end()));
        return find(img, max);
    }
};

struct min_elems {
    elems operator()(const image& img) {
        int min = MAX_BRIGHT;
        for (auto row : img)
            min = std::min(min, *std::min_element(row.begin(), row.end()));
        return find(img, min);
    }
};

class find_elems {
    int &bright;
public:
    find_elems( int &arg ) : bright(arg) {}

    elems operator()(const image& img) {
        return find(img, bright);
    }
};

struct mark {
    auto operator()(tuple<image, elems, elems, elems> input) {
        image img = get<0>(input);
        frame_elems(img, get<1>(input));
        frame_elems(img, get<2>(input));
        frame_elems(img, get<3>(input));
        return img;
    }
private:
    void frame_elems(image &img, auto elems) {
        for (elem el : elems) {
            int row = el.first;
            int col = el.second;
            for (int i = row - 1; i <= row + 1; ++i)
                for (int j = col - 1; j <= col + 1; ++j) {
                    if (i == row && j == col)
                        continue;
                    try {
                        img.at(i).at(j) = MAX_BRIGHT;
                    } catch (const std::out_of_range &e) {}
                }
        }
    }
};

struct inverse {
    image operator()(const image& input) {
        image img = input;
        for (auto &row : img)
            std::for_each(row.begin(), row.end(), [](int& pixel) {
                pixel = MAX_BRIGHT - pixel;
            });
        return img;
    }
};

struct avg_bright {
    double operator()(const image& img) {
        int sum = 0;
        for (auto row : img)
            for (int val : row)
                sum += val;
        return sum / (double) (img.size() * img[1].size());
    }
};

struct print {
    void operator()(const image &img) {
        for (auto row : img) {
            for (int & point : row)
                std::cout << point << ' ';
            std::cout << std::endl;
        }
        std::cout << std::endl;
    }
};

int main(int argc, char* argv[]) {

    int given_bright;
    int h = HEIGHT;
    int w = WIDTH;
    int limit;
    std::string file;
    try {
        cxxopts::Options options(argv[0], "Flow graph");
        options.add_options()
                ("b,bright", "Given brightness", cxxopts::value<int>(given_bright)->default_value("127"))
                ("l,limit", "Parallel image process limit", cxxopts::value<int>(limit)->default_value("5"))
                ("f,file", "File to log average value", cxxopts::value<std::string>(file)->default_value("avglog.txt"));
        options.parse(argc, argv);
    } catch (const cxxopts::OptionException &e) {
        std::cout << "error parsing options: " << e.what() << std::endl;
        exit(1);
    }

    std::srand( time(0) );
    std::ofstream fout(file);

    graph g;
    source_node<image> generator(g, generate(h, w), false);
    limiter_node<image> limiter(g, limit);
    broadcast_node<image> send_to_finders(g);
    function_node<image,elems> find_max( g, unlimited, max_elems() );
    function_node<image,elems> find_min( g, unlimited, min_elems() );
    function_node<image,elems> find_given( g, unlimited, find_elems(given_bright) );
    join_node< tuple<image, elems, elems, elems>, queueing > join_to_frame( g );
    function_node<tuple<image, elems, elems, elems>, image> mark_all( g, unlimited, mark() );
    broadcast_node<image> split(g);
    function_node<image,image> invert( g, unlimited, inverse() );
    function_node<image,double> average( g, unlimited, avg_bright());
    function_node<double, continue_msg> logger(g, unlimited, [&fout](float avg) -> continue_msg {
        fout << avg << std::endl;
        return continue_msg();
    });
    join_node<tuple<continue_msg, continue_msg>, queueing > final_join( g );
    function_node<tuple<continue_msg, continue_msg>,continue_msg>
            transformer( g, unlimited, [](tuple<continue_msg, continue_msg>) {
        return continue_msg();
    });
    function_node<image,continue_msg> printer( g, serial, print() );

    make_edge( generator, limiter );
    make_edge( limiter, send_to_finders );
    make_edge( send_to_finders, find_max );
    make_edge( send_to_finders, find_min );
    make_edge( send_to_finders, find_given );
    make_edge( send_to_finders, input_port<0> (join_to_frame) );
    make_edge( find_max, input_port<1> (join_to_frame) );
    make_edge( find_min, input_port<2> (join_to_frame) );
    make_edge( find_given, input_port<3> (join_to_frame) );
    make_edge( join_to_frame, mark_all );
    make_edge( mark_all, split );
    make_edge( split, invert );
    make_edge( split, average );
    make_edge( average, logger );
    make_edge( logger, input_port<0>(final_join) );
    make_edge( invert, printer );
    make_edge( printer, input_port<1>(final_join) );
    make_edge( final_join, transformer );
    make_edge( transformer, limiter.decrement );

    generator.activate();
    g.wait_for_all();
    fout.close();
    printf("Finished");
    return 0;
}