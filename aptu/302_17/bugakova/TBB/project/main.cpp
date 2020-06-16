/**
 * Images need to be placed in the work directory of project.
 */

#include <iostream>
#include <tbb/flow_graph.h>
#include <cstdio>
#include<string>
#include <cassert>
#include <dirent.h>

class Image;

typedef std::vector<std::pair<int, int>> vp;
typedef std::pair<Image, vp> vpp;

const std::string prefix = "img";

class Image {
private:
    int n, m;
    std::vector<std::vector<int>> img;
    std::string name;
public:
    Image() {}
    Image(int n, int m, std::vector<std::vector<int>> img) {
        this->n = n;
        this->m = m;
        this->img = img;
    }
    std::string get_name() const& {
        return name;
    }
    Image readImage(std::string file_name) {
        name = file_name;
        freopen(file_name.c_str(), "r", stdin);
        scanf("%d ", &n);
        scanf("%d", &m);
        img.resize(n);
        for (int i = 0; i < n; i++) {
            img[i].resize(m);
            for (int j = 0; j < m; j++) {
                scanf("%d ", &img[i][j]);
            }
        }
        fclose(stdin);
        return *this;
    }
    void set(int i, int j, int value) {
        assert(0 <= i);
        assert(i < n);
        assert(0 <= j);
        assert(j < m);
        img[i][j] = value;
    }
    Image inverse() const& {
        std::vector<std::vector<int>> new_img;
        new_img.resize(n);
        for (int i = 0; i < n; i++) {
            new_img[i].resize(m);
            for (int j = 0; j < m; j++) {
                new_img[i][j] = 255 - img[i][j];
            }
        }
        return *(new Image(n, m, new_img));
    }
    double mean_bright() const& {
        double res = 0;
        for (int i = 0; i < n; i++) {
            for (int j = 0; j < m; j++) {
                res += img[i][j];
            }
        }
        return res/(n*m);
    }
    vp find_min_pnts() const& {
        int min_bright = 256;
        vp pnts;
        for (int i = 0; i < n; i++) {
            for (int j = 0; j < m; j++) {
                if (min_bright > img[i][j]) {
                    min_bright = img[i][j];
                    pnts.clear();
                } else if (min_bright == img[i][j]) {
                    pnts.push_back(std::make_pair(i, j));
                }
            }
        }
        return pnts;
    }
    vp find_max_pnts() const& {
        int max_bright = 0;
        vp pnts;
        for (int i = 0; i < n; i++) {
            for (int j = 0; j < m; j++) {
                if (max_bright < img[i][j]) {
                    max_bright = img[i][j];
                    pnts.clear();
                } else if (max_bright == img[i][j]) {
                    pnts.push_back(std::make_pair(i, j));
                }
            }
        }
        return pnts;
    }
    vp find_const_pnts(int value) const& {
        vp pnts;
        for (int i = 0; i < n; i++) {
            for (int j = 0; j < m; j++) {
                if (img[i][j] == value) {
                    pnts.push_back(std::make_pair(i, j));
                }
            }
        }
        return pnts;
    }
    void draw_square(int i, int j) {
        for (int ii = i - 1; ii <= i + 1; ii++) {
            for (int jj = j - 1; jj <= j + 1; jj++) {
                if (ii >= 0 && ii < n && jj >= 0 && jj < m) {
                    img[ii][jj] = 255;
                }
            }
        }
    }
    void draw_squares(vp pnts) {
        for (auto pnt: pnts) {
            draw_square(pnt.first, pnt.second);
        }
    }
};

std::vector<Image> read_imgs() {
    std::vector<Image> imgs;
    DIR* cur_dir = opendir("./");
    dirent* ent;
    if (cur_dir != NULL) {
        while ((ent = readdir(cur_dir)) != NULL) {
            std::string file_name = ent->d_name;
            if (file_name.substr(0, 3) == prefix) {
                imgs.push_back(Image().readImage(file_name));
            }
        }
    }
    return imgs;
}

int main(int args, char* argv[]) {
    int bright_of_interest = -1;
    int max_imgs = -1;
    std::string log_file = "";

    for (int i = 1; i < args; i++) {
        std::string word = argv[i];
        if (i + 1 >= args) {
            std::cerr << "can't find the value for flag" << std::endl;
            return 1;
        }
        if (word == "-b") {
            if (bright_of_interest != -1) {
                std::cerr << "wrong flags(duplicate)" << std::endl;
                return 1;
            }
            bright_of_interest = atoi(argv[++i]);
        } else if (word == "-l") {
            if (max_imgs != -1) {
                std::cerr << "wrong flags(duplicate)" << std::endl;
                return 1;
            }
            max_imgs = atoi(argv[++i]);
        } else if (word == "-f") {
            if (log_file != "") {
                std::cerr << "wrong flags(duplicate)" << std::endl;
                return 1;
            }
            log_file = argv[++i];
        } else {
            std::cerr << "wrong flag" << std::endl;
            return 1;
        }
    }

    std::vector<Image> imgs = read_imgs();

    tbb::flow::graph tbb_graph;
    tbb::flow::broadcast_node<Image> begin_node(tbb_graph);
    tbb::flow::function_node<Image, vpp> max_pnts(tbb_graph, max_imgs, [] (Image const& img) {
        return std::make_pair(img, img.find_max_pnts());
    });
    tbb::flow::function_node<Image, vpp> min_pnts(tbb_graph, max_imgs, [] (Image const& img) {
        return std::make_pair(img, img.find_min_pnts());
    });
    tbb::flow::function_node<Image, vpp> value_pnts(tbb_graph, max_imgs, [bright_of_interest] (Image const& img) {
        return std::make_pair(img, img.find_const_pnts(bright_of_interest));
    });
    tbb::flow::join_node<tbb::flow::tuple<vpp, vpp, vpp>> join(tbb_graph);
    tbb::flow::function_node<tbb::flow::tuple<vpp, vpp, vpp>, Image> draw_squares(
            tbb_graph, max_imgs, [] (tbb::flow::tuple<vpp, vpp, vpp> const& res) {
                vpp min_pnts = std::get<0>(res);
                vpp max_pnts = std::get<1>(res);
                vpp val_pnts = std::get<2>(res);

                min_pnts.first.draw_squares(min_pnts.second);
                max_pnts.first.draw_squares(max_pnts.second);
                val_pnts.first.draw_squares(val_pnts.second);

                return min_pnts.first;
    });
    tbb::flow::function_node<Image, Image> inverse_node(tbb_graph, max_imgs, [] (Image const& img) {
        return img.inverse();
    });
    tbb::flow::function_node<Image, std::pair<std::string, double>> mean_bright(tbb_graph, max_imgs, [] (Image const& img) {
        return std::make_pair(img.get_name(), img.mean_bright());
    });
    tbb::flow::buffer_node<std::pair<std::string, double>> end_node(tbb_graph);

    tbb::flow::make_edge(begin_node, min_pnts);
    tbb::flow::make_edge(begin_node, max_pnts);
    tbb::flow::make_edge(begin_node, value_pnts);
    tbb::flow::make_edge(min_pnts, tbb::flow::input_port<0>(join));
    tbb::flow::make_edge(max_pnts, tbb::flow::input_port<1>(join));
    tbb::flow::make_edge(value_pnts, tbb::flow::input_port<2>(join));
    tbb::flow::make_edge(join, draw_squares);
    tbb::flow::make_edge(draw_squares, inverse_node);
    tbb::flow::make_edge(draw_squares, mean_bright);
    tbb::flow::make_edge(mean_bright, end_node);

    for (int i = 0; i < (int)imgs.size(); i++) {
        assert(begin_node.try_put(imgs[i]));
    }

    tbb_graph.wait_for_all();

    if (log_file != "") {
        freopen(log_file.c_str(), "w", stdout);
        for (int i = 0; i < (int) imgs.size(); i++) {
            std::pair<std::string, double> cur_res;
            assert (end_node.try_get(cur_res));
            std::cout << cur_res.first << " " << cur_res.second << std::endl;
        }
        fclose(stdout);
    }

    return 0;
}