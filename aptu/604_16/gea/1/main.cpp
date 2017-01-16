#include <iostream>
#include <tbb/flow_graph.h>
#include <random>
#include <iostream>
#include <cstdlib>
#include <cstdio>
#include <fstream>
#include "image_mat.h"

using namespace tbb::flow;

using bw_image = image_mat<uint8_t>;
using image_with_poss = std::pair<bw_image*, std::vector<img_pos>>;
using positions_join_tpl = std::tuple<image_with_poss, image_with_poss, image_with_poss>;

/**
 * to print uint8_t as ints not as chars
 */
std::ostream& operator<<(std::ostream& out, const uint8_t& num) {
    return out << (unsigned int) num;
}

std::random_device rd;
std::mt19937 rng(rd());
std::uniform_int_distribution<size_t> uni(0, std::numeric_limits<uint8_t>::max());
uint8_t random_filler(size_t row, size_t col) {
    return (uint8_t) uni(rng);
}

void print_usage() {
    std::cout << "OPTIONS: " << std::endl;
    std::cout << "    -b <Brightness value to search for>, default = 127" << std::endl;
    std::cout << "    -l <Parallel img. processing cnt limit>, default = 4" << std::endl;
    std::cout << "    [-f <Brightness log file path>], defaults to STDOUT" << std::endl;
}

struct opts {
    uint8_t br_val;
    size_t parallel_lim;
    std::string br_log;
    opts(uint8_t br = 127, size_t lim = 4, std::string brLog = ""): br_val(br), parallel_lim(lim), br_log(brLog) {
    }
};

opts read_opts(int argc, char** argv) {
    std::vector<std::string> args;
    for (int i = 1; i < argc; ++i) {
        args.push_back(std::string(argv[i]));
    }
    opts op;
    for (size_t i = 0; i < args.size(); ++i) {
        if (i == args.size() - 1 || (args[i] != "-b" && args[i] != "-l" && args[i] != "-f")) {
            print_usage();
            exit(1);
        }
        if (args[i] == "-b") {
            op.br_val = (uint8_t) std::stoi(args[++i]);
        } else if (args[i] == "-l") {
            op.parallel_lim = std::stoul(args[++i]);
        } else if (args[i] == "-f") {
            op.br_log = args[++i];
        }
    }
    return op;
}


int main(int argc, char** argv) {
    const size_t ROWS = 100;
    const size_t COLS = 300;
    const size_t MAX_MATS = 1000;

    auto options = read_opts(argc, argv);

    std::cout << "Going to use this options:" << std::endl;
    std::cout << "   Brightness value to search for = " << options.br_val << std::endl;
    std::cout << "   Max parallel processing images = " << options.parallel_lim << std::endl;
    std::cout << "   Log file name                  = " << (options.br_log.empty() ? "STDOUT" : options.br_log)
              << std::endl;

    graph g;

    source_node<bw_image *> source(
            g,
            [](bw_image *&img) {
                static size_t mat_cnt = 0;
                if (mat_cnt >= MAX_MATS) {
                    return false;
                }
                mat_cnt += 1;
                img = new bw_image(ROWS, COLS, random_filler);
                return true;
            },
            false // messages will not be generated until activate explicitly called
    );

    // setting up limiter, which limits number of processed images in graph
    limiter_node<bw_image*> limiter(g, options.parallel_lim);

    make_edge(source, limiter);

    // node, which broadcasts one image to all processing logic nodes
    broadcast_node<bw_image*> broadcast_img(g);
    make_edge(limiter, broadcast_img);

    // node, which search max br. indices
    function_node<bw_image*, image_with_poss> calc_max_positions(g, unlimited,
            [](bw_image* img) {
                return std::make_pair(img, img->get_maxs());
            }
    );

    // node, which search min br. indices
    function_node<bw_image*, image_with_poss> calc_min_positions(g, unlimited,
            [](bw_image* img) {
                return std::make_pair(img, img->get_mins());
            }
    );

    // node, which location positions where br. equal to given in options
    function_node<bw_image*, image_with_poss> calc_br_positions(g, unlimited,
            [&options](bw_image* img) {
                return std::make_pair(img, img->get_equal_to(options.br_val));
            }
    );

    // broadcasting image to all calulating nodes
    make_edge(broadcast_img, calc_max_positions);
    make_edge(broadcast_img, calc_min_positions);
    make_edge(broadcast_img, calc_br_positions);

    auto img_with_pos_key_getter = [](const image_with_poss& iwp) {
        return reinterpret_cast<uintptr_t>(iwp.first);
    };

    join_node<positions_join_tpl, key_matching<uintptr_t>> join_positions(g,
                                                                          img_with_pos_key_getter,
                                                                          img_with_pos_key_getter,
                                                                          img_with_pos_key_getter);

    // joining all calculated nodes
    make_edge(calc_max_positions, input_port<0>(join_positions));
    make_edge(calc_min_positions, input_port<1>(join_positions));
    make_edge(calc_br_positions, input_port<2>(join_positions));

    // highlighting node
    function_node<positions_join_tpl, bw_image*> highlight_positions(g, unlimited,
            [&options](positions_join_tpl join_data) {
                bw_image* img = get<0>(join_data).first;
                img->highlight(get<0>(join_data).second, 255);
                img->highlight(get<1>(join_data).second, 255);
                img->highlight(get<2>(join_data).second, 255);
                return img;
            }
    );

    make_edge(join_positions, highlight_positions);

    broadcast_node<bw_image*> highlighted_image_broadcast(g);
    make_edge(highlight_positions, highlighted_image_broadcast);

    using mean_type = int;
    using mean_res = std::pair<bw_image*, mean_type>;
    // inversion returns pair of old and inverted matrices
    using inv_res = std::pair<bw_image*, bw_image*>;
    using final_join_in = std::tuple<bw_image*, inv_res>;

    function_node<bw_image*, mean_res> calc_mean(
            g,
            unlimited,
            [](bw_image* img) {
                return std::make_pair(img, img->get_mean<mean_type>(0));
            }
    );

    std::ofstream f_out(options.br_log);
    std::ostream& mean_out = f_out.is_open() ? f_out : std::cout;

    // log returns image, which must be deleted at the end of the pipeline
    function_node<mean_res, bw_image*> log_mean(
            g,
            1, // log is shared resource
            [&mean_out](mean_res res) {
                mean_out << "MEAN BRIGHTNESS: " << res.second << std::endl;
                return res.first;
            }
    );

    make_edge(highlighted_image_broadcast, calc_mean);
    make_edge(calc_mean, log_mean);

    function_node<bw_image*, inv_res> calc_inv(
            g,
            unlimited,
            [](bw_image* img) {
                bw_image* new_img = new bw_image(*img);
                return std::make_pair(img, new_img);
            }
    );
    make_edge(highlighted_image_broadcast, calc_inv);


    join_node<final_join_in, key_matching<uintptr_t>> final_join(
            g,
            [](bw_image* img) {
                return reinterpret_cast<uintptr_t>(img);
            },
            [](const inv_res& invr) {
                return reinterpret_cast<uintptr_t>(invr.first);
            }
    );

    make_edge(log_mean, input_port<0>(final_join));
    make_edge(calc_inv, input_port<1>(final_join));

    function_node<final_join_in, continue_msg> cleanup_and_continue(
            g,
            unlimited,
            [](const final_join_in& res) {
                // cleaning up inverted and source matrices
                delete get<1>(res).second;
                delete get<0>(res);
                return continue_msg();
            }
    );

    make_edge(final_join, cleanup_and_continue);
    make_edge(cleanup_and_continue, limiter.decrement); // decrement limiter "barrier"

    source.activate();
    g.wait_for_all();

    return 0;
}