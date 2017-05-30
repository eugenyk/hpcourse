#include <tbb/flow_graph.h>
#include "models.h"
#include <cstdlib>
#include <fstream>
#include <memory>

using namespace tbb::flow;
using std::ofstream;

int main(int argc, char *argv[]) {
	int result = 0;

    uint b = 360;
    int l = 5;
    ofstream out("output.txt");
    int w = 100;
    int h = 100;

    srand(time(NULL));

	graph g; 
    source_node<std::shared_ptr<image>> src(g, img_generator(w, h, l));
    function_node<std::shared_ptr<image>, vector<pair<int, int>>> find_max_node(g, l, find_max());
    function_node<std::shared_ptr<image>, vector<pair<int, int>>> find_min_node(g, l, find_min());
    function_node<std::shared_ptr<image>, vector<pair<int, int>>> find_val_node(g, l, find_val(b));
    join_node<std::tuple<std::shared_ptr<image>, vector<pair<int, int>>, vector<pair<int, int>>, vector<pair<int, int>>>, queueing> join(g);
    function_node<std::tuple<std::shared_ptr<image>, vector<pair<int, int>>, vector<pair<int, int>>, vector<pair<int, int>>>, bool>
        invert_img_node(g, l, invert_img());
    function_node<std::tuple<std::shared_ptr<image>, vector<pair<int, int>>, vector<pair<int, int>>, vector<pair<int, int>>>, bool>
        average_img_node(g, l, average_img(out));

    make_edge(src, find_max_node);
    make_edge(src, find_min_node);
    make_edge(src, find_val_node);
    make_edge(src, input_port<0>(join));
    make_edge(find_max_node, input_port<1>(join));
    make_edge(find_min_node, input_port<2>(join));
    make_edge(find_val_node, input_port<3>(join));
    make_edge(join, invert_img_node);
    make_edge(join, average_img_node);

    src.activate();
	g.wait_for_all();
 
	return 0;
}
