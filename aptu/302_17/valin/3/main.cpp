#include <iostream>
#include <vector>
#include <set>
#include <tbb/flow_graph.h>
#include <algorithm>
#include <boost/program_options/options_description.hpp>
#include <boost/program_options/variables_map.hpp>
#include <boost/program_options/parsers.hpp>


namespace po = boost::program_options;

typedef std::tuple<int, int> point;
typedef uint8_t u8;
typedef std::vector<std::vector<u8>> image;

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

        return t_img;
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

int main(int argc, char **argv) {
    int brightness = -1;
    size_t limit = 1;

    po::options_description desc;
    desc.add_options()
            ("b,b", po::value<int>(), "brightness")
            ("l,l", po::value<int>(), "limit")
            ("f,f", po::value<std::string>(), "log")
            ("i,i", po::value<std::vector<std::string>>()->multitoken(), "images");

    po::positional_options_description pos;
    pos.add("i,i", -1);

    po::variables_map vm;
    po::store(po::command_line_parser(argc, argv).options(desc).positional(pos).run(), vm);
    po::notify(vm);

    if (vm.count("b"))
        brightness = vm["b"].as<int>();

    if (vm.count("f"))
        freopen(vm["f"].as<std::string>().c_str(), "w", stdout);

    if (vm.count("l"))
        limit = vm["l"].as<int>();

    std::list<image> images;
    if (vm.count("i")) {
        for (std::string f : vm["i"].as<std::vector<std::string>>()) {
            image img;
            int w, h;
            freopen(f.c_str(), "r", stdin);
            std::cin >> h >> w;
            for (int i = 0; i < h; ++i) {
                std::vector<u8> row;
                for (int j = 0; j < w; ++j) {
                    int a;
                    std::cin >> a;
                    row.push_back(a);
                }
                img.push_back(row);
            }
            images.push_back(img);
        }
    }

    tbb::flow::graph graph;
    tbb::flow::broadcast_node<tagged<image>> input(graph);
    tbb::flow::function_node<tagged<image>, tagged<std::set<point>>> equiv_node(graph, limit, Equiv(brightness));
    tbb::flow::function_node<tagged<image>, tagged<std::set<point>>> min_node(graph,limit, Min());
    tbb::flow::function_node<tagged<image>, tagged<std::set<point>>> max_node(graph, limit, Max());
    tbb::flow::join_node<
            std::tuple<tagged<image>, tagged<std::set<point>>, tagged<std::set<point>>, tagged<std::set<point>>>,
            tbb::flow::key_matching<int>
    > join_node(graph, get_tag<image>(), get_tag<std::set<point>>(), get_tag<std::set<point>>(), get_tag<std::set<point>>());

    tbb::flow::function_node<std::tuple<tagged<image>, tagged<std::set<point>>, tagged<std::set<point>>, tagged<std::set<point>>>,
            tagged<image>
    > highlight_node(graph, limit, Highlight());
    tbb::flow::function_node<tagged<image>, tagged<image>> inverse_node(graph, limit, Inverse());
    tbb::flow::function_node<tagged<image>, tagged<int>> mean_node(graph, limit, Mean());
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

    int tag = 0;
    for (image img : images)
        input.try_put(tagged<image>(tag++, img));

    graph.wait_for_all();
    for (int i = 0; i < tag; ++i) {
        tagged<int> m;
        if (mean_buffer.try_get(m))
            std::cout << m.tag << ":" << m.obj << '\n';
    }


    return 0;
}