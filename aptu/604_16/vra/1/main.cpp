#include <iostream>
#include <fstream>
#include <iomanip>
#include <cstdlib>
#include <sstream> 
#include <vector>
#include <tbb/flow_graph.h>

using namespace std;
using namespace tbb::flow;

const int rows = 10;
const int cols = 10;
const int total_pictures = 10000;


template <class T>
struct message {
    message() {};
    message(int key, T value): key(key), value(value) {}
    int key;
    T value;
};

struct pixel {
    pixel() {};
    pixel(int x, int y): x(x), y(y) {}
    int x;
    int y;
};

typedef unsigned char uc;
typedef unsigned char** picture;
typedef unsigned char const * const * const c_picture;
typedef vector<pixel> pixels;

int image_number = 0;

template <class T>
int get_key(message<T> msg) {
    return msg.key;
}

void backlight_pixel(uc** p, pixel pi) {
    for (int i = -1; i <= 1; ++i) {
        for (int j = -1; j <= 1; ++j) {
            if (pi.x + i >= 0 && pi.x + i < rows && pi.y + j >= 0 && pi.y + j < cols && !(i == 0 && j == 0)) {
                p[pi.x + i][pi.y + j] = 255;
            }
        }
    }
}


int main(int argc, const char** argv) {

    int max_pictures = 100;
    int user_value = -1;
    string file_name = "";

    for (int i = 1; i < argc; ++i) {
        if (strcmp(argv[i], "-b") == 0) {
            user_value = (uc) atoi(argv[++i]);
        }
        else if (strcmp(argv[i], "-l") == 0) {
            max_pictures = atoi(argv[++i]);
        }
        else if (strcmp(argv[i], "-f") == 0) {
            file_name = argv[++i];
        }
        else {
            cout << "-b <byte> | -l <int> | -f <string>\n";
        }
    }

    graph g;

    source_node<message<picture>> source(g, [&](message<picture> &p) -> bool {
        if (image_number >= total_pictures) return false;
        p.value = new uc*[rows];
        for (int i = 0; i < rows; ++i) {
            p.value[i] = new uc[cols];
            for (int j = 0; j < cols; ++j) {
                p.value[i][j] = rand() % 256;
            }
        }
        p.key = image_number++;
        return true;
    });

    limiter_node<message<picture>> limiter(g, max_pictures);

    join_node<tuple<message<picture>, message<pixels>, message<pixels>, message<pixels>>, tag_matching> join1(g, 
        &get_key<picture>, &get_key<pixels>, &get_key<pixels>, &get_key<pixels>);

    function_node<message<picture>, tuple<message<picture>, message<picture>, message<picture>, message<picture>>> 
        pre_split1(g, unlimited,
        [](message<picture> p) {
            tuple<message<picture>, message<picture>, message<picture>, message<picture>> n(p, p, p, p);
            return n;
        }
    );

    split_node<tuple<message<picture>, message<picture>, message<picture>, message<picture>>> split1(g);

    function_node<message<picture>, message<pixels>> max(g, unlimited, [&](message<picture> p) -> message<pixels> { 
        uc m = 0;
        for (int i = 0; i < rows; ++i) {
            for (int j = 0; j < cols; ++j) {
                m = p.value[i][j] > m ? p.value[i][j]: m;
            }
        }
        pixels v;
        for (int i = 0; i < rows; ++i) {
            for (int j = 0; j < cols; ++j) {
                if(p.value[i][j] == m) v.push_back(pixel(i, j));
            }
        }
        return message<pixels>(p.key, v);
    });

    function_node<message<picture>, message<pixels>> min(g, unlimited, [&](message<picture> p) -> message<pixels> { 
        uc m = 255;
        for (int i = 0; i < rows; ++i) {
            for (int j = 0; j < cols; ++j) {
                m = p.value[i][j] < m ? p.value[i][j]: m;
            }
        }
        pixels v;
        for (int i = 0; i < rows; ++i) {
            for (int j = 0; j < cols; ++j) {
                if(p.value[i][j] == m) v.push_back(pixel(i, j));
            }
        }
        return message<pixels>(p.key, v);
    });

    function_node<message<picture>, message<pixels>> user(g, unlimited, [&](message<picture> p) -> message<pixels> { 
        pixels v;
        for (int i = 0; i < rows; ++i) {
            for (int j = 0; j < cols; ++j) {
                if(p.value[i][j] == user_value) v.push_back(pixel(i, j));
            }
        }
        return message<pixels>(p.key, v);
    });    

    function_node<tuple<message<picture>, message<pixels>, message<pixels>, message<pixels>>, message<picture>> backlight(g,
        unlimited, 
        [&](tuple<message<picture>, message<pixels>, message<pixels>, message<pixels>> t) -> message<picture> { 
            picture p = get<0>(t).value;
            uc** np = new uc*[rows];
            for (int i = 0; i < rows; ++i) {
                np[i] = new uc[cols];
                for (int j = 0; j < cols; ++j) {
                    np[i][j] = p[i][j];
                }
                delete p[i];
            }
            delete p;
            
            for (int i = 0; i < get<1>(t).value.size(); ++i) {
                pixel pi = get<1>(t).value[i];
                backlight_pixel(np, pi);
            }
            for (int i = 0; i < get<2>(t).value.size(); ++i) {
                pixel pi = get<2>(t).value[i];
                backlight_pixel(np, pi);
            }
            for (int i = 0; i < get<3>(t).value.size(); ++i) {
                pixel pi = get<3>(t).value[i];
                backlight_pixel(np, pi);
            }
            return message<picture>(get<0>(t).key, np);
        }
    );

    function_node<message<picture>, tuple<message<picture>, message<picture>, message<picture>>> 
        pre_split2(g, unlimited,
        [](message<picture> p) {
            tuple<message<picture>, message<picture>, message<picture>> n(p, p, p);
            return n;
        }
    );

    split_node<tuple<message<picture>, message<picture>, message<picture>>> split2(g);

    function_node<message<picture>, message<int>> inverse(g, unlimited, [&](message<picture> p) -> message<int> { 
        stringstream ss;
        ss << p.key << "\n";
        for (int i = 0; i < rows; ++i) {
            for (int j = 0; j < cols; ++j) {
                ss << setw(3) << 255 - p.value[i][j] << " ";
            }
            ss << "\n";
        }
        ss << "\n";
        cout << ss.str();
        return message<int>(p.key, 0);
    });

    function_node<message<picture>, message<int>> mean(g, unlimited, [&](message<picture> p) -> message<int> { 
        if (file_name != "") {
            int mean = 0;
            for (int i = 0; i < rows; ++i) {
                for (int j = 0; j < cols; ++j) {
                    mean += p.value[i][j];
                }
            }
            mean /= rows * cols;
            ofstream of(file_name, fstream::app);
            of << p.key << " - " << mean << endl;
        }
        return message<int>(p.key, 0);
    });

    join_node<tuple<message<picture>, message<int>, message<int>>, tag_matching> join2(g, 
        &get_key<picture>, &get_key<int>, &get_key<int>);

    function_node<tuple<message<picture>, message<int>, message<int>>, continue_msg> end1(g, unlimited, 
        [&](tuple<message<picture>, message<int>, message<int>> t) -> continue_msg {
            message<picture> p = get<0>(t);
            for (int i = 0; i < rows; ++i) {
                delete p.value[i];
            }
            delete p.value;
            return continue_msg();
        }
    );


    make_edge(source, limiter);

    make_edge(limiter, pre_split1);

    make_edge(pre_split1, split1);

    make_edge(output_port<0>(split1), input_port<0>(join1));

    make_edge(output_port<1>(split1), max);
    make_edge(max, input_port<1>(join1));

    make_edge(output_port<2>(split1), min);
    make_edge(min, input_port<2>(join1));

    make_edge(output_port<3>(split1), user);
    make_edge(user, input_port<3>(join1));

    make_edge(join1, backlight);

    make_edge(backlight, pre_split2);

    make_edge(pre_split2, split2);

    make_edge(output_port<0>(split2), input_port<0>(join2));
    
    make_edge(output_port<1>(split2), inverse);
    make_edge(inverse, input_port<1>(join2));

    make_edge(output_port<2>(split2), mean);
    make_edge(mean, input_port<2>(join2));

    make_edge(join2, end1);
    
    make_edge(end1, limiter.decrement);


    g.wait_for_all();

    return 0;
}