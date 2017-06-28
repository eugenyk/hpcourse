#include "tbb/flow_graph.h"
#include <iostream>
#include <random>
#include <string.h>
#include <fstream>
#include <thread>
#include <set>
#include <memory>

using namespace std;
using namespace tbb::flow;

typedef uint8_t brightness;

struct user_params {
    brightness custom_br;
    unsigned parall;
    string br_journal;
    bool journal_set;
    user_params() : journal_set(false) {}
};

struct image {
    static const size_t N = 100;
    static const size_t M = 200;
    static const size_t SUMMARY = N * M;
    brightness matrix[N][M];
    void clear() {
        memset(matrix, 0, sizeof(matrix));
    }
    static bool bounds(int x, int y) {
        return x >=0 && y >= 0 && x < N && y < M;
    }
    brightness at(int x, int y) {
        return matrix[x][y];
    }
};

const size_t HIGHTLIGHT_RECT = 3;

struct local_image {
    static const size_t N = 10;
    brightness matrix[N][N];
};

user_params parse_opts(int argc, char** argv) {
    user_params param;

    int opt;

    while ((opt = getopt(argc, argv, "b:l:f:")) != -1) {
       switch (opt) {
       case 'b':
           param.custom_br = atoi(optarg);
           break;
       case 'l':
           param.parall = atoi(optarg);
           break;
       case 'f':
           param.br_journal = string(optarg);
           param.journal_set = true;
           break;
       default:
           cerr << "Unknown option";
           exit(1);
       }
    }
    return param;
}

void gen_msg(image & ret) {
    std::random_device rd;
    std::mt19937 mt(rd());
    for(size_t i = 0; i < ret.N; i++)
        for(size_t j = 0; j < ret.M; j++) {
            std::uniform_int_distribution<brightness> dist(0, std::numeric_limits<brightness>::max());
            ret.matrix[i][j] = dist(mt);
        }
}

class brigtness_proc {
public:
    struct pixel {
        unsigned x;
        unsigned y;
        pixel(unsigned x, unsigned y) : x(x), y(y) {}
    };
    enum mode_t{
        MAX,
        MIN,
        LEVEL
    } ;
    brigtness_proc(mode_t mode, brightness level=0) : mode(mode), level(level) {}
    void init() {
        if (mode == MAX)
            level = std::numeric_limits<brightness>::min();
        else if(mode == MIN)
            level = std::numeric_limits<brightness>::max();
    }

    void update(brightness r) {
        if (mode == MAX)
           level = std::max(level, r);
       else if(mode == MIN)
           level = std::min(level, r);
    }
    vector<pixel> operator()(shared_ptr<image> image) {
        init();
        for(unsigned i = 0; i< image->N; i++)
            for(unsigned j = 0; j < image->M; j++)
                update(image->matrix[i][j]);

        vector<pixel> ret;
        for(unsigned i = 0; i< image->N; i++)
           for(unsigned j = 0; j < image->M; j++) {
               if (image->matrix[i][j] == level) {
                   ret.push_back(pixel(i, j));
               }
           }
        return ret;
    }
private:
    brightness level;
    mode_t mode;
};


class highlight_proc {
    ssize_t rect;
    void hightlight_rect(image * r, brigtness_proc::pixel pixel, image * old_image) {
        for(int x = -rect; x <= rect; x++)
            for(int y = -rect ; y <= rect; y++) {
                int i = x + pixel.x;
                int j = y + pixel.y;
                if (image::bounds(i, j)) {
                    r->matrix[i][j] = old_image->at(x, y);
                }
            }
    }
    void proc_for(image * ret, vector<brigtness_proc::pixel> &s, image * old_image) {
        for(auto p : s) {
            hightlight_rect(ret, p, old_image);
        }
    }
public:
    highlight_proc(const size_t rect) : rect(rect) {}
    shared_ptr<image> operator()(
            std::tuple<
            shared_ptr<image>,
            vector<brigtness_proc::pixel> ,
            vector<brigtness_proc::pixel> ,
            vector<brigtness_proc::pixel>
            > src) {
        image * old_image = get<0>(src).get();
        image * ret = new image();
        ret->clear();

        proc_for(ret, get<1>(src), old_image);
        proc_for(ret, get<2>(src), old_image);
        proc_for(ret, get<3>(src), old_image);
        return shared_ptr<image>(ret);
    }
};

class inverse_proc {
public:
    shared_ptr<image> operator()(shared_ptr<image> im) {
        image * ret = new image();
        for (int i = 0; i < im->N; i++)
            for(int j = 0; j < im->M; j++)
                ret->matrix[i][j] = std::numeric_limits<brightness>::max() - im->at(i, j);
        return shared_ptr<image>(ret);
    }
};

class avg_proc {
public:
    double operator()(shared_ptr<image> im) {
        double ret = 0;
        for (int i = 0; i < im->N; i++)
            for(int j = 0; j < im->M; j++)
                ret += im->at(i, j);
        return ret / im->SUMMARY;
    }
};

class print_proc {
    std::ostream &os;
public:
    print_proc(std::ostream &os)  : os(os){

    }
    double operator()(double ret) {
        os << "Average caclulated " << ret << endl;
        return ret;
    }
};


void build_graph(user_params & params) {
    ostream * log = params.journal_set ? new std::ofstream(params.br_journal) : &cout;
    graph g;
    source_node<shared_ptr<image>> src(g, [](shared_ptr<image> &ptr ) ->bool {
        image * im = new image();
        gen_msg(*im);
        ptr.reset(im);
        return true;}
    );

    limiter_node<shared_ptr<image>> lim (g, params.parall);

    function_node<shared_ptr<image>, vector<brigtness_proc::pixel>> br_min(g, unlimited, brigtness_proc(brigtness_proc::MIN));
    function_node<shared_ptr<image>, vector<brigtness_proc::pixel>> br_max(g, unlimited, brigtness_proc(brigtness_proc::MAX));
    function_node<shared_ptr<image>, vector<brigtness_proc::pixel>> br_cust(g, unlimited, brigtness_proc(brigtness_proc::LEVEL, params.custom_br));

    join_node< std::tuple<
    shared_ptr<image>,
    vector<brigtness_proc::pixel> ,
    vector<brigtness_proc::pixel> ,
    vector<brigtness_proc::pixel>
    > > jn(g);

    function_node< std::tuple<
    shared_ptr<image>,
    vector<brigtness_proc::pixel> ,
    vector<brigtness_proc::pixel> ,
    vector<brigtness_proc::pixel>
    >,  shared_ptr<image>> hi(g, unlimited, highlight_proc(HIGHTLIGHT_RECT));

    function_node<shared_ptr<image>, shared_ptr<image> > inv(g, unlimited, inverse_proc());
    function_node<shared_ptr<image>, double> avg(g, unlimited, avg_proc());

    function_node<double, double> pr(g, unlimited, print_proc(*log));

    join_node<std::tuple<shared_ptr<image>, double>> fin(g);


    queue_node<int> node(g);
    make_edge(src, lim);
    make_edge(lim, br_min);
    make_edge(lim, br_max);
    make_edge(lim, br_cust);

    make_edge(lim, input_port<0>(jn));
    make_edge(br_min, input_port<1>(jn));
    make_edge(br_max, input_port<2>(jn));
    make_edge(br_cust, input_port<3>(jn));

    make_edge(jn, hi);

    make_edge(hi, inv);
    make_edge(hi, avg);

    make_edge(avg, pr);

    make_edge(inv, input_port<0>(fin));
    make_edge(pr, input_port<1>(fin));


    g.wait_for_all();
    if (params.journal_set)
        delete log;
}

int main(int argc, char** argv) {
   user_params param = parse_opts(argc, argv);
   cerr << "User opts : custom_br = " << (int)param.custom_br << ", parall =  " << param.parall
           << ", br_journal = " << param.br_journal << "\n";
   build_graph(param);
   return 0;
}
