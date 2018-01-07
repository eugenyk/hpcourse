#include <tbb/flow_graph.h>
#include "nodes.h"
#include <cstdlib>
#include <fstream>
#include <memory>
#include <sstream>

using namespace tbb::flow;
using std::ofstream;

int main(int argc, char *argv[]) {
	
	uint interestingBright = 127;
	uint maxImg = 5;
	bool withLog = false;
	std::string logFile = "log.txt";

	for (int i = 1; i < argc; ++i) {
        if (std::string(argv[i]) == "-b") {
            std::stringstream ss(std::string(argv[i+1]));
            ss >> interestingBright;
        } else if (std::string(argv[i]) == "-l") {
            std::stringstream ss(std::string(argv[i+1]));
            ss >> maxImg;
        } else if (std::string(argv[i]) == "-f") {
            logFile = std::string(argv[i+1]);
			withLog = true;
       } else {
            puts("Error while parsing args");
            puts("Usage:");
            puts("-b <int>    - interesting bright value");
            puts("-l <int>    - max parallel process img");
            puts("-f <string> - log file");
			return 0;
        }
		++i;
    }

	ofstream out(logFile);
    srand(time(NULL));

	graph g; 
    source_node<std::shared_ptr<image>> src(g, img_creator());
	limiter_node<std::shared_ptr<image>> limit(g, maxImg);
	broadcast_node<std::shared_ptr<image>> broad_1(g);

    function_node<std::shared_ptr<image>, vector<pair<int, int>>> find_max_node(g, tbb::flow::unlimited, find_max());
    function_node<std::shared_ptr<image>, vector<pair<int, int>>> find_min_node(g, tbb::flow::unlimited, find_min());
    function_node<std::shared_ptr<image>, vector<pair<int, int>>> find_val_node(g, tbb::flow::unlimited, find_val(interestingBright));

    join_node<tbb::flow::tuple<std::shared_ptr<image>, vector<pair<int, int>>, vector<pair<int, int>>, vector<pair<int, int>>>, queueing> join(g);
	//broadcast_node<tbb::flow::tuple<std::shared_ptr<image>, vector<pair<int, int>>, vector<pair<int, int>>, vector<pair<int, int>>> broad_2(g);

    function_node<tbb::flow::tuple<std::shared_ptr<image>, vector<pair<int, int>>, vector<pair<int, int>>, vector<pair<int, int>>>, bool>
        invert_img_node(g, tbb::flow::unlimited, invert_img());
    function_node<tbb::flow::tuple<std::shared_ptr<image>, vector<pair<int, int>>, vector<pair<int, int>>, vector<pair<int, int>>>, std::string>
        average_img_node(g, tbb::flow::unlimited, average_img());

	function_node<std::string, bool> print_log_node(g, tbb::flow::unlimited, print_log(out));

	make_edge(src, limit);
	make_edge(limit, broad_1);

    make_edge(broad_1, find_max_node);
    make_edge(broad_1, find_min_node);
    make_edge(broad_1, find_val_node);

    make_edge(broad_1, input_port<0>(join));
    make_edge(find_max_node, input_port<1>(join));
    make_edge(find_min_node, input_port<2>(join));
    make_edge(find_val_node, input_port<3>(join));

	//make_edge(join, broad_2);

    make_edge(join, invert_img_node);
    make_edge(join, average_img_node);

	if (withLog) {
		make_edge(average_img_node, print_log_node);
	}

    src.activate();
	g.wait_for_all();
 
	return 0;
}