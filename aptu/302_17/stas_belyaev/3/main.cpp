#include <iostream>
#include <tbb/tbb.h>
#include <set>

template<class T>
struct Data {
    int id;
    T t;

    Data() {}

    Data(const int tag, const T t) : id(tag), t(t) {}
};

typedef u_int8_t pixel_t;
typedef std::vector<std::vector<pixel_t>> image_t;
typedef Data<image_t> Image;
typedef std::pair<int, int> point_t;
typedef std::set<point_t> point_set_t;
typedef Data<point_set_t> PointSet;
typedef Data<double> Double;

struct Select {
    const pixel_t selecting_pixel;

    Select(const pixel_t selecting_pixel) : selecting_pixel(selecting_pixel) {}

    PointSet operator()(const Image &image) const {
        point_set_t selecting_set;
        for (int x = 0; x < image.t.size(); ++x) {
            for (int y = 0; y < image.t[x].size(); ++y) {
                if (image.t[x][y] == selecting_pixel) {
                    selecting_set.insert({x, y});
                }
            }
        }
        return PointSet(image.id, selecting_set);
    }
};

struct Min {
    PointSet operator()(const Image &image) const {
        pixel_t min_pixel = std::numeric_limits<pixel_t>::max();
        for (int x = 0; x < image.t.size(); ++x) {
            for (int y = 0; y < image.t[x].size(); ++y) {
                min_pixel = std::min(min_pixel, image.t[x][y]);
            }
        }
        return Select(min_pixel)(image);
    }
};

struct Max {
    PointSet operator()(const Image &image) const {
        pixel_t max_pixel = std::numeric_limits<pixel_t>::min();
        for (int x = 0; x < image.t.size(); ++x) {
            for (int y = 0; y < image.t[x].size(); ++y) {
                max_pixel = std::max(max_pixel, image.t[x][y]);
            }
        }
        return Select(max_pixel)(image);
    }
};

template<class T>
struct GetId {
    int operator()(const Data<T> &data) {
        return data.id;
    }
};

struct Highlight {
    void brighten(image_t &image, const int px, const int py) const {
        for (int x = std::max(px - 1, 0); x < std::min(px + 2, (int) image.size()); ++x) {
            for (int y = std::max(py - 1, 0); y < std::min(py + 2, (int) image[0].size()); ++y) {
                image[x][y] = std::numeric_limits<pixel_t>::max();
            }
        }
    }

    Image operator()(std::tuple<Image, PointSet, PointSet, PointSet> data) const {
        Image image;
        PointSet ps1, ps2, ps3;
        std::tie(image, ps1, ps2, ps3) = data;
        image_t result_image = image.t;
        for (int x = 0; x < image.t.size(); ++x) {
            for (int y = 0; y < image.t[x].size(); ++y) {
                if (ps1.t.find({x, y}) != ps1.t.end()
                    || ps2.t.find({x, y}) != ps2.t.end()
                    || ps3.t.find({x, y}) != ps3.t.end()) {
                    brighten(result_image, x, y);
                }
            }
        }
        return Image(image.id, result_image);
    }
};

struct Mean {
    Double operator()(const Image &image) const {
        double sum = 0;
        for (int x = 0; x < image.t.size(); ++x) {
            for (int y = 0; y < image.t[x].size(); ++y) {
                sum += image.t[x][y];
            }
        }
        return Double(image.id, sum / (image.t.size() * image.t[0].size()));
    }
};

struct Inverse {
    Image operator()(const Image &image) const {
        image_t inverse_image = image.t;
        for (int x = 0; x < image.t.size(); ++x) {
            for (int y = 0; y < image.t[x].size(); ++y) {
                inverse_image[x][y] = (pixel_t) (255 - image.t[x][y]);
            }
        }
        return Image(image.id, inverse_image);
    };
};

struct Params {
    pixel_t brightness = 0;
    size_t img_limit = std::numeric_limits<size_t>::max();
    std::string log_filename = "";

    Params() {}
};

Params parse_params(int argc, char **argv) {
    Params params;
    for (int i = 0; i < argc; ++i) {
        if (strcmp(argv[i], "-b") == 0) {
            params.brightness = (pixel_t) atoi(argv[i + 1]);
        } else if (strcmp(argv[i], "-l") == 0) {
            params.img_limit = (size_t) atoi(argv[i + 1]);
        } else if (strcmp(argv[i], "-f") == 0) {
            params.log_filename = argv[i + 1];
        }
    }
    return params;
}

image_t create_image(const size_t height, const size_t width) {
    image_t image;
    image.resize(height);
    for (auto &row : image) {
        row.resize(width);
        for (auto &pixel : row) {
            pixel = (pixel_t) rand();
        }
    }
    return image;
}

void test(const Params &params,
          tbb::flow::graph &graph,
          tbb::flow::limiter_node<Image> &input_node,
          tbb::flow::buffer_node<Double> &buffer_node) {
    printf("Start testing.\n");
    for (int i = 0; i < 20; ++i) {
        Image image = Image(i, create_image(1024, 1024));
        while (!input_node.try_put(image));
    }
    printf("Waiting for all images to proceed.\n");
    graph.wait_for_all();
    FILE *file = NULL;
    if (params.log_filename.size() > 0) {
        file = fopen(params.log_filename.c_str(), "w");
    }
    for (int i = 0; i < 20; ++i) {
        Double mean;
        if (buffer_node.try_get(mean)) {
            if (file) {
                fprintf(file, "%d %f\n", mean.id, mean.t);
            } else {
                printf("%d %f\n", mean.id, mean.t);
            }
        }
    }
    printf("End testing.\n");
    if (file) {
        fclose(file);
    }
}

void make_and_test_graph(const Params &params) {
    using namespace tbb::flow;

    graph graph;

    limiter_node<Image> input_node(graph, params.img_limit);

    function_node<Image, PointSet> min_element_node(graph, unlimited, Min());
    make_edge(input_node, min_element_node);

    function_node<Image, PointSet> max_element_node(graph, unlimited, Max());
    make_edge(input_node, max_element_node);

    function_node<Image, PointSet> fixed_element_node(graph, unlimited, Select(params.brightness));
    make_edge(input_node, fixed_element_node);

    join_node<
            std::tuple<Image, PointSet, PointSet, PointSet>,
            key_matching<int>
    > join_node1(graph, GetId<image_t>(), GetId<point_set_t>(), GetId<point_set_t>(), GetId<point_set_t>());
    make_edge(input_node, input_port<0>(join_node1));
    make_edge(max_element_node, input_port<1>(join_node1));
    make_edge(min_element_node, input_port<2>(join_node1));
    make_edge(fixed_element_node, input_port<3>(join_node1));

    function_node<
            std::tuple<Image, PointSet, PointSet, PointSet>,
            Image
    > highlight_node(graph, unlimited, Highlight());
    make_edge(join_node1, highlight_node);

    function_node<Image, Image> inverse_node(graph, unlimited, Inverse());
    make_edge(highlight_node, inverse_node);

    function_node<Image, Double> mean_node(graph, unlimited, Mean());
    make_edge(highlight_node, mean_node);

    buffer_node<Double> buffer_node(graph);
    make_edge(mean_node, buffer_node);

    join_node<
            std::tuple<Image, Double>,
            key_matching<int>
    > join_node2(graph, GetId<image_t>(), GetId<double>());
    make_edge(highlight_node, input_port<0>(join_node2));
    make_edge(mean_node, input_port<1>(join_node2));

    function_node<
            std::tuple<Image, Double>,
            continue_msg
    > finish_node(graph,
                  unlimited,
                  [](const std::tuple<Image, Double> data) -> continue_msg { return continue_msg(); });
    make_edge(join_node2, finish_node);
    make_edge(finish_node, input_node.decrement);

    test(params, graph, input_node, buffer_node);
}

int main(int argc, char **argv) {
    Params params = parse_params(argc, argv);
    make_and_test_graph(params);
}