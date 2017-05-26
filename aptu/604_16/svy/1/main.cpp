#include <tbb/flow_graph.h>
#include <random>
#include <functional>
#include <vector>
#include <utility>
#include <fstream>
#include <algorithm>
#include <thread>
#include <iostream>
#include <sstream>

using namespace tbb::flow;

const int size = 1000;
bool verbose = false;

void usage(const char* name) {
    std::cout << "USAGE: " << name << " -b <value> -f <file> -l <limit> [-v]" << std::endl;
}

void trace(const std::string& message) {
    if(verbose) {
        std::cout << "[" << std::this_thread::get_id() << "]: " << message << std::endl;
    }
}

class img_generator {
public:
    int** operator()(int dummy) {
        trace("generator");
        (void)dummy;
        int** mat = new int*[size];
        *mat = new int[size*size];
        for(int i = 1; i < size; ++i) {
            mat[i] = mat[i - 1] + size;
        }
        std::random_device rd;
        std::mt19937 mt(rd());
        std::uniform_int_distribution<> dist(0, 0x00ffffff);
        for(int i = 0; i < size*size; ++i) {
            mat[0][i] = dist(mt);
        }
        return mat;
    }
};

class lookup {
    std::function<int(int, int)> f;
public:
    lookup(std::function<int(int, int)> _f) : f(_f) {}
    std::pair<int**, std::vector<int>> operator()(int** mat) {
        trace("lookup");
        int r = mat[0][0];
        std::vector<int> points;
        for(int i = 1; i < size*size; ++i) {
            if(mat[0][i] == r) {
                points.push_back(i);
            } else {
                int t = f(mat[0][i], r);
                if(t != r) {
                    r = t;
                    points.clear();
                    points.push_back(i);
                }
            }
        }

        return std::make_pair(mat, std::move(points));
    }
};

typedef std::pair<int**, std::vector<int>> pair_t;

class painter {
    void draw_rect(int** mat, int pos) {
        int i = pos / size;
        int j = pos % size;
        int rect = 10;

        int l = std::max(0, j - rect);
        int r = std::min(size - 1, j + rect);
        int u = std::max(0, i - rect);
        int b = std::min(size - 1, i + rect);

        for(int k = l; k <= r; ++k) {
            mat[u][k] = mat[b][k] = 0x00ffffff;
        }
        for(int k = u; k <= b; ++k) {
            mat[k][l] = mat[k][r] = 0x00ffffff;
        }
    }
public:
    int** operator()(std::tuple<pair_t, pair_t, pair_t> args) {
        trace("painter");
        int** mat = std::get<0>(args).first;

        std::vector<int> points(std::get<0>(args).second);
        points.insert(points.end(), std::get<1>(args).second.begin(), std::get<1>(args).second.end());
        points.insert(points.end(), std::get<1>(args).second.begin(), std::get<1>(args).second.end());
        for(auto p : points) {
            draw_rect(mat, p);
        }
        return mat;
    }
};

class img_invertor {
public:
    int** operator()(int** mat) {
        trace("img_invertor");
        for(int i = 0; i < size*size; ++i) {
            mat[0][i] = 0x00ffffff - mat[0][i];
        }
        return mat;
    }
};

class avg_lookup {
public:
    double operator()(int** mat) {
        trace("avg_lookup");
        double r = 0;
        for(int i = 0; i < size*size; ++i) {
            r += mat[0][i]/(size*size);
        }

        return r;
    }
};

class file_writter {
    std::string name;
    std::ofstream file;
public:
    file_writter(const std::string& name) : name(name), file(name) {}
    file_writter(const file_writter& f) {
        name = f.name;
        file.open(name);
    }
    file_writter(file_writter&& f) {
        f.file.close();
        name = f.name;
        file.open(name);
    };
    void operator()(int avg) {
        trace("file_writter");
        file << "0x" << std::hex << avg << std::endl;
    }
};

int main(int argc, char* argv[]) {
    graph g;
    int p = -1;
    std::string filename;
    int limit = -1;

    for(int i = 1; i < argc; ++i) {
        if(std::string("-v") == argv[i]) {
            verbose = true;
            continue;
        } else if(std::string("-b") == argv[i] && i != argc - 1) {
            std::stringstream ss(argv[i+1]);
            ss >> std::hex >> p;
            i += 1;
            continue;
        } else if(std::string("-f") == argv[i] && i != argc - 1) {
            filename = argv[i + 1];
            i += 1;
            continue;
        } else if(std::string("-l") == argv[i] && i != argc - 1) {
            std::stringstream ss(argv[i+1]);
            ss >> limit;
            i += 1; 
            continue;
        } else {
            usage(argv[0]);
            return 1;
        }
    }

    if(p == -1 || filename.empty() || limit == -1) {
        usage(argv[0]);
        return 1;
    }

    function_node<int, int**> generator(g, limit, img_generator());
    broadcast_node<int**> splitter_1(g);
    function_node<int**, pair_t> max_lookup(g, unlimited, lookup([](int a, int b) { return std::max(a, b); }));
    function_node<int**, pair_t> min_lookup(g, unlimited, lookup([](int a, int b) { return std::min(a, b); }));
    function_node<int**, pair_t> strict_lookup(g, unlimited, lookup([p](int a, int b) { return a == p || b == p ? p : -1; }));
    join_node<std::tuple<pair_t, pair_t, pair_t>, queueing> join(g);
    function_node<std::tuple<pair_t, pair_t, pair_t>, int**> drawer(g, unlimited, painter());
    broadcast_node<int**> splitter_2(g);
    function_node<int> writter(g, serial, file_writter(filename));
    function_node<int**, int**> invertor(g, unlimited, img_invertor());
    function_node<int**, int> avg(g, unlimited, avg_lookup());

    make_edge(generator, splitter_1);
    make_edge(splitter_1, max_lookup);
    make_edge(splitter_1, min_lookup);
    make_edge(splitter_1, strict_lookup);
    make_edge(max_lookup, input_port<0>(join));
    make_edge(min_lookup, input_port<1>(join));
    make_edge(strict_lookup, input_port<2>(join));
    make_edge(join, drawer);
    make_edge(drawer, splitter_2);
    make_edge(splitter_2, invertor);
    make_edge(splitter_2, avg);
    make_edge(avg, writter);
    

    for (int i = 1; i <= 10; ++i)
		generator.try_put(1);
	g.wait_for_all();

}
