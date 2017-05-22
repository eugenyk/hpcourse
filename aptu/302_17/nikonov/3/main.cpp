#include <iostream>
#include <set>
#include <tbb/flow_graph.h>
#include <cassert>
#include <ctime>

#include "Picture.h"
#include "Options.h"

using namespace std;
using namespace tbb;

const size_t TEST_N = 1000;

template<typename T>
struct KeyToObj {
    int key;
    T obj;

    KeyToObj() {}

    KeyToObj(int _key, T _obj) : key(_key), obj(_obj) {}
};

typedef KeyToObj<Picture> PicAndKey;
typedef KeyToObj<set<Pnt>> PntsAndKey;
typedef set<Pnt> Points;


void print_usage() {
    cout << "USAGE: hw03.out -b <Brightness in range[0, 255]> -l <Limit of images processing at the same time> "
            "-f <output file name>" << endl;
}

template<typename T>
struct getKey {
    int operator()(const KeyToObj<T> &obj) {
        return obj.key;
    }
};

struct MinBrightness {
    PntsAndKey operator()(const PicAndKey &pic) {
        set<Pnt> result;
        u_char cur_min = 255;
        for (size_t i = 0; i < pic.obj.height; i++) {
            for (size_t j = 0; j < pic.obj.width; j++) {
                if (pic.obj.pic[i][j] < cur_min) {
                    cur_min = pic.obj.pic[i][j];
                    result.clear();
                }
                if (pic.obj.pic[i][j] == cur_min) {
                    result.insert(make_pair(i, j));
                }
            }
        }
        return PntsAndKey(pic.key, result);
    }
};

struct MaxBrightness {
    PntsAndKey operator()(const PicAndKey &pic) {
        set<Pnt> result;
        u_char cur_max = 0;
        for (size_t i = 0; i < pic.obj.height; i++) {
            for (size_t j = 0; j < pic.obj.width; j++) {
                if (pic.obj.pic[i][j] > cur_max) {
                    cur_max = pic.obj.pic[i][j];
                    result.clear();
                }
                if (pic.obj.pic[i][j] == cur_max) {
                    result.insert(make_pair(i, j));
                }
            }
        }
        return PntsAndKey(pic.key, result);
    }
};

struct CntPntsWithBrightness {
    u_char value;

    CntPntsWithBrightness(u_char _value) : value(_value) {}

    PntsAndKey operator()(const PicAndKey &pic) {
        set<Pnt> result;
        for (size_t i = 0; i < pic.obj.height; i++) {
            for (size_t j = 0; j < pic.obj.width; j++) {
                if (pic.obj.pic[i][j] == value) {
                    result.insert(make_pair(i, j));
                }
            }
        }
        return PntsAndKey(pic.key, result);
    }
};

struct Highlight {
    static bool contains(const Points points, const Pnt pnt) {
        return points.find(pnt) != points.end();
    }

    static void recolor(Picture &pic, const Pnt pnt) {
        int x = (int) get<0>(pnt);
        int y = (int) get<1>(pnt);
        for (int i = max(x - 1, 0); i < pic.height; i++) {
            for (int j = max(y - 1, 0); j < pic.width; j++) {
                pic.pic[i][j] = 255;
            }
        }
    }

    PicAndKey operator()(const tuple<PicAndKey, PntsAndKey, PntsAndKey, PntsAndKey> &data) {
        Picture newpic = get<0>(data).obj;
        for (size_t i = 0; i < newpic.height; i++) {
            for (size_t j = 0; j < newpic.width; j++) {
                if (contains(get<1>(data).obj, make_pair(i, j)) || contains(get<2>(data).obj, make_pair(i, j)) ||
                    contains(get<3>(data).obj, make_pair(i, j))) {
                    recolor(newpic, make_pair(i, j));
                }
            }
        }
        return PicAndKey(get<0>(data).key, newpic);
    }
};

struct Inverse {
    PicAndKey operator()(const PicAndKey &picAndKey) {
        Picture newpic = picAndKey.obj;
        for (size_t i = 0; i < newpic.height; i++) {
            for (size_t j = 0; j < newpic.width; j++) {
                newpic.pic[i][j] = (u_char) (255 - picAndKey.obj.pic[i][j]);
            }
        }
        return PicAndKey(picAndKey.key, newpic);
    }
};

struct Mean {
    KeyToObj<size_t> operator()(const PicAndKey &picAndKey) {
        size_t res = 0;
        for (size_t i = 0; i < picAndKey.obj.height; i++) {
            for (size_t j = 0; j < picAndKey.obj.width; j++) {
                res += picAndKey.obj.pic[i][j];
            }
        }
        return KeyToObj<size_t>(picAndKey.key, res / (picAndKey.obj.height * picAndKey.obj.width));
    }
};

int main(int argc, char **argv) {
#ifndef DEBUG
    std::srand((unsigned int) std::time(NULL));
#else
    std::srand(123);
#endif

    Options *options;
    try {
        options = new Options(argc, argv);
    } catch (invalid_argument arg) {
        cout << arg.what() << endl;
        print_usage();
        return -1;
    }

    if (options->log_name != "") {
        assert(freopen(options->log_name.c_str(), "w", stdout));
    }

    flow::graph graph;
    flow::limiter_node<PicAndKey> start(graph, options->limit);
    flow::function_node<PicAndKey, PntsAndKey> min_node(graph, flow::unlimited, MinBrightness());
    flow::function_node<PicAndKey, PntsAndKey> max_node(graph, flow::unlimited, MaxBrightness());
    flow::function_node<PicAndKey, PntsAndKey> cnt_node(graph, flow::unlimited,
                                                        CntPntsWithBrightness(options->brightness));
    flow::join_node<tuple<PicAndKey, PntsAndKey, PntsAndKey, PntsAndKey>, flow::key_matching<int>> join_node(graph,
                                                                                                             getKey<Picture>(),
                                                                                                             getKey<Points>(),
                                                                                                             getKey<Points>(),
                                                                                                             getKey<Points>());

    flow::function_node<tuple<PicAndKey, PntsAndKey, PntsAndKey, PntsAndKey>, PicAndKey> hl_node(graph, flow::unlimited,
                                                                                                 Highlight());
    flow::function_node<PicAndKey, PicAndKey> inv_node(graph, flow::unlimited, Inverse());
    flow::function_node<PicAndKey, KeyToObj<size_t>> mean_node(graph, flow::unlimited, Mean());
    flow::join_node<tuple<PicAndKey, KeyToObj<size_t>>, flow::key_matching<int>> join_node1(graph, getKey<Picture>(),
                                                                                            getKey<size_t>());
    flow::buffer_node<KeyToObj<size_t>> buffer(graph);
    flow::function_node<tuple<PicAndKey, KeyToObj<size_t> >, flow::continue_msg> end_node(graph, flow::unlimited,
                                                                                       [](const tuple<PicAndKey, KeyToObj<size_t>>) -> flow::continue_msg {
                                                                                           return flow::continue_msg();
                                                                                       });
    flow::make_edge(start, min_node);
    flow::make_edge(start, max_node);
    flow::make_edge(start, cnt_node);
    flow::make_edge(start, flow::input_port<0>(join_node));
    flow::make_edge(min_node, flow::input_port<1>(join_node));
    flow::make_edge(max_node, flow::input_port<2>(join_node));
    flow::make_edge(cnt_node, flow::input_port<3>(join_node));

    flow::make_edge(join_node, hl_node);
    flow::make_edge(hl_node, mean_node);
    flow::make_edge(mean_node, buffer);
    flow::make_edge(hl_node, inv_node);

    flow::make_edge(inv_node, flow::input_port<0>(join_node1));
    flow::make_edge(mean_node, flow::input_port<1>(join_node1));
    flow::make_edge(join_node1, end_node);
    flow::make_edge(end_node, start.decrement);

    for (size_t i = 0; i < TEST_N; i++) {
        Picture nxtPic(40, 40);
        nxtPic.fillWithRandomValues();
        PicAndKey picAndKey = KeyToObj<Picture>((int)i, nxtPic);
        while (!start.try_put(picAndKey));
    }

    graph.wait_for_all();

    for (int i = 0; i < TEST_N; i++) {
        KeyToObj<size_t> nxtmean;
        if (buffer.try_get(nxtmean)) {
            cout << nxtmean.key << " " << nxtmean.obj << endl;
        }
    }
    delete options;
    return 0;
}