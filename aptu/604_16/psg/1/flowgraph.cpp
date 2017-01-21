#include "tbb/flow_graph.h"

#include <iostream>
#include <string>
#include <vector>
#include <fstream>
#include <mutex>

using position_t = std::pair<int, int>;
using points_t = std::vector<position_t>;
using pixels_t = std::vector<std::vector<size_t>>;

struct config
{
    bool brightness_search = false;
    size_t brightness;
    size_t images_count;
    std::string log_path;
};

config parse_arguments(int argc, char** argv)
{
    using namespace std;
    if (argc < 3)
    {
        cout << "Usage: flowgraph [-b <value>] -l <value> [-f <value>]\n -b (brightness [0, 255])\n -l (limit of images)\n -f (brightness log path)" << endl;
        exit(0);
    }

    config conf;
    for (int i = 1; i < argc; ++i)
    {
        string arg(argv[i]);

        if (arg == "-b")
        {
            i++;
            int brightness = atoi(argv[i]);
            if (brightness < 0 || brightness > 255)
            {
                cerr << "Brightness should be in range [0,255]" << endl;
                exit(0);
            }
            conf.brightness = brightness;
            conf.brightness_search = true;
        }
        else if (arg == "-l")
        {
            i++;
            conf.images_count = atoi(argv[i]);

        }
        else if (arg == "-f")
        {
            i++;
            conf.log_path = argv[i];
        }
        else
        {
            cerr << "Unknown argument" << endl;
            exit(0);
        }
    }
    if (conf.brightness_search && conf.log_path == "") {
        cerr << "Provide path to log file" << endl;
        exit(0);        
    }

    return conf;
}

const size_t MAX_BRIGHTNESS = 255;
const size_t DEFAULT_WIDTH = 100;
const size_t DEFAULT_HEIGHT = 100;

class Image {
private:
    pixels_t pixels;
    size_t w;
    size_t h;

public:
    Image(size_t width, size_t height)
        : w(width), h(height) {
            pixels = pixels_t(h, std::vector<size_t>(w));
        }

    Image()
        : w(DEFAULT_WIDTH), h(DEFAULT_HEIGHT) {
            pixels = pixels_t(h, std::vector<size_t>(w));
        }

    void init_rand() {
        for (size_t i = 0; i < h; ++i) {
            for (size_t j = 0; j < w; ++j) {
                pixels[i][j] = rand() % (MAX_BRIGHTNESS + 1);
            }
        }
    }

    points_t get_bright_points(bool max) const {
        int most_bright = 0;
        for (size_t i = 0; i < h; ++i) {
            for (size_t j = 0; j < w; ++j) {
                if ((pixels[i][j] > most_bright && max) || (pixels[i][j] < most_bright && !max)) {
                    most_bright = pixels[i][j];
                }
            }
        }
        points_t result;
        for (size_t i = 0; i < h; ++i) {
            for (size_t j = 0; j < w; ++j) {
                if (pixels[i][j] == most_bright) {
                    result.push_back(std::make_pair(i, j));
                }
            }
        }
        return result;
    }

    points_t find_points(int target) const {
        points_t result;
        for (size_t i = 0; i < h; ++i) {
            for (size_t j = 0; j < w; ++j) {
                if (pixels[i][j] == target) {
                    result.push_back(std::make_pair(i, j));
                }
            }
        }
        return result;
    }

    float mean() const {
        float result = 0.f;
        for (size_t i = 0; i < h; ++i) {
            for (size_t j = 0; j < w; ++j) {
                result += pixels[i][j];
            }
        }
        return result / (w * h);
    }

    void highlight_points(points_t& points) {
        for (std::pair<size_t, size_t> feature: points) {
            color_square(feature.first, feature.second);
        }
    }

    void invert_colors() {
        for (size_t i = 0; i < h; ++i) {
            for (size_t j = 0; j < w; ++j) {
                pixels[i][j] = MAX_BRIGHTNESS - pixels[i][j];
            }
        }
    }

    void color_square(int y, int x) {
        for (int i = x - 1; i <= x + 1; ++i) {
            for (int j = y - 1; j <= y + 1; ++j) {
                if (i < 0 || i >= h || j < 0 || j >= w) {
                    continue;
                }
                pixels[i][j] = MAX_BRIGHTNESS;
            }
        }
    }
};

const size_t DEFAULT_IMAGES_CNT = 10;

void calc_flow(const config& conf) {
    using namespace std;
    using namespace tbb::flow;

    size_t images_count = conf.images_count;
    int brightness = conf.brightness;
    string log_path = conf.log_path;

    size_t counter = 0;
    auto image_gen = [&](Image& output) -> bool {
        if (counter >= DEFAULT_IMAGES_CNT) {
            return false;
        }
        counter++;
        Image image(DEFAULT_WIDTH, DEFAULT_HEIGHT);
        image.init_rand();
        output = image;
        return true;
    };

    graph g;
    source_node<Image> source(g, image_gen);
    limiter_node<Image> limit(g, images_count);
    broadcast_node<Image> broadcast(g);

    auto min_func = [](const Image& image) -> points_t {
        return image.get_bright_points(false);
    };
    function_node<Image, points_t, queueing> min_node(g, serial, min_func);

    auto max_func = [](const Image& image) -> points_t {
        return image.get_bright_points(true);
    };
    function_node<Image, points_t, queueing> max_node(g, serial, max_func);

    auto equal_func = [brightness](const Image& image) -> points_t {
        return image.find_points(brightness);
    };
    function_node<Image, points_t, queueing> equal_node(g, serial, equal_func);

    join_node<tuple<Image, points_t, points_t, points_t>, queueing> join(g);

    auto highlight_func = [](tuple<Image, points_t, points_t, points_t> input)->Image {
        Image image = get<0>(input);
        image.highlight_points(get<1>(input));
        image.highlight_points(get<2>(input));
        image.highlight_points(get<3>(input));
        return image;
    };
    function_node<tuple<Image, points_t, points_t, points_t>, Image, rejecting> highlight(g, unlimited, highlight_func);
    broadcast_node<Image> broadcast2(g);

    auto mean_func = [](const Image& image)->float {
        return image.mean();
    };
    function_node<Image, float, queueing> mean_node(g, serial, mean_func);

    auto invert_func = [](const Image& image)->Image {
        Image result(image);
        result.invert_colors();
        return result;
    };
    function_node<Image, Image, rejecting> invert_node(g, unlimited, invert_func);

    auto continue_func = [](float mean)->continue_msg {
        return continue_msg();
    };
    function_node<float, continue_msg, rejecting> continue_node(g, unlimited, continue_func);

    mutex mtx;
    ofstream log(log_path);
    auto output_func = [&](float mean)->continue_msg {
        string msg = "Mean brightness: " + to_string(mean);
        mtx.lock(); // otherwise some output lines could be lost
        log << msg << endl; 
        mtx.unlock();
        return continue_msg();
    };
    function_node<float, continue_msg, queueing> output(g, serial, output_func);

    make_edge(source, limit);
    make_edge(limit, broadcast);

    make_edge(broadcast, equal_node);
    make_edge(broadcast, max_node);
    make_edge(broadcast, min_node);

    make_edge(broadcast, input_port<0>(join));
    make_edge(max_node, input_port<1>(join));
    make_edge(min_node, input_port<2>(join));
    make_edge(equal_node, input_port<3>(join));

    make_edge(join, highlight);
    make_edge(highlight, broadcast2);
    make_edge(broadcast2, mean_node);
    make_edge(broadcast2, invert_node);
    if (conf.brightness_search) {
        make_edge(mean_node, output);
        make_edge(output, limit.decrement);
    } else {
        make_edge(mean_node, continue_node);
        make_edge(continue_node, limit.decrement);
    }

    g.wait_for_all();
    log.close();
}

int main(int argc, char* argv[]) {
    try {
        const config conf = parse_arguments(argc, argv);
        calc_flow(conf);
        std::cout << "completed" << std::endl;
        return 0;
    } catch (std::exception& e) {
        std::cerr << "Error occurred. Error text is : \"" << e.what() << "\"\n";
        return -1;
    }
}
