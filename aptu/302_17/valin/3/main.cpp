#include <iostream>
#include <vector>
#include <set>
#include <tbb/flow_graph.h>
#include <algorithm>


typedef std::tuple<int, int> point;
typedef uint8_t u8;
typedef std::vector<std::vector<u8>> image;

image random_image() {
    const static size_t maxw = 100;
    const static size_t maxh = 100;

    size_t h = std::max<size_t>(1, ((size_t) random()) % maxh);
    size_t w = std::max<size_t>(1, ((size_t) random()) % maxw);

    image img;

    for (int i = 0; i < h; ++i) {
        std::vector<u8> row;
        for (int j = 0; j < w; ++j) {
            u8 dot = (u8) random();
            row.push_back(dot);
        }
        img.push_back(row);
    }

    return img;
}

template <typename T>
struct tagged {
    int tag;
    T obj;
    tagged() {}
    tagged(int tag, T obj) : tag(tag), obj(obj) {}
};

template <typename T>
struct get_tag {
    int operator()(const tagged<T> &a) {
        return a.tag;
    }
};

struct Equiv {
    bool fake = false;
    const u8 val;

    Equiv(int v) : val((u8) v) {
        if (v < 0)
            fake = true;
    }

    tagged<std::set<point>> operator()(const tagged<image>& t_img) {
        const image& img = t_img.obj;
        std::set<point> val_set;
        if (!fake) {
            for (int i = 0; i < img.size(); ++i)
                for (int j = 0; j < img[0].size(); ++j)
                    if (img[i][j] == val)
                        val_set.insert(std::make_pair(i, j));
        }
        return tagged<std::set<point>>(t_img.tag, val_set);
    }
};

struct Max {
    tagged<std::set<point>> operator()(const tagged<image>& t_img) {
        const image& img = t_img.obj;
        std::set<point> max_set;
        u8 val = 0;
        for (int i = 0; i < img.size(); ++i)
            for (int j = 0; j < img[0].size(); ++j) {
                if (img[i][j] == val)
                    max_set.insert(std::make_pair(i, j));
                if (img[i][j] > val) {
                    max_set.clear();
                    max_set.insert(std::make_pair(i, j));
                    val = img[i][j];
                }
            }

        return tagged<std::set<point>>(t_img.tag, max_set);
    }
};

struct Min {
    tagged<std::set<point>> operator()(const tagged<image>& t_img) {
        const image& img = t_img.obj;
        std::set<point> min_set;
        u8 val = 255;

        for (int i = 0; i < img.size(); ++i)
            for (int j = 0; j < img[0].size(); ++j) {
                if (img[i][j] == val)
                    min_set.insert(std::make_pair(i, j));
                if (img[i][j] < val) {
                    min_set.clear();
                    min_set.insert(std::make_pair(i, j));
                    val = img[i][j];
                }
            }

        return tagged<std::set<point>>(t_img.tag, min_set);
    }
};

struct Highlight {
    tagged<image> operator() (const std::tuple<tagged<image>, tagged<std::set<point>>, tagged<std::set<point>>, tagged<std::set<point>>>& img_and_points) {
        image img = std::get<0>(img_and_points).obj;
        const std::set<point>& pt1 = std::get<1>(img_and_points).obj;
        const std::set<point>& pt2 = std::get<2>(img_and_points).obj;
        const std::set<point>& pt3 = std::get<3>(img_and_points).obj;
        std::set<point> pt;
        pt.insert(pt1.begin(), pt1.end());
        pt.insert(pt2.begin(), pt2.end());
        pt.insert(pt3.begin(), pt3.end());

        for (point p : pt) {
            highlight(img, p);
        }

        return tagged<image>(std::get<0>(img_and_points).tag, img);
    }

    static void highlight(image &img, point &p) {
        int ip = std::get<0>(p);
        int jp = std::get<1>(p);

        for (int i = -1; i < 2; ++i) {
            for (int j = -1; j < 2; ++j) {
                if (ip + i < 0 || jp + j < 0
                    || img[0].size() <= jp + j || img.size() <= ip + i
                    || i == 0 && j == 0)
                    continue;
                img[ip + i][jp + j] = 255;
            }
        }
    }
};

struct Inverse {
    tagged<image> operator() (const tagged<image>& t_img) {
        image img = t_img.obj;
        for (auto r : img)
            for (auto& p : r)
                p = -p;

        return tagged<image>(t_img.tag, img);
    }
};

struct Mean {
    tagged<int> operator() (const tagged<image>& t_img) {
        const image& img = t_img.obj;
        int n = img[0].size() * img.size();
        unsigned int s = 0;
        for (auto r : img)
            for (auto p : r)
                s += p;

        return tagged<int>(t_img.tag, s / n);
    }
};

struct Options {
    int brightness = -1;
    size_t limit = 1;
} options;

void parse_options(Options &options, int argc, char** argv) {
    int i = 1;
    while (i < argc) {
        std::string arg = std::string(argv[i]);
        if (arg == "-b") {
            options.brightness = atoi(argv[i + 1]);
            i += 2;
        } else if (arg == "-l") {
            options.limit = (size_t) atoi(argv[i + 1]);
            i += 2;
        } else if (arg == "-f") {
            freopen(argv[i + 1], "w", stdout);
            i += 2;
        } else {
            throw std::runtime_error("bad options");
        }
    }
}


int main(int argc, char **argv) {
    parse_options(options, argc, argv);

    tbb::flow::graph graph;
    tbb::flow::limiter_node<tagged<image>> input(graph, options.limit);
    tbb::flow::function_node<tagged<image>, tagged<std::set<point>>> equiv_node(graph, tbb::flow::unlimited, Equiv(options.brightness));
    tbb::flow::function_node<tagged<image>, tagged<std::set<point>>> min_node(graph, tbb::flow::unlimited, Min());
    tbb::flow::function_node<tagged<image>, tagged<std::set<point>>> max_node(graph, tbb::flow::unlimited, Max());
    tbb::flow::join_node<
            std::tuple<tagged<image>, tagged<std::set<point>>, tagged<std::set<point>>, tagged<std::set<point>>>,
            tbb::flow::key_matching<int>
    > join_node(graph, get_tag<image>(), get_tag<std::set<point>>(), get_tag<std::set<point>>(), get_tag<std::set<point>>());

    tbb::flow::function_node<std::tuple<tagged<image>, tagged<std::set<point>>, tagged<std::set<point>>, tagged<std::set<point>>>,
            tagged<image>
    > highlight_node(graph, tbb::flow::unlimited, Highlight());
    tbb::flow::function_node<tagged<image>, tagged<image>> inverse_node(graph, tbb::flow::unlimited, Inverse());
    tbb::flow::function_node<tagged<image>, tagged<int>> mean_node(graph, tbb::flow::unlimited, Mean());
    tbb::flow::join_node<
            std::tuple<tagged<image>, tagged<int>>,
            tbb::flow::key_matching<int>
    > join_node_notify(graph, get_tag<image>(), get_tag<int>());

    tbb::flow::function_node<std::tuple<tagged<image>, tagged<int>>, tbb::flow::continue_msg>
            notifier(graph, tbb::flow::unlimited, [] (std::tuple<tagged<image>, tagged<int>> o) ->
                tbb::flow::continue_msg {
                    return tbb::flow::continue_msg();
                });

    tbb::flow::buffer_node<tagged<int>> mean_buffer(graph);

    tbb::flow::make_edge(input, equiv_node);
    tbb::flow::make_edge(input, min_node);
    tbb::flow::make_edge(input, max_node);

    tbb::flow::make_edge(input, tbb::flow::input_port<0>(join_node));
    tbb::flow::make_edge(equiv_node, tbb::flow::input_port<1>(join_node));
    tbb::flow::make_edge(min_node, tbb::flow::input_port<2>(join_node));
    tbb::flow::make_edge(max_node, tbb::flow::input_port<3>(join_node));

    tbb::flow::make_edge(join_node, highlight_node);

    tbb::flow::make_edge(highlight_node, inverse_node);
    tbb::flow::make_edge(highlight_node, mean_node);
    tbb::flow::make_edge(mean_node, mean_buffer);

    tbb::flow::make_edge(inverse_node, tbb::flow::input_port<0>(join_node_notify));
    tbb::flow::make_edge(mean_node, tbb::flow::input_port<1>(join_node_notify));
    tbb::flow::make_edge(join_node_notify, notifier);
    tbb::flow::make_edge(notifier, input.decrement);

    const int N = 10000;

    for (int i = 0; i < N; ++i) {
        tagged<image> t_img = tagged<image>(i, random_image());
        while (!input.try_put(t_img));
    }

    graph.wait_for_all();
    for (int i = 0; i < N; ++i) {
        tagged<int> m;
        if (mean_buffer.try_get(m))
            std::cout << m.tag << ":" << m.obj << '\n';
    }

    return 0;
}
