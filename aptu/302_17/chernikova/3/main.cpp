#include "tbb/flow_graph.h"
#include <iostream>
#include <sstream>
#include <fstream>

const int N = 10, M = 10, MAX_VAL = 256;

int interestingBright = -1;
size_t maxProgressImg = 10;
std::string logFile = "log.txt";
std::ofstream fout;

struct img {
    std::string name;
    std::vector<std::vector<int>> data;
};

struct elems {
    std::string name;
    std::vector<std::pair<int, int> > data;
};

struct find_elems {
protected:
    elems find_eq(const img& img, int val) {
        elems elems;
        elems.name = img.name;
        for (int i = 0; i < (int)img.data.size(); ++i) {
            for (int j = 0; j < (int)img.data[i].size(); ++j) {
                if (img.data[i][j] == val) {
                    elems.data.push_back(std::make_pair(i, j));
                }
            }
        }
        return elems;
    }
};

struct find_max : public find_elems {
public:
    elems operator()(const img& img) {
        std::cerr << "find max " << img.name << std::endl;

        int mx = img.data[0][0];
        for (int i = 0; i < (int)img.data.size(); ++i) {
            for (int j = 0; j < (int)img.data.size(); ++j) {
                if (img.data[i][j] > mx) {
                    mx = img.data[i][j];
                }
            }
        }
        return find_eq(img, mx);
    }
};

struct find_min : public find_elems {
public:
    elems operator()(const img& img) {
        std::cerr << "find min " << img.name << std::endl;

        int mn = img.data[0][0];
        for (int i = 0; i < (int)img.data.size(); ++i) {
            for (int j = 0; j < (int)img.data.size(); ++j) {
                if (img.data[i][j] < mn) {
                    mn = img.data[i][j];
                }
            }
        }
        return find_eq(img, mn);
    }
};

struct find_eqv : public find_elems {
public:
    elems operator()(const img& img) {
        std::cerr << "find eq " << img.name << std::endl;
        return find_eq(img, interestingBright);
    }
};

struct getImgName {
public:
    std::string operator()(const img& img) {
        return img.name;
    }
};

struct getElemName {
public:
    std::string operator()(const elems& elems) {
        return elems.name;
    }
};

struct markBox {
private:
    void markOneSet(img &img, elems elems) {
        for (std::pair<int, int> coord : elems.data) {
            for (int i = -1; i <= 1; ++i) {
                for (int j = -1; j <= 1; ++j) {
                    if (i == 0 && j == 0) continue;
                    if (i + coord.first < 0 || i + coord.first >= img.data.size()) continue;
                    if (j + coord.second < 0 || j + coord.second >= img.data[0].size()) continue;
                    img.data[i + coord.first][j + coord.second] = 255;
                }
            }
        }
    }
public:
    img operator()(const std::tuple<img, elems, elems, elems>& tp) {
        img res = std::get<0>(tp);
        markOneSet(res, std::get<1>(tp));
        markOneSet(res, std::get<2>(tp));
        markOneSet(res, std::get<3>(tp));
        return res;
    }
};

struct inver {
public:
    img operator()(const img& imeg) {
        std::cerr << "invr " << imeg.name << std::endl;
        img res;
        res.name = imeg.name;
        res.data.resize(imeg.data.size());
        for (int i = 0; i < (int)imeg.data.size(); ++i) {
            res.data[i].resize(imeg.data[i].size());
            for (int j = 0; j < (int)imeg.data[i].size(); ++j) {
                res.data[i][j] = 255 - imeg.data[i][j];
            }
        }
        return res;
    }
};

struct mean {
public:
    std::pair<std::string, double> operator()(const img& img) {
        int sum = 0;
        for (int i = 0; i < (int)img.data.size(); ++i) {
            for (int j = 0; j < (int)img.data[i].size(); ++j) {
                sum += img.data[i][j];
            }
        }

        return std::make_pair(img.name, sum * 1.0/(img.data.size() * img.data[0].size()));
    }
};


struct printMean {
public:
    tbb::flow::continue_msg operator()(const std::pair<std::string, double>& x) {
        fout << "mean " << x.first << " " << x.second << std::endl;
        return tbb::flow::continue_msg();
    }
};


img genRandomMatrix(std::string name) {
    img img;
    img.name = name;
    img.data.resize(N);
    for (int i = 0; i < N; ++i) {
        for (int j = 0; j < M; ++j) {
            img.data[i].push_back(rand()%MAX_VAL);
        }
    }

    return img;
}

int main(int argc, char** argv) {
    for (int i = 1; i < argc; ++i) {
        if (std::string(argv[i]) == "-b") {
            ++i;
            std::stringstream ss(std::string(argv[i]));
            ss >> interestingBright;
        } else if (std::string(argv[i]) == "-l") {
            ++i;
            std::stringstream ss(std::string(argv[i]));
            ss >> maxProgressImg;
        } else if (std::string(argv[i]) == "-f") {
            ++i;
            logFile = std::string(argv[i]);
        } else {
            puts("error while parsing args");
            puts("Usage:");
            puts("-b <int>    - interesting bright value");
            puts("-l <int>    - max parallel process img");
            puts("-f <string> - log file");
        }
    }

    fout.open(logFile);

    tbb::flow::graph g;
    tbb::flow::limiter_node<img> input(g, maxProgressImg);
    tbb::flow::function_node<img, elems > max(g, tbb::flow::unlimited, find_max());
    tbb::flow::function_node<img, elems > min(g, tbb::flow::unlimited, find_min());
    tbb::flow::function_node<img, elems > eq(g, tbb::flow::unlimited, find_eqv());
    tbb::flow::join_node<std::tuple<img, elems, elems, elems>, tbb::flow::key_matching<std::string> > join_node(g, getImgName(), getElemName(), getElemName(), getElemName());
    tbb::flow::function_node<std::tuple<img, elems, elems, elems>, img> mark(g, tbb::flow::unlimited, markBox());
    tbb::flow::function_node<img, img> invr(g, tbb::flow::unlimited, inver());
    tbb::flow::function_node<img, std::pair<std::string, double>> eval_mean(g, tbb::flow::unlimited, mean());
    tbb::flow::function_node<std::pair<std::string, double>, tbb::flow::continue_msg> print(g, tbb::flow::unlimited, printMean());

    tbb::flow::make_edge(input, max);
    tbb::flow::make_edge(input, min);
    tbb::flow::make_edge(input, eq);

    tbb::flow::make_edge(input, tbb::flow::input_port<0>(join_node));
    tbb::flow::make_edge(max, tbb::flow::input_port<1>(join_node));
    tbb::flow::make_edge(min, tbb::flow::input_port<2>(join_node));
    tbb::flow::make_edge(eq, tbb::flow::input_port<3>(join_node));

    tbb::flow::make_edge(join_node, mark);
    tbb::flow::make_edge(mark, invr);
    tbb::flow::make_edge(mark, eval_mean);
    tbb::flow::make_edge(eval_mean, print);

    for (int i = 0; i < 10; ++i) {
        std::stringstream ss;
        ss << "img" << i;
        input.try_put(genRandomMatrix(ss.str()));
    }

    g.wait_for_all();
    fout.close();
    return 0;
}