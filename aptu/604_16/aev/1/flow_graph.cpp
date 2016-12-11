#define TBB_PREVIEW_GRAPH_NODES 1
#include "tbb/flow_graph.h"
#include "tbb/atomic.h"
#include "tbb/spin_mutex.h"

#include <random>
#include <fstream>
#include <iomanip>
#include <set>

using namespace std;
using namespace tbb;
using namespace tbb::flow;

struct image_t {
    vector<vector<unsigned char>> img;
    size_t w, h;
    size_t id;
};

struct search_result_t {
    vector<pair<size_t, size_t>> pos;
    unsigned char value;
};

class image_generator_body {
public:
    image_generator_body(size_t count, size_t w, size_t h)
        : _count(count)
        , _w(w)
        , _h(h)
        , _idx(0)
    {}

    bool operator()(image_t & output) {
        size_t id = _idx++;
        if (id >= _count) {
            return false;
        }
        default_random_engine gen(0xA1B2C3D4 ^ id);
        uniform_int_distribution<int> rnd(0, 255);

        output = image_t{
            { _h, vector<unsigned char>(_w, 0) },
            _w,
            _h,
            id
        };

        for (size_t i = 0; i < output.h; ++i) {
            for (size_t j = 0; j < output.w; ++j) {
                output.img[i][j] = rnd(gen);
            }
        }
        return true;
    }
private:
    size_t _count;
    size_t _w;
    size_t _h;
    size_t _idx;
};

class min_search_func_body {
public:
    search_result_t operator()(const image_t & input) {
        search_result_t result;
        for (size_t i = 0; i < input.h; ++i) {
            for (size_t j = 0; j < input.w; ++j) {
                result.value = min(result.value, input.img[i][j]);
            }
        }
        for (size_t i = 0; i < input.h; ++i) {
            for (size_t j = 0; j < input.w; ++j) {
                if (input.img[i][j] == result.value) {
                    result.pos.push_back(make_pair(i, j));
                }
            }
        }
        return result;
    }
};

class max_search_func_body {
public:
    search_result_t operator()(const image_t & input) {
        search_result_t result;
        for (size_t i = 0; i < input.h; ++i) {
            for (size_t j = 0; j < input.w; ++j) {
                result.value = max(result.value, input.img[i][j]);
            }
        }
        for (size_t i = 0; i < input.h; ++i) {
            for (size_t j = 0; j < input.w; ++j) {
                if (input.img[i][j] == result.value) {
                    result.pos.push_back(make_pair(i, j));
                }
            }
        }
        return result;
    }
};

class value_search_func_body {
public:
    value_search_func_body(unsigned char value) : _value(value) {}

    search_result_t operator()(const image_t & input) {
        search_result_t result{ {}, _value };
        for (size_t i = 0; i < input.h; ++i) {
            for (size_t j = 0; j < input.w; ++j) {
                if (input.img[i][j] == result.value) {
                    result.pos.push_back(make_pair(i, j));
                }
            }
        }
        return result;
    }
private:
    unsigned char _value;
};

class highlight_func_body {
public:
    highlight_func_body(unsigned char value) : _value(value) {}

    image_t operator()(const flow::tuple< image_t, search_result_t, search_result_t, search_result_t > & input) {
        image_t image = get<0>(input);
        vector<vector<bool>> search_results(image.h, vector<bool>(image.w, false));
        for (auto p : get<1>(input).pos) {
            search_results[p.first][p.second] = true;
        }
        for (auto p : get<2>(input).pos) {
            search_results[p.first][p.second] = true;
        }
        for (auto p : get<3>(input).pos) {
            search_results[p.first][p.second] = true;
        }
        for (size_t i = 0; i < image.h; ++i) {
            for (size_t j = 0; j < image.w; ++j) {
                if (search_results[i][j]) {
                    continue;
                }
                bool has_neighbour_result = false;
                for (int di = -1; di <= 1; ++di) {
                    for (int dj = -1; dj <= 1; ++dj) {
                        if (di == 0 && dj == 0) continue;
                        int pi = i + di, pj = j + dj;
                        if (valid(image, pi, pj) && search_results[pi][pj]) {
                            has_neighbour_result = true;
                        }
                    }
                }
                if (has_neighbour_result) {
                    image.img[i][j] = _value;
                }
            }
        }
        return image;
    }
private:
    bool valid(const image_t & img, int r, int c) {
        return r >= 0 && size_t(r) < img.h && c >= 0 && size_t(c) < img.w;
    }
    unsigned char _value;
};

class inverse_func_body {
public:
    image_t operator()(const image_t & input) {
        image_t result(input);
        for (size_t i = 0; i < input.h; ++i) {
            for (size_t j = 0; j < input.w; ++j) {
                result.img[i][j] = 255 - result.img[i][j];
            }
        }
        return result;
    }
};

class avg_func_body {
public:
    double operator()(const image_t & input) {
        double result = 0.0;
        for (size_t i = 0; i < input.h; ++i) {
            for (size_t j = 0; j < input.w; ++j) {
                result += input.img[i][j];
            }
        }
        return result / input.w / input.h;
    }
};

class results_func_body {
public:
    results_func_body(ofstream & out) : _out(out) {}

    continue_msg operator()(const flow::tuple< image_t, double > & input) {
        if (_out) {
            spin_mutex::scoped_lock lock(_write_lock);
            _out << setprecision(2) << fixed << get<0>(input).id << ": " << get<1>(input) << endl;
        }
        return continue_msg();
    }
private:
    ofstream& _out;
    static spin_mutex _write_lock;
};
spin_mutex results_func_body::_write_lock;


constexpr size_t HIGHLIGHT_VALUE = 255;

int main(int argc, char *argv[]) {
    unsigned char search_value = 42;
    size_t parallel_limit = 2;
    string output_file;

    size_t images_count = 10;
    size_t images_width = 512;
    size_t images_height = 512;

    for (int i = 1; i < argc; ++i) {
        if (!strcmp(argv[i], "-b") && i + 1 < argc) {
            search_value = atoi(argv[i + 1]);
            ++i;
        }
        if (!strcmp(argv[i], "-l") && i + 1 < argc) {
            parallel_limit = atoi(argv[i + 1]);
            ++i;
        }
        if (!strcmp(argv[i], "-f") && i + 1 < argc) {
            output_file = argv[i + 1];
            ++i;
        }

        if (!strcmp(argv[i], "-c") && i + 1 < argc) {
            images_count = atoi(argv[i + 1]);
            ++i;
        }
        if (!strcmp(argv[i], "-w") && i + 1 < argc) {
            images_width = atoi(argv[i + 1]);
            ++i;
        }
        if (!strcmp(argv[i], "-h") && i + 1 < argc) {
            images_height = atoi(argv[i + 1]);
            ++i;
        }
    }

    ofstream output;
    if (!output_file.empty()) {
        output.open(output_file);
    }

    graph flow_graph;

    source_node< image_t > image_generator(flow_graph, image_generator_body(images_count, images_width, images_height), false);
    limiter_node< image_t > limiter(flow_graph, parallel_limit, 0);
    broadcast_node< image_t > image_broadcast(flow_graph);
    function_node< image_t, search_result_t > min_search_func(flow_graph, unlimited, min_search_func_body());
    function_node< image_t, search_result_t > max_search_func(flow_graph, unlimited, max_search_func_body());
    function_node< image_t, search_result_t > value_search_func(flow_graph, unlimited, value_search_func_body(search_value));
    join_node< flow::tuple< image_t, search_result_t, search_result_t, search_result_t > > search_joiner(flow_graph);
    function_node< flow::tuple< image_t, search_result_t, search_result_t, search_result_t >, image_t >
        highlight_func(flow_graph, unlimited, highlight_func_body(HIGHLIGHT_VALUE));
    broadcast_node< image_t > highlighted_image_broadcast(flow_graph);
    function_node< image_t, image_t > inverse_func(flow_graph, unlimited, inverse_func_body());
    function_node< image_t, double > avg_func(flow_graph, unlimited, avg_func_body());
    join_node< flow::tuple< image_t, double > > output_join(flow_graph);
    function_node< flow::tuple< image_t, double >, continue_msg >
        results_func(flow_graph, unlimited, results_func_body(output));
    make_edge(image_generator, limiter);
    make_edge(limiter, image_broadcast);
    make_edge(image_broadcast, min_search_func);
    make_edge(image_broadcast, max_search_func);
    make_edge(image_broadcast, value_search_func);
    make_edge(image_broadcast, input_port<0>(search_joiner));
    make_edge(min_search_func, input_port<1>(search_joiner));
    make_edge(max_search_func, input_port<2>(search_joiner));
    make_edge(value_search_func, input_port<3>(search_joiner));
    make_edge(search_joiner, highlight_func);
    make_edge(highlight_func, highlighted_image_broadcast);
    make_edge(highlighted_image_broadcast, inverse_func);
    make_edge(highlighted_image_broadcast, avg_func);
    make_edge(inverse_func, input_port<0>(output_join));
    make_edge(avg_func, input_port<1>(output_join));
    make_edge(output_join, results_func);
    make_edge(results_func, limiter.decrement);
    image_generator.activate();
    flow_graph.wait_for_all();
    return 0;
}
