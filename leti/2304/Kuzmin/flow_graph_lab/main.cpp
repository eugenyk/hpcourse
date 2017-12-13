#include <tbb/flow_graph.h>
#include <CImg.h>
#include <fstream>
#include <vector>

using namespace tbb::flow;
using namespace std;
using namespace cimg_library;

typedef tuple<CImg, string> ImgTuple; //image + name
typedef tuple<ImgTuple, vector<int[2]>, vector<int[2]>, vector<int[2]>> ImgDotsTuple; //image + max, min, custom dots

string intens_file, imgs_file;
int find_intens, threads;
ifstream infile = NULL;

//source_node
struct load_imgs {
  bool operator()(ImgTuple& output) {
    if (!infile) infile(imgs_file);
    string img_name;
    if (infile >> img_name)
    {
        CImg data(img_name.c_str());
        output(data, img_name);
        return true;
    }
    infile.close();
    return false;
  }
};

//func_node
struct calc_intens {
  ImgDotsTuple operator()(ImgTuple& input) {

    return NULL;
  }
};

int main(int argc, char *argv[]) {
    int result = 0;

    graph g;
    source_node<CImg> load_imgs_node(g, load_imgs(), false);
    broadcast_node<CImg> bcast_imgs(g);
    buffer_node<CImg> calc_intens_queue(g);
    buffer_node<CImg> invert_queue(g);
    buffer_node<ImgDotsTuple> bcast_dots_queue(g);
    broadcast_node<ImgDotsTuple> bcast_dots(g);
    buffer_node<ImgDotsTuple> select_dots_queue(g);
    buffer_node<ImgDotsTuple> calc_avg_intens_queue(g);
    buffer_node<CImg> save_imgs_queue(g);
    function_node<ImgTuple, ImgDotsTuple> calc_intens_node( g, threads, calc_intens() );

    return 0;
}
