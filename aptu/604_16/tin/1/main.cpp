#include "tbb/flow_graph.h"
#include "node_functions.hpp"
#include "image.hpp"

using namespace tbb::flow;
using namespace node_functions;
using std::shared_ptr;


struct configs {
    size_t limit = 10;
    int value = 123;
    bool has_logger = false;
    std::string log_file = "";
};


configs read_command_line(int argc, char* argv[]) {
    configs config;
    for (int i = 1; i < argc; ++i) {
        if (strcmp(argv[i], "-b") == 0) {
            config.value = std::stoi(argv[++i]);
        } else if (strcmp(argv[i], "-l") == 0) {
            config.limit = static_cast<size_t>(std::stoi(argv[++i]));
        } else if (strcmp(argv[i], "-f") == 0) {
            config.has_logger = true;
            config.log_file = argv[++i];
        } else {
            std::cout << "-b <value_to_find> -l <image_limit> -f <log_filename>" << std::endl;
        }
    }
    return config;
}


int main(int argc, char* argv[]) {
    configs config = read_command_line(argc, argv);
    size_t limit = config.limit;
    int value = config.value;
    bool has_logger = config.has_logger;
    std::string log_file = config.log_file;
    std::ofstream fout;
    fout.open(log_file, std::ofstream::out | std::ofstream::trunc);
    fout.close();

    size_t square_side = 2;
    size_t image_rows = 20;
    size_t image_cols = 20;
    size_t images_num = 20000;
    image_generator image_gen(images_num, image_rows, image_cols);

    graph g;
    source_node<shared_ptr<Image>> input_node(g, image_gen);
    limiter_node<shared_ptr<Image>> limit_node (g, limit);
    function_node<shared_ptr<Image>, ImageWithValue> max_node(g, unlimited, max());
    function_node<shared_ptr<Image>, ImageWithValue> min_node(g, unlimited, min());
    function_node<shared_ptr<Image>, ImageWithValue> add_value_node(g, unlimited, add_value(value));
    function_node<ImageWithValue, ImageWithPoints> value_points_node(g, unlimited, get_values());
    function_node<ImageWithValue, ImageWithPoints> max_points_node(g, unlimited, get_values());
    function_node<ImageWithValue, ImageWithPoints> min_points_node(g, unlimited, get_values());
    join_node<tbb::flow::tuple<ImageWithPoints, ImageWithPoints, ImageWithPoints>, key_matching<int>>
            point_join_node (g, image_with_points_to_key(), image_with_points_to_key(), image_with_points_to_key());

    function_node<tbb::flow::tuple<ImageWithPoints, ImageWithPoints, ImageWithPoints>,
            shared_ptr<Image>> merger_node(g, unlimited, merge(square_side));
    function_node<shared_ptr<Image>, shared_ptr<Image>> inverter_node(g, unlimited, invert());
    function_node<shared_ptr<Image>, float_with_id> averages_node(g, unlimited, count_average());
    join_node<tbb::flow::tuple<shared_ptr<Image>, float_with_id>, key_matching<int>>
            answer_join_node(g, image_to_key(), tuple_to_key<double>());
    function_node<tbb::flow::tuple<shared_ptr<Image>, float_with_id>, continue_msg> output_signal_node(g, unlimited,
            [](const tbb::flow::tuple<shared_ptr<Image>, float_with_id>& tuple) {
                return continue_msg();
            });

    function_node<float_with_id, float_with_id> logger_node(g, serial, logger(log_file));

    make_edge(limit_node, max_node);
    make_edge(limit_node, min_node);
    make_edge(limit_node, add_value_node);
    make_edge(max_node, max_points_node);
    make_edge(min_node, min_points_node);
    make_edge(add_value_node, value_points_node);
    make_edge(value_points_node, input_port<0>(point_join_node));
    make_edge(max_points_node, input_port<1>(point_join_node));
    make_edge(min_points_node, input_port<2>(point_join_node));
    make_edge(point_join_node, merger_node);
    make_edge(merger_node, inverter_node);
    make_edge(merger_node, averages_node);
    make_edge(inverter_node, input_port<0>(answer_join_node));
    if (!has_logger) {
        make_edge(averages_node, input_port<1>(answer_join_node));
    }
    else {
        make_edge(averages_node, logger_node);
        make_edge(logger_node, input_port<1>(answer_join_node));
    }
    make_edge(answer_join_node, output_signal_node);
    make_edge(output_signal_node, limit_node.decrement);
    make_edge(input_node, limit_node);

    g.wait_for_all();

    return 0;
}