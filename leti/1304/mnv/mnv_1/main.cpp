#include <QCoreApplication>
#include "colormanagement.h"

using namespace tbb::flow;
using namespace std;

int main(int argc, char* argv[]) {
    srand(time(NULL));

    tuple<int, int, int, string> params = Utils::parseArguments(argc, argv);
    int n = get<0>(params);
    int l = get<1>(params);
    int b = get<2>(params);
    string f = get<3>(params);

    ofstream file;
    if (!f.empty())
    {
        file = ofstream(f, std::ios_base::app);
        file << "---------------------\n";
    }

    graph g;

    source_node<Image*> src_node(g, source_body(n), false);
    limiter_node<Image*> lim_node(g, l);
    function_node<Image*, vector<int>> max_brightness_node(g, unlimited, max_brightness());
    function_node<Image*, vector<int>> min_brightness_node(g, unlimited, min_brightness());
    function_node<Image*, vector<int>> cnt_brightness_node(g, unlimited, cnt_brightness(b));
    join_node<tuple<Image*, vector<int>, vector<int>, vector<int>>, queueing> join(g);
    function_node<tuple<Image*, vector<int>, vector<int>, vector<int>>, Image*> highlight_node(g, unlimited, highlight());
    function_node<Image*, Image*> process_inv_node(g, unlimited, process_inv());
    function_node<Image*, tuple<int, int>> process_avg_node(g, unlimited, process_avg(file));
    function_node<tuple<int, int>, int> write_avg_node(g, serial, write_avg(file));
    join_node<tuple<Image*, int>> eop_join(g);
    function_node<tuple<Image*, int>, continue_msg> eop_node(g, unlimited, eop_body());

    make_edge(src_node, lim_node);
    make_edge(lim_node, max_brightness_node);
    make_edge(lim_node, min_brightness_node);
    make_edge(lim_node, cnt_brightness_node);
    make_edge(lim_node, input_port<0>(join));
    make_edge(max_brightness_node, input_port<1>(join));
    make_edge(min_brightness_node, input_port<2>(join));
    make_edge(cnt_brightness_node, input_port<3>(join));
    make_edge(join, highlight_node);
    make_edge(highlight_node, process_inv_node);
    make_edge(highlight_node, process_avg_node);
    make_edge(process_avg_node, write_avg_node);
    make_edge(process_inv_node, input_port<0>(eop_join));
    make_edge(write_avg_node, input_port<1>(eop_join));
    make_edge(eop_join, eop_node);
    make_edge(eop_node, lim_node.decrement);

    src_node.activate();
    g.wait_for_all();

    if (!f.empty())
        file.close();
    std::cout << "JOB DONE" << std::endl;
    return 0;
}

