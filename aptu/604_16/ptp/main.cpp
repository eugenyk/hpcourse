#include "tbb/flow_graph.h"
#include <iostream>
#include <map>
#include <string>
#include <cstdlib>
#include <ctime>
#include <fstream>

#include "image.h"

std::map<std::string, std::string> parse_args(int argc, char* argv[]) {
    std::map<std::string, std::string> args;
    for (size_t i = 1; i < argc; i += 2) {
        args[argv[i]] = argv[i + 1];
    }
    return args;
}

std::unique_ptr<int[]> generate_image(size_t height, size_t width) {
    std::unique_ptr<int[]> data(new int[height * width]);
    for (size_t i = 0; i < height * width; ++i) {
        data[i] = std::rand() % 256;
    }
    return data;
}

bool source_fn(std::shared_ptr<image> &p) {
    static size_t image_count = 100;
    size_t height = 1000;
    size_t width = 1000;

    if (image_count) {
        --image_count;
        p.reset(new image(height, width, generate_image(height, width)));
        return true;
    }
    return false;
}

using pair_vec = std::vector<std::pair<size_t, size_t>>;

std::shared_ptr<image> add_squares(std::tuple<pair_vec, pair_vec, pair_vec, std::shared_ptr<image>> tup) {
    std::shared_ptr<image> im = std::get<3>(tup);
    for (auto const& ij : std::get<0>(tup)) {
        im->square_around(ij.first, ij.second, 255);
    }
    for (auto const& ij : std::get<1>(tup)) {
        im->square_around(ij.first, ij.second, 255);
    }
    for (auto const& ij : std::get<2>(tup)) {
        im->square_around(ij.first, ij.second, 255);
    }

    return im;
}

int main(int argc, char* argv[]) {
    using namespace tbb::flow;

    std::srand(std::time(0));
    auto args = parse_args(argc, argv);

    int max_objects = std::stoi(args["-l"]);
    int given = std::stoi(args["-b"]);
    std::shared_ptr<std::ofstream> journal;
    if (args.find("-j") != args.end())
        journal.reset(new std::ofstream(args["-j"]));

    graph g;
    source_node<std::shared_ptr<image>> source(g, source_fn);

    limiter_node<std::shared_ptr<image>> limiter(g, max_objects);
    broadcast_node<std::shared_ptr<image>> broadcast(g);
    make_edge(source, limiter);
    make_edge(limiter, broadcast);

    using image_fn_node = function_node<std::shared_ptr<image>, pair_vec>;
    image_fn_node min_fn(g, unlimited, [](std::shared_ptr<image> im) { return im->min_positions(); });
    image_fn_node max_fn(g, unlimited, [](std::shared_ptr<image> im) { return im->max_positions(); });
    image_fn_node given_fn(g, unlimited, [given](std::shared_ptr<image> im) { return im->positions(given); });
    make_edge(broadcast, min_fn);
    make_edge(broadcast, max_fn);
    make_edge(broadcast, given_fn);

    join_node<std::tuple<pair_vec, pair_vec, pair_vec, std::shared_ptr<image>>, queueing> join(g);
    make_edge(min_fn, std::get<0>(join.input_ports()));
    make_edge(max_fn, std::get<1>(join.input_ports()));
    make_edge(given_fn, std::get<2>(join.input_ports()));
    make_edge(broadcast, std::get<3>(join.input_ports()));

    function_node<std::tuple<pair_vec, pair_vec, pair_vec, std::shared_ptr<image>>, std::shared_ptr<image>>
        squares_node(g, unlimited, add_squares);
    make_edge(join, squares_node);
    broadcast_node<std::shared_ptr<image>> broadcast2(g);
    make_edge(squares_node, broadcast2);

    function_node<std::shared_ptr<image>, float> mean_node(g, unlimited,
        [](std::shared_ptr<image> im) { return im->mean(); });
    make_edge(broadcast2, mean_node);

    join_node<std::tuple<float, std::shared_ptr<image>, std::shared_ptr<image>>, queueing> join2(g);
    function_node<float, float> journal_node(g, 1,
        [journal](float mean) {
            (*journal) << mean << std::endl;
            return mean;
        });
    if (journal) {
        make_edge(mean_node, journal_node);
        make_edge(journal_node, std::get<0>(join2.input_ports()));
    } else {
        make_edge(mean_node, std::get<0>(join2.input_ports()));
    }

    function_node<std::shared_ptr<image>, std::shared_ptr<image>> inverse_node(g, unlimited,
        [](std::shared_ptr<image> im) { return im->inverse(); });
    make_edge(broadcast2, inverse_node);
    make_edge(inverse_node, std::get<1>(join2.input_ports()));
    make_edge(broadcast2, std::get<2>(join2.input_ports()));

    function_node<std::tuple<float, std::shared_ptr<image>, std::shared_ptr<image>>, continue_msg>
        deleter(g, unlimited,
        [](std::tuple<float, std::shared_ptr<image>, std::shared_ptr<image>> tup) {
            return continue_msg();
        });
    make_edge(join2, deleter);
    make_edge(deleter, limiter.decrement);

    g.wait_for_all();

    return 0;
}
