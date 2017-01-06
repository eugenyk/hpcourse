#include <cstdio>
#include <cstdlib>
#include <random>
#include <thread>
#include <iostream>
#include <fstream>
#include "tbb/flow_graph.h"

using namespace tbb::flow;
using namespace std;

typedef vector<pair<size_t, size_t>> points;
const int MAX_INTENSITY = 255;
const int IMAGE_SIZE = 256;

class Image {
public:
    Image(size_t width = IMAGE_SIZE, size_t height = IMAGE_SIZE)
            : width_(width), height_(height) {
        data_ = vector<vector<int>>(height_, vector<int>(width_));
    }

    void fill_random() {
        for (size_t i = 0; i < height_; ++i) {
            for (size_t j = 0; j < width_; ++j) {
                data_[i][j] = rand() % (MAX_INTENSITY + 1);
            }
        }
    }

    points max_intensity_points() const {
        int max_intensity = 0;
        for (size_t i = 0; i < height_; ++i) {
            for (size_t j = 0; j < width_; ++j) {
                max_intensity = max(data_[i][j], max_intensity);
            }
        }
        points result;
        for (size_t i = 0; i < height_; ++i) {
            for (size_t j = 0; j < width_; ++j) {
                if (data_[i][j] == max_intensity) {
                    result.push_back(make_pair(i, j));
                }
            }
        }
        return result;
    }

    points min_intensity_points() const {
        int min_intensity = MAX_INTENSITY;
        for (size_t i = 0; i < height_; ++i) {
            for (size_t j = 0; j < width_; ++j) {
                min_intensity = min(data_[i][j], min_intensity);
            }
        }
        points result;
        for (size_t i = 0; i < height_; ++i) {
            for (size_t j = 0; j < width_; ++j) {
                if (data_[i][j] == min_intensity) {
                    result.push_back(make_pair(i, j));
                }
            }
        }
        return result;
    }

    points target_intensity_points(int target) const {
        points result;
        for (size_t i = 0; i < height_; ++i) {
            for (size_t j = 0; j < width_; ++j) {
                if (data_[i][j] == target) {
                    result.push_back(make_pair(i, j));
                }
            }
        }
        return result;
    }

    float mean_intensity() const {
        float result = 0.f;
        for (size_t i = 0; i < height_; ++i) {
            for (size_t j = 0; j < width_; ++j) {
                result += data_[i][j];
            }
        }
        return result / (width_ * height_);
    }

    void invert() {
        for (size_t i = 0; i < height_; ++i) {
            for (size_t j = 0; j < width_; ++j) {
                data_[i][j] = MAX_INTENSITY - data_[i][j];
            }
        }
    }

    void highlight_features(points features) {
        for (pair<size_t, size_t> feature: features) {
            mark_rectangle(feature.first, feature.second);
        }
    }

private:
    size_t width_;
    size_t height_;
    vector<vector<int>> data_;

    void mark_rectangle(size_t y, size_t x) {
        for (int i = (int) x - 1; i <= x + 1; ++i) {
            for (int j = (int) y - 1; j <= y + 1; ++j) {
                fill_point(i, j, MAX_INTENSITY);
            }
        }
    }

    void fill_point(int i, int j, int value) {
        if (i >= 0 && i < height_ && j >= 0 && j < width_) {
            data_[i][j] = value;
        }
    }
};

int main(int argc, char* argv[]) {
    size_t images_limit = 10;
    string log_file = "log.txt";
    int target_intensity = 127;

    cout << "Usage: -b <target intensity in [0, 255]>  -l <images limit> -f <log file>" << endl;
    for (int i = 0; i < argc; ++i) {
        if (strcmp(argv[i], "-b") == 0) {
            target_intensity = stoi(argv[++i]);
            cout << "Target intensity = " << target_intensity << endl;
            continue;
        }
        if (strcmp(argv[i], "-l") == 0) {
            images_limit = (size_t) stoi(argv[++i]);
            cout << "Images limit = " << images_limit << endl;
            continue;
        }
        if (strcmp(argv[i], "-f") == 0) {
            log_file = argv[++i];
            continue;
        }
    }
    ofstream log(log_file);

    graph g;
    size_t total_images = 1000;
    size_t counter = 0;
    auto generator = [&](Image& image) -> bool {
        if (counter >= total_images) {
            return false;
        }
        counter++;
        image.fill_random();
        return true;
    };
    source_node<Image> source(g, generator);
    limiter_node<Image> limit(g, images_limit);
    broadcast_node<Image> broadcast(g);

    auto max_body = [](const Image& image) -> points {
        return image.max_intensity_points();
    };
    auto min_body = [](const Image& image) -> points {
        return image.min_intensity_points();
    };
    auto equal_body = [target_intensity](const Image& image) -> points {
        return image.target_intensity_points(target_intensity);
    };

    function_node<Image, points, rejecting> max_node(g, serial, max_body);
    function_node<Image, points, rejecting> min_node(g, serial, min_body);
    function_node<Image, points, rejecting> equal_node(g, serial, equal_body);
    join_node<tuple<Image, points, points, points>, queueing> join(g);

    auto highlight_body = [&log](tuple<Image, points, points, points> input)->Image {
        Image image = get<0>(input);
        image.highlight_features(get<1>(input));
        image.highlight_features(get<2>(input));
        image.highlight_features(get<3>(input));
        return image;
    };
    function_node<tuple<Image, points, points, points>, Image, rejecting> highlight(g, unlimited, highlight_body);
    broadcast_node<Image> broadcast2(g);

    auto mean_body = [](const Image& image)->float {
        return image.mean_intensity();
    };
    function_node<Image, float, rejecting> mean_node(g, serial, mean_body);

    auto invert_body = [](const Image& image)->Image {
        Image result(image);
        result.invert();
        return result;
    };
    function_node<Image, Image, rejecting> invert_node(g, unlimited, invert_body);

    auto output_body = [&log](float mean)->continue_msg {
        log << "Mean intensity = " << mean << endl;
        return continue_msg();
    };
    function_node<float, continue_msg, rejecting> output(g, serial, output_body);

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
    make_edge(mean_node, output);
    make_edge(output, limit.decrement);

    source.activate();
    g.wait_for_all();
    log.close();
    cout << "Ok!" << endl;
    return 0;
}
