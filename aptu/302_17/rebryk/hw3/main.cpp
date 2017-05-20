#include <iostream>
#include <tbb/tbb.h>
#include <set>
#include <limits>
#include <vector>

template<class T>
struct Tagged {
    int tag;
    T data;

    Tagged() {}
    Tagged(const int tag, const T data) : tag(tag), data(data) {}
};

template<class T>
struct Tag {
    int operator()(const Tagged<T> &tagged) {
        return tagged.tag;
    }
};

typedef std::vector<std::vector<u_int8_t>> Image;
typedef Tagged<Image> TImage;

typedef std::pair<size_t, size_t> Point;
typedef std::set<Point> PointSet;
typedef Tagged<PointSet> TPointSet;

typedef Tagged<double> TDouble;


struct Options {
    size_t limit;
    u_int8_t brightness;
    std::string log_name;

    Options() {
        limit = std::numeric_limits<size_t>::max();
        brightness = 0;
        log_name = "";
    }
};

Image create_image(const size_t height, const size_t weight) {
    Image image(height);
    for (auto &row : image) {
        row.resize(weight);
        for (auto &x : row) {
            x = static_cast<u_int8_t>(rand());
        }
    }

    return image;
}

struct FindPixels {
    const u_int8_t value;

    FindPixels(const u_int8_t value) : value(value) {}

    TPointSet operator()(const TImage &image) const {
        PointSet result;
        for (size_t i = 0; i < image.data.size(); ++i) {
            for (size_t j = 0; j < image.data[i].size(); ++j) {
                if (image.data[i][j] == value) {
                    result.insert({i, j});
                }
            }
        }

        return TPointSet(image.tag, result);
    }
};

struct MaxElement {
    TPointSet operator()(const TImage &image) const {
        u_int8_t max_value = 0;
        for (auto &row : image.data) {
            max_value = std::max(max_value, *std::max_element(row.begin(), row.end()));
        }

        return FindPixels(max_value)(image);
    }
};

struct MinElement {
    TPointSet operator()(const TImage &image) const {
        u_int8_t min_value = 255;
        for (auto &row : image.data) {
            min_value = std::min(min_value, *std::min_element(row.begin(), row.end()));
        }

        return FindPixels(min_value)(image);
    }
};

struct Highlight {
    bool contains(const PointSet &set, const Point &point) const {
        return set.find(point) != set.end();
    }

    void highlight(Image &image, const int x, const int y) const {
        const int height = static_cast<int>(image.size());
        const int width = static_cast<int>(image[0].size());

        for (int i = std::max(x - 1, 0); i < std::min(x + 2, height); ++i) {
            for (int j = std::max(y - 1, 0); j < std::min(y + 2, width); ++j) {
                image[i][j] = 255;
            }
        }
    }

    TImage operator()(std::tuple<TImage, TPointSet, TPointSet, TPointSet> data) const {
        const Image &original = std::get<0>(data).data;
        Image result = original;

        for (int i = 0; i < original.size(); ++i) {
            for (int j = 0; j < original[i].size(); ++j) {
                if (contains(std::get<1>(data).data, {i, j}) || contains(std::get<2>(data).data, {i, j}) ||
                    contains(std::get<3>(data).data, {i, j})) {
                    highlight(result, i, j);
                }
            }
        }

        return TImage(std::get<0>(data).tag, result);
    }
};

struct MeanBrightness {
    TDouble operator()(const TImage &image) const {
        double sum = 0;
        for (auto &row : image.data) {
            for (auto &x : row) {
                sum += x;
            }
        }

        const int height = static_cast<int>(image.data.size());
        const int width = static_cast<int>(image.data[0].size());

        return Tagged<double>(image.tag, sum / (height * width));
    }
};

struct Inverse {
    TImage operator()(const TImage &image) const {
        const Image &original = image.data;
        Image result = original;

        for (int i = 0; i < original.size(); ++i) {
            for (int j = 0; j < original[i].size(); ++j) {
                result[i][j] = static_cast<u_int8_t>(255 - original[i][j]);
            }
        }

        return TImage(image.tag, result);
    };
};

void draw_image(const Image& image) {
    for (int i = 0; i < image.size(); ++i) {
        for (int j = 0; j < image[i].size(); ++j) {
            std::cout << (int) image[i][j] << " ";
        }
        std::cout << std::endl;
    }
}

const size_t IMAGES_COUNT = 30;
const size_t HEIGHT = 500;
const size_t WIDTH = 500;

Options parse_options(int args, char **argv) {
    Options options;

    for (int i = 1; i < args; i += 2) {
        if (strcmp(argv[i], "-b") == 0) {
            options.brightness = static_cast<u_int8_t>(atoi(argv[i + 1]));
        }

        if (strcmp(argv[i], "-I") == 0) {
            options.limit = static_cast<size_t>(atoi(argv[i + 1]));
        }

        if (strcmp(argv[i], "-f") == 0) {
            options.log_name = std::string(argv[i + 1]);
        }
    }

    return options;
}

int main(int argc, char **argv) {
    Options options = parse_options(argc, argv);

    tbb::flow::graph graph;

    tbb::flow::limiter_node<TImage> input_node(graph, options.limit);

    tbb::flow::function_node<TImage, TPointSet> max_element_node(graph, tbb::flow::unlimited, MaxElement());
    tbb::flow::function_node<TImage, TPointSet> min_element_node(graph, tbb::flow::unlimited, MinElement());
    tbb::flow::function_node<TImage, TPointSet> fixed_element_node(graph, tbb::flow::unlimited,
                                                                   FindPixels(options.brightness));


    tbb::flow::join_node<
            std::tuple<TImage, TPointSet, TPointSet, TPointSet>,
            tbb::flow::key_matching<int>
    > join_node(graph, Tag<Image>(), Tag<PointSet>(), Tag<PointSet>(), Tag<PointSet>());


    tbb::flow::function_node<
            std::tuple<TImage, TPointSet, TPointSet, TPointSet>,
            TImage
    > highlight_node(graph, tbb::flow::unlimited, Highlight());

    tbb::flow::function_node<TImage, TImage> inverse_node(graph, tbb::flow::unlimited, Inverse());
    tbb::flow::function_node<TImage, TDouble> mean_node(graph, tbb::flow::unlimited, MeanBrightness());

    tbb::flow::buffer_node<TDouble> buffer_node(graph);

    tbb::flow::join_node<
            std::tuple<TImage, TDouble>,
            tbb::flow::key_matching<int>
    > join_node2(graph, Tag<Image>(), Tag<double>());

    tbb::flow::function_node<
            std::tuple<TImage, TDouble>,
            tbb::flow::continue_msg
    > terminal_node(graph,
                    tbb::flow::unlimited,
                    [](const std::tuple<TImage, TDouble> data) -> tbb::flow::continue_msg { return tbb::flow::continue_msg(); });


    tbb::flow::make_edge(input_node, max_element_node);
    tbb::flow::make_edge(input_node, min_element_node);
    tbb::flow::make_edge(input_node, fixed_element_node);

    tbb::flow::make_edge(input_node, tbb::flow::input_port<0>(join_node));
    tbb::flow::make_edge(max_element_node, tbb::flow::input_port<1>(join_node));
    tbb::flow::make_edge(min_element_node, tbb::flow::input_port<2>(join_node));
    tbb::flow::make_edge(fixed_element_node, tbb::flow::input_port<3>(join_node));

    tbb::flow::make_edge(join_node, highlight_node);

    tbb::flow::make_edge(highlight_node, inverse_node);
    tbb::flow::make_edge(highlight_node, mean_node);

    tbb::flow::make_edge(mean_node, buffer_node);

    tbb::flow::make_edge(highlight_node, tbb::flow::input_port<0>(join_node2));
    tbb::flow::make_edge(mean_node, tbb::flow::input_port<1>(join_node2));

    tbb::flow::make_edge(join_node2, terminal_node);
    tbb::flow::make_edge(terminal_node, input_node.decrement);


    // TESTING

    for (int i = 0; i < IMAGES_COUNT; ++i) {
        const TImage image = TImage(i, create_image(HEIGHT, WIDTH));
        while (!input_node.try_put(image));
    }

    graph.wait_for_all();

    FILE* file = NULL;

    if (options.log_name.size() > 0) {
        file = fopen(options.log_name.c_str(), "w");
    }

    for (int i = 0; i < IMAGES_COUNT; ++i) {
        TDouble mean;
        if (buffer_node.try_get(mean)) {
            if (file) {
                fprintf(file, "%d %f\n", mean.tag, mean.data);
            } else {
                printf("%d %f\n", mean.tag, mean.data);
            }
        }
    }

    if (file) {
        fclose(file);
    }

    return 0;
}