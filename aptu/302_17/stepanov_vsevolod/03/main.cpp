#define _GLIBCXX_DEBUG

#include <iostream>
#include <tbb/tbb.h>
#include <set>

typedef std::vector<std::vector<uint8_t>> image;

image gen_random_image(size_t height, size_t width) {
    image img(height, std::vector<uint8_t >(width));
    for (int i = 0; i < height; i++) {
        for (int j = 0; j < width; j++) {
            img[i][j] = (uint8_t) rand();
        }
    }
    return img;
}

template<class T> struct with_name {
    std::string name;
    T object;
    with_name() {}
    with_name(std::string name_, T object_): name(name_), object(object_) {}
};

template<class T> struct get_name {
    std::string operator()(const with_name<T> &x) {
        return x.name;
    }
};

struct point {
    int i, j;
    point(int i_, int j_): i(i_), j(j_) {}
    bool operator < (point p) const {
        return i == p.i ? j < p.j : i < p.i;
    }
};

struct find_equal {
    uint8_t val;
    find_equal(uint8_t val_): val(val_) {}

    with_name<std::set<point>> operator()(const with_name<image> &img_with_name) {
        std::set<point> result;
        const image& img = img_with_name.object;
        for (int i = 0; i < img.size(); i++) {
            for (int j = 0; j < img[0].size(); j++) {
                if (img[i][j] == val) {
                    result.insert(point(i, j));
                }
            }
        }
        return with_name<std::set<point>>(img_with_name.name, result);
    }
};

struct find_max {
    with_name<std::set<point>> operator()(const with_name<image> &img_with_name) {
        uint8_t min = 0;
        const image& img = img_with_name.object;
        for (int i = 0; i < img.size(); i++) {
            min = std::min(min, *std::min_element(img[i].begin(), img[i].end()));
        }
        return find_equal(min)(img_with_name);
    }
};

struct find_min {
    with_name<std::set<point>> operator()(const with_name<image> &img_with_name) {
        uint8_t max = 0;
        const image& img = img_with_name.object;
        for (int i = 0; i < img.size(); i++) {
            max = std::max(max, *std::max_element(img[i].begin(), img[i].end()));
        }
        return find_equal(max)(img_with_name);
    }
};

struct highlight {
    size_t  size;
    highlight(size_t size_): size(size_) {}

    with_name<image> operator()(const std::tuple<
            with_name<image>,
            with_name<std::set<point>>,
            with_name<std::set<point>>,
            with_name<std::set<point>>
    > selected_points) {
        const image &img = std::get<0>(selected_points).object;
        size_t height = img.size();
        size_t width = img[0].size();

        image new_img = img;
        for (int i = 0; i < height; i++) {
            for (int j = 0; j < width; j++) {
                bool selected = std::get<1>(selected_points).object.count(point(i, j))
                                || std::get<2>(selected_points).object.count(point(i, j))
                                || std::get<3>(selected_points).object.count(point(i, j));
                if (!selected) continue;
                for (int new_i = std::max(0, i - (int) size); new_i < std::min(i + size + 1, height); new_i++) {
                    for (int new_j = std::max(0, j - (int) size); new_j < std::min(j + size + 1, width); new_j++) {
                        new_img[new_i][new_j] = 255;
                    }
                }
            }
        }
        return with_name<image>(std::get<0>(selected_points).name, new_img);
    }
};

struct inverse {
    with_name<image> operator()(const with_name<image> &img_with_name) {
        const image &img = img_with_name.object;
        image new_img = img;
        for (int i = 0; i < img.size(); i++) {
            for (int j = 0; j < img[0].size(); j++) {
                new_img[i][j] = (uint8_t) (255 - new_img[i][j]);
            }
        }
        return with_name<image>(img_with_name.name, new_img);
    }
};

struct mean {
    with_name<double> operator() (const with_name<image> &img_with_name) {
        const image &img = img_with_name.object;
        double sum = 0;
        for (int i = 0; i < img.size(); i++) {
            for (int j = 0; j < img[0].size(); j++) {
                sum += img[i][j];
            }
        }
        return with_name<double>(img_with_name.name, sum / img.size() / img[0].size());
    }
};

struct continue_msg {
    tbb::flow::continue_msg operator() (std::tuple<with_name<image>, with_name<double>> x) {
        return tbb::flow::continue_msg();
    }
};

struct options {
    uint8_t val_to_find = 255;
    size_t image_limit = 1;
    std::string file_name = "";
};


options get_options(char **first, char **last) {
    options opt = options();
    while (first != last) {
        if (strcmp(*first, "-b") == 0) {
            first++;
            if (first != last) {
                opt.val_to_find = (uint8_t) atoi(*first);
                first++;
            }
        } else if (strcmp(*first, "-l") == 0) {
            first++;
            if (first != last) {
                opt.image_limit = (size_t) atoi(*first);
                first++;
            }
        } else if (strcmp(*first, "-f") == 0) {
            first++;
            if (first != last) {
                opt.file_name = *first;
                first++;
            }
        } else first++;
    }
    return opt;
}

int main(int argc, char **argv) {
    options opt = get_options(argv, argv + argc);
    if (opt.file_name != "") {
        freopen(opt.file_name.c_str(), "w", stdout);
    }
    tbb::flow::graph g;
    tbb::flow::limiter_node<with_name<image>> input_node(g, opt.image_limit);

    tbb::flow::function_node<with_name<image>, with_name<std::set<point>>>
            max_node(g, tbb::flow::unlimited, find_max());
    tbb::flow::function_node<with_name<image>, with_name<std::set<point>>>
            min_node(g, tbb::flow::unlimited, find_min());
    tbb::flow::function_node<with_name<image>, with_name<std::set<point>>>
            equal_node(g, tbb::flow::unlimited, find_equal(opt.val_to_find));


    tbb::flow::join_node<
            std::tuple<with_name<image>, with_name<std::set<point>>, with_name<std::set<point>>, with_name<std::set<point>>>,
            tbb::flow::key_matching<std::string>
    > join_node(g, get_name<image>(), get_name<std::set<point>>(), get_name<std::set<point>>(), get_name<std::set<point>>());

    tbb::flow::function_node<
            std::tuple<with_name<image>, with_name<std::set<point>>, with_name<std::set<point>>, with_name<std::set<point>>>,
            with_name<image>
    > highlight_node(g, tbb::flow::unlimited, highlight(1));

    tbb::flow::function_node<with_name<image>, with_name<image>> inverse_node(g, tbb::flow::unlimited, inverse());
    tbb::flow::function_node<with_name<image>, with_name<double>> mean_node(g, tbb::flow::unlimited, mean());

    tbb::flow::buffer_node<with_name<double>> mean_buffer(g);

    tbb::flow::join_node<std::tuple<with_name<image>, with_name<double>>, tbb::flow::key_matching<std::string>>
            terminal_node(g, get_name<image>(), get_name<double>());

    tbb::flow::function_node<std::tuple<with_name<image>, with_name<double>>, tbb::flow::continue_msg>
            decrement_node(g, tbb::flow::unlimited, continue_msg());

    tbb::flow::make_edge(input_node, max_node);
    tbb::flow::make_edge(input_node, min_node);
    tbb::flow::make_edge(input_node, equal_node);

    tbb::flow::make_edge(input_node, tbb::flow::input_port<0>(join_node));
    tbb::flow::make_edge(max_node, tbb::flow::input_port<1>(join_node));
    tbb::flow::make_edge(min_node, tbb::flow::input_port<2>(join_node));
    tbb::flow::make_edge(equal_node, tbb::flow::input_port<3>(join_node));

    tbb::flow::make_edge(join_node, highlight_node);

    tbb::flow::make_edge(highlight_node, inverse_node);
    tbb::flow::make_edge(highlight_node, mean_node);

    tbb::flow::make_edge(mean_node, mean_buffer);

    tbb::flow::make_edge(highlight_node, tbb::flow::input_port<0>(terminal_node));
    tbb::flow::make_edge(mean_node, tbb::flow::input_port<1>(terminal_node));

    tbb::flow::make_edge(terminal_node, decrement_node);
    tbb::flow::make_edge(decrement_node, input_node.decrement);

    const size_t IMAGE_COUNT = 100;
    for (int i = 0; i < IMAGE_COUNT; i++) {
        with_name<image> img = with_name<image>(std::to_string(i), gen_random_image(100, 100));
        while (!input_node.try_put(img));
    }

    g.wait_for_all();
    for (int i = 0; i < IMAGE_COUNT; i++) {
        with_name<double> mean_value;
        if (mean_buffer.try_get(mean_value)) {
            std::cout << mean_value.name << " " << mean_value.object << "\n";
        }
    }
    return 0;
}