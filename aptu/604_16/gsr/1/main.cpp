#include <tbb/flow_graph.h>
#include <iostream>
#include <fstream>
#include <stdlib.h>

#include "img_ops.h"

using namespace std;
using namespace tbb::flow;

using pos_v = vector<size_t>;

void print_usage() {
    cout << "-b brightness (0-255) -I images parallel [-f log fileName]" << endl;
    exit(1);
}

int main(int argc, char *argv[]) {
    if (argc != 5 && argc != 7) {
        print_usage();
    }

    int brightness = 0;
    size_t max_parallel = 0;
    string path = "";

    if (argv[1] == string("-b")) {
        brightness = stoi(argv[2]);
    } else {
        print_usage();
    }

    if (argv[3] == string("-I")) {
        max_parallel = stoi(argv[4]);
    } else {
        print_usage();
    }

    if (argc > 5) {
        if (argv[5] == string("-f")) {
            path = argv[6];
        } else {
            print_usage();
        }
    }

    size_t generated = 0;
    graph g;


    source_node<image> source(g,
                              [&generated](image &out) {
                                  if (generated >= NUM_IMAGES)
                                      return false;
                                  ++generated;
                                  image im(DEFAULT_HEIGHT, DEFAULT_WIDTH);
                                  out = im;
                                  return true;
                              }, false);

    function_node<image, pos_v> max_node(g, 1,
                                         [](image const &im) {
                                             unsigned char maxi = 0;
                                             im.map([&maxi](unsigned char elem) {
                                                 if (elem > maxi) {
                                                     maxi = elem;
                                                 }
                                             });
                                             return im.filter(maxi);
                                         });

    function_node<image, pos_v> min_node(g, 1,
                                         [](image const &im) {
                                             unsigned char mini = 255;
                                             im.map([&mini](unsigned char elem) {
                                                 if (elem < mini) {
                                                     mini = elem;
                                                 }
                                             });
                                             return im.filter(mini);
                                         });


    function_node<image, pos_v> eq_node(g, 1,
                                        [brightness](image const &im) {
                                            return im.filter(brightness);
                                        });

    function_node<tuple<image, pos_v, pos_v, pos_v>, image>
            highlighter(g, unlimited,
                        [](tuple<image, pos_v, pos_v, pos_v> const &in) {
                            image im(get<0>(in));
                            im.highlight(get<1>(in));
                            im.highlight(get<2>(in));
                            im.highlight(get<3>(in));
                            return im;
                        });

    function_node<image, float> mean_node(g, unlimited,
                                          [](image const &im) {
                                              return im.mean();
                                          });

    function_node<image, image> invert_node(g, unlimited,
                                            [](image const &im) {
                                                image res(im);
                                                unsigned char maxi = 0;
                                                res.map([&maxi](unsigned char elem) {
                                                    if (elem > maxi) {
                                                        maxi = elem;
                                                    }
                                                });
                                                res.invert(maxi);
                                                return res;
                                            });

    function_node<float, continue_msg> output(g, unlimited,
                                              [&path](float mean) {
                                                  if (!path.empty()) {
                                                      std::ofstream log(path, fstream::app|fstream::out);
                                                      log << mean << std::endl;
                                                  }
                                                  return continue_msg();
                                              });

    limiter_node<image> limiter(g, max_parallel);
    make_edge(source, limiter);

    broadcast_node<image> broadcast_im(g);
    make_edge(limiter, broadcast_im);

    make_edge(broadcast_im, max_node);
    make_edge(broadcast_im, min_node);
    make_edge(broadcast_im, eq_node);

    join_node<tuple<image, pos_v, pos_v, pos_v>> joiner(g);
    make_edge(broadcast_im, input_port<0>(joiner));
    make_edge(max_node, input_port<1>(joiner));
    make_edge(eq_node, input_port<2>(joiner));
    make_edge(min_node, input_port<3>(joiner));

    make_edge(joiner, highlighter);

    broadcast_node<image> broadcast_im2(g);
    make_edge(highlighter, broadcast_im2);

    make_edge(broadcast_im2, mean_node);
    make_edge(broadcast_im2, invert_node);


    make_edge(mean_node, output);
    make_edge(output, limiter.decrement);

    source.activate();
    g.wait_for_all();
    return 0;
}
