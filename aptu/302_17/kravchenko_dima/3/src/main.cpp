#include "utils.h"
#include "image.h"

#include <tbb/flow_graph.h>

#include <iostream>
#include <fstream>
#include <string.h>
#include <stdlib.h>

using namespace tbb::flow;

int l = 5;
int b = 123;

std::ofstream *out = NULL;

static void set_option(char *last_option, char *value)
{
    if (!strcmp(last_option, "-b")) {
        b = atoi(value);
        std::cout << "b changed to " << b << std::endl;
    }
    if (!strcmp(last_option, "-l")) {
        l = atoi(value);
        std::cout << "l changed to " << l << std::endl;
    }
    if (!strcmp(last_option, "-f")) {
        out = new std::ofstream(value);
        std::cout << "results will be printed to '" << value << "'" << std::endl;
    }
}

static void parse_args(int argc, char **argv)
{
    char *last_option = NULL;
    for (int i = 1; i < argc; i++) {
        if (last_option != NULL) {
            set_option(last_option, argv[i]);
            last_option = NULL;
        } else {
            last_option = argv[i];
        }
    }
}

int main(int argc, char **argv) {
    parse_args(argc, argv);

    if (out == NULL)
        out = new std::ofstream("/dev/null");

    srand(time(NULL));
/*
 *
 * S -- source_node
 * F -- function_node
 * J -- join_node 
 *
 *             -----------------------------------------------+
 *            /           -----(F -- find_max_elements)-----  |              -----(F -- inverse_brightness)-------------------------------------
 *           /           /                                  \ |             /                                                                   \
 *  (S -- generate_image)------(F -- find_min_elements)------(J)----(F -- highlight found)                                                      (J)----(F -- clean)
 *                       \                                  /               \                                                                   /
 *                        -----(F -- find_equal_elements)---                 -----(F -- get_avg_brightness)-----(F -- print_avg_brightness)-----
 *
 *
 *
 *
 *
 */

    tbb::flow::graph g;

// create graph nodes

    tbb::flow::source_node<image *> src(g,
                [](image * &next_image) -> bool {
                if (l-- <= 0)
                    return false;
                next_image = generate_image(l);
                return true;
            });

    tbb::flow::function_node<image *, std::vector<point_t>> find_max_elements_node(g, 1,
            [](image *img) -> std::vector<point_t> {
                return find_max_elements(img);
            });

    tbb::flow::function_node<image *, std::vector<point_t>> find_min_elements_node(g, 1,
            [](image *img) -> std::vector<point_t> {
                return find_min_elements(img);
            });

    tbb::flow::function_node<image *, std::vector<point_t>> find_equal_elements_node(g, 1,
            [](image *img) -> std::vector<point_t> {
                return find_equal_elements(img, b);
            });

    tbb::flow::join_node<std::tuple<image *, std::vector<point_t>, std::vector<point_t>, std::vector<point_t>>> join1(g);

    tbb::flow::function_node<std::tuple<image *, std::vector<point_t>, std::vector<point_t>, std::vector<point_t>>, image *> highlight_found_node(g, 1,
            [](std::tuple<image *, std::vector<point_t>, std::vector<point_t>, std::vector<point_t>> t) {
                return highlight_found(t);
            });

    tbb::flow::function_node<image *, bool> inverse_brightness_node(g, 1,
            [](image *img) {
                inverse_brightness(img);
                return true;
            });

    tbb::flow::function_node<image *, std::pair<image *, double>> get_avg_brightness_node(g, 1,
            [](image *img) {
                return get_avg_brightness(img);
            });

    tbb::flow::function_node<std::pair<image *, double>, image *> print_avg_brightness_node(g, 1,
            [](std::pair<image *, double> a) {
                return print_avg_brightness(a, *out);
            });

    tbb::flow::join_node<std::tuple<image *, bool>> join2(g);

    tbb::flow::function_node<std::tuple<image *, bool>, bool> clean_up_node(g, 1,
            [](std::tuple<image *, bool> t) {
                image *img = std::get<0>(t);
                delete img;
                return true;
            });

// create edges

    tbb::flow::make_edge(src, find_max_elements_node);
    tbb::flow::make_edge(src, find_min_elements_node);
    tbb::flow::make_edge(src, find_equal_elements_node);

    tbb::flow::make_edge(src, input_port<0>(join1));
    tbb::flow::make_edge(find_max_elements_node, input_port<1>(join1));
    tbb::flow::make_edge(find_min_elements_node, input_port<2>(join1));
    tbb::flow::make_edge(find_equal_elements_node, input_port<3>(join1));

    tbb::flow::make_edge(join1, highlight_found_node);

    tbb::flow::make_edge(highlight_found_node, inverse_brightness_node);
    tbb::flow::make_edge(highlight_found_node, get_avg_brightness_node);
    
    tbb::flow::make_edge(get_avg_brightness_node, print_avg_brightness_node);

    tbb::flow::make_edge(print_avg_brightness_node, input_port<0>(join2));
    tbb::flow::make_edge(inverse_brightness_node, input_port<1>(join2));

    tbb::flow::make_edge(join2, clean_up_node);

 // graph is ready, activate source

    src.activate();

// wait to finish

    g.wait_for_all();

    out->close();
    delete out;

    return 0;
}
