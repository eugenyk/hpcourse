#include <tbb/flow_graph.h>
#include <iostream>
#include <fstream>

#include "GeneratedImage.h"

using namespace std;
using namespace tbb::flow;

const size_t N_IMAGES = 50;
const size_t WIDTH = 680;
const size_t HEIGHT = 550;

int brightness = 0;
size_t parallel_limit = 0;
string log_path = "";
size_t n_gen_sofar = 0;

auto source_f = [&n_gen_sofar](GeneratedImage &out) {
    if (n_gen_sofar >= N_IMAGES)
        return false;
    ++n_gen_sofar;
    GeneratedImage im(HEIGHT, WIDTH);
    out = im;
    return true;
};

void print_usage() {
    cout << "-b <brightness value> -l <n images proceed in parallel> -f <log file>\n";
}

int parse_arguments(int argc, char *argv[]) {
    if (argc != 7 || argv[1] != string("-b") || argv[3] != string("-l") || argv[5] != string("-f")) {
        print_usage();
        return 1;
    }
    brightness = stoi(argv[2]);
    parallel_limit = stoi(argv[4]);
    log_path = argv[6];

    return 0;
}


auto max_f = [](GeneratedImage const &im) {
    unsigned char maxi = 0;
    im.map([&maxi](unsigned char elem) {
        if (elem > maxi) {
            maxi = elem;
        }
    });
    return im.filter(maxi);
};

auto min_f = [](GeneratedImage const &im) {
    unsigned char mini = 255;
    im.map([&mini](unsigned char elem) {
        if (elem < mini) {
            mini = elem;
        }
    });
    return im.filter(mini);
};

auto eq_f = [brightness](GeneratedImage const &im) {
    return im.filter(brightness);
};

auto marker_f = [](tuple<GeneratedImage, vector<size_t>, vector<size_t>, vector<size_t>> const &in) {
    GeneratedImage im(get<0>(in));
    im.highlight(get<1>(in));
    im.highlight(get<2>(in));
    im.highlight(get<3>(in));
    return im;
};

auto mean_f = [](GeneratedImage const &im) {
    return im.mean();
};

auto invert_f = [](GeneratedImage const &im) {
    GeneratedImage res(im);
    unsigned char maxi = 0;
    res.map([&maxi](unsigned char elem) {
        if (elem > maxi) {
            maxi = elem;
        }
    });
    res.invert(maxi);
    return res;
};

auto output_f = [&log_path](float mean) {
    if (!log_path.empty()) {
        std::ofstream log(log_path, fstream::app | fstream::out);
        log << mean << std::endl;
    }
    return continue_msg();
};

int main(int argc, char *argv[]) {

    const int code = parse_arguments(argc, argv);
    if (code != 0) {
        return code;
    }

    graph g;

    source_node<GeneratedImage> source_node(g, source_f, false);
    function_node<GeneratedImage, vector<size_t>> max_node(g, unlimited, max_f);
    function_node<GeneratedImage, vector<size_t>> min_node(g, unlimited, min_f);
    function_node<GeneratedImage, vector<size_t>> eq_node(g, unlimited, eq_f);
    function_node<tuple<GeneratedImage, vector<size_t>, vector<size_t>, vector<size_t>>, GeneratedImage>
            marker(g, unlimited, marker_f);
    function_node<GeneratedImage, float> mean_node(g, unlimited, mean_f);
    function_node<GeneratedImage, GeneratedImage> invert_node(g, unlimited, invert_f);
    function_node<float, continue_msg> output_node(g, unlimited, output_f);

    limiter_node<GeneratedImage> limiter(g, parallel_limit);
    make_edge(source_node, limiter);
    broadcast_node<GeneratedImage> broadcaster(g);
    make_edge(limiter, broadcaster);
    make_edge(broadcaster, max_node);
    make_edge(broadcaster, min_node);
    make_edge(broadcaster, eq_node);
    join_node<tuple<GeneratedImage, vector<size_t>, vector<size_t>, vector<size_t>>> joiner(g);
    make_edge(broadcaster, input_port<0>(joiner));
    make_edge(max_node, input_port<1>(joiner));
    make_edge(eq_node, input_port<2>(joiner));
    make_edge(min_node, input_port<3>(joiner));
    make_edge(joiner, marker);
    broadcast_node<GeneratedImage> broadcaster2(g);
    make_edge(marker, broadcaster2);
    make_edge(broadcaster2, mean_node);
    make_edge(broadcaster2, invert_node);
    make_edge(mean_node, output_node);
    make_edge(output_node, limiter.decrement);

    source_node.activate();
    g.wait_for_all();
    return 0;
}
