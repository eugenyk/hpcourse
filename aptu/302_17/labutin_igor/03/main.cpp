#include <iostream>
#include <tbb/flow_graph.h>
#include <fstream>
#include <vector>

typedef std::pair<int, int> Position;
typedef std::vector<Position> Positions;

const int SQUARE_SIZE = 2;
const int COLOR_MARK = 255;

struct Image {
    int h;
    int w;
    std::string name;
    std::vector<std::vector<int>> values;

    Image() {}

    Image(int h, int w, std::string name) : h(h), w(w), name(name), values(h) {
        for (int i = 0; i < h; i++) {
            values[i].resize(w);
        }
    }
};


std::vector<Image> read_images() {
    std::vector<Image> res;
    for (int file_id = 0;; file_id++) {
        std::ifstream image_file;
        image_file.open("../data/image_" + std::to_string(file_id) + ".test");
        if (!image_file.is_open()) {
            break;
        }
        int h, w;
        image_file >> h >> w;
        Image image(h, w, "image_" + std::to_string(file_id));
        for (int i = 0; i < h; i++) {
            for (int j = 0; j < w; j++) {
                image_file >> image.values[i][j];
            }
        }
        res.push_back(image);
        image_file.close();
    }
    return res;
}


void print_usage() {
    std::cerr << "Usage:\n";
    std::cerr << "gen_images\n";
    std::cerr << "“-b 123”: интересующее значение яркости, для шага № 2\n";
    std::cerr << "“-l 5”: предел одновременно обрабатываемых приложением изображений\n";
    std::cerr << "“-f log.txt”: имя файла журнала яркостей\n";
    exit(1);
}


void parse_args(int argc, char* argv[], int& arg_b, size_t& arg_l, std::string& arg_f) {
    char cur_arg = 0;
    for (int i = 1; i < argc; i++) {
        switch (cur_arg) {
            case 0:
                if (argv[i][0] == '-') {
                    cur_arg = argv[i][1];
                } else {
                    print_usage();
                }
                break;
            case 'b':
                arg_b = atoi(argv[i]);
                cur_arg = 0;
                break;
            case 'l':
                arg_l = atoi(argv[i]);
                cur_arg = 0;
                break;
            case 'f':
                arg_f = std::string(argv[i]);
                cur_arg = 0;
                break;
            default:
                print_usage();
        }
    }
}


struct find_elements {
    Positions operator()(const Image& image, const int val) {
        Positions res;
        for (int i = 0; i < image.h; i++) {
            for (int j = 0; j < image.w; j++) {
                if (image.values[i][j] == val) {
                    res.push_back({i, j});
                }
            }
        }
        return res;
    }
};

struct find_max_elements {
    Positions operator()(const Image& image) {
        Positions res;
        int max_val = 0;
        for (int i = 0; i < image.h; i++) {
            for (int j = 0; j < image.w; j++) {
                max_val = std::max(max_val, image.values[i][j]);
            }
        }
        return find_elements()(image, max_val);
    }
};


struct find_min_elements {
    Positions operator()(const Image& image) {
        Positions res;
        int min_val = 256;
        for (int i = 0; i < image.h; i++) {
            for (int j = 0; j < image.w; j++) {
                min_val = std::min(min_val, image.values[i][j]);
            }
        }
        return find_elements()(image, min_val);
    }
};


void mark_square(Image& image, const Positions& positions) {
    for (auto pos : positions) {
        for (int dx = -SQUARE_SIZE; dx < SQUARE_SIZE; dx++) {
            for (int dy = -SQUARE_SIZE; dy < SQUARE_SIZE; dy++) {
                if (dx != 0 || dy != 0) {
                    int new_x = pos.first + dx;
                    int new_y = pos.second + dy;
                    if (new_x >= 0 && new_x < image.h && new_y >= 0 && new_y < image.w) {
                        image.values[new_x][new_y] = COLOR_MARK;
                    }
                }
            }
        }
    }
}


struct mark_all {
    Image operator()(std::tuple<Image, Positions, Positions, Positions> args) {
        Image image = std::get<0>(args);
        mark_square(image, std::get<1>(args));
        mark_square(image, std::get<2>(args));
        mark_square(image, std::get<3>(args));
        return image;
    }
};


struct calc_inverse {
    Image operator()(const Image& image) {
        Image res(image.h, image.w, image.name + "_inverted");
        for (int i = 0; i < res.h; i++) {
            for (int j = 0; j < res.w; j++) {
                res.values[i][j] = 255 - image.values[i][j];
            }
        }
        return res;
    }
};


struct calc_mean {
    std::pair<std::string, double> operator()(const Image& image) {
        double res = 0;
        for (int i = 0; i < image.h; i++) {
            for (int j = 0; j < image.w; j++) {
                res += image.values[i][j];
            }
        }
        return std::make_pair(image.name, res / image.h / image.w);
    }
};


int main(int argc, char* argv[]) {
    std::vector<Image> images = read_images();

    int arg_b = 0;
    size_t arg_l = 5;
    std::string arg_f = "";
    parse_args(argc, argv, arg_b, arg_l, arg_f);

    tbb::flow::graph graph;

    tbb::flow::broadcast_node<Image> start(graph);

    tbb::flow::function_node<Image, Positions> find_max(graph, arg_l, find_max_elements());
    tbb::flow::function_node<Image, Positions> find_min(graph, arg_l, find_min_elements());
    tbb::flow::function_node<Image, Positions> find_eq(graph, arg_l,
                                                       [arg_b](const Image& image) -> Positions {
                                                           return find_elements()(image, arg_b);
                                                       });
    tbb::flow::join_node<std::tuple<Image, Positions, Positions, Positions>> join(graph);
    tbb::flow::function_node<std::tuple<Image, Positions, Positions, Positions>, Image> mark(graph,
                                                                                             arg_l,
                                                                                             mark_all());
    tbb::flow::function_node<Image, Image> inverse(graph, arg_l, calc_inverse());
    tbb::flow::function_node<Image, std::pair<std::string, double>> mean(graph, arg_l, calc_mean());
    tbb::flow::buffer_node<std::pair<std::string, double>> result_mean(graph);

    tbb::flow::make_edge(start, find_max);
    tbb::flow::make_edge(start, find_min);
    tbb::flow::make_edge(start, find_eq);
    tbb::flow::make_edge(start, tbb::flow::input_port<0>(join));
    tbb::flow::make_edge(find_min, tbb::flow::input_port<1>(join));
    tbb::flow::make_edge(find_eq, tbb::flow::input_port<2>(join));
    tbb::flow::make_edge(find_eq, tbb::flow::input_port<3>(join));
    tbb::flow::make_edge(join, mark);
    tbb::flow::make_edge(mark, inverse);
    tbb::flow::make_edge(mark, mean);
    tbb::flow::make_edge(mean, result_mean);

    for (Image image : images) {
        start.try_put(image);
    }

    graph.wait_for_all();

    if (arg_f != "") {
        std::ofstream out;
        out.open(arg_f);
        for (int i = 0; i < images.size(); i++) {
            std::pair<std::string, double> res;
            if (result_mean.try_get(res)) {
                out << "mean(" << res.first << ") = " << res.second << std::endl;
            } else {
                std::cerr << "can't get result" << std::endl;
            }
        }
        out.close();
    }

    return 0;
}