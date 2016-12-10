#include <tbb/flow_graph.h>
#include <iostream>
#include <boost/program_options.hpp>
#include <random>
#include <fstream>
#include "random_image.h"
#include "nodes.h"

using namespace tbb;
using namespace flow;
using namespace std;

int main(int argc, char** argv) {
  namespace po = boost::program_options;
  uint8_t brightness;
  size_t limit;
  string log;
  uint32_t count;

  po::options_description description("flow graph");
  description.add_options()
      (",b", po::value<uint8_t>(&brightness)->default_value(42), "Find pixel with specified color")
      ("limit,l", po::value<size_t>(&limit)->default_value(100), "Limit for images procedding simultaneously")
      ("file,f", po::value<string>(&log)->default_value("log.txt"), "File for log average values")
      ("count,c", po::value<uint32_t>(&count)->default_value(1000), "Count of images for precessing");

  po::variables_map parameters;
  try {
    po::store(po::command_line_parser(argc, argv).options(description).run(), parameters);
  }
  catch (boost::program_options::error const& e) {
    cerr << e.what() << endl;
    cout << description << endl;
    return 1;
  }

  notify(parameters);
  ofstream log_file{log};

  graph graph;

  source_node<random_image> src{graph, generator_node{count}};

  limiter_node<random_image> limiter{graph, limit};

  broadcast_node<random_image> first_broadcast{graph};

  function_node<random_image, vector<point>, rejecting> eq{graph, unlimited, eq_node{brightness}};
  function_node<random_image, vector<point>, rejecting> min{graph, unlimited, min_node{}};
  function_node<random_image, vector<point>, rejecting> max{graph, unlimited, max_node{}};

  join_node<tuple<random_image, vector<point>, vector<point>, vector<point>>, queueing> join{graph};

  function_node<tuple<random_image, vector<point>, vector<point>, vector<point>>, random_image> highlight{graph, unlimited, highlight_node{brightness}};

  broadcast_node<random_image> second_broadcast{graph};

  function_node<random_image, random_image, rejecting> inverse{graph, unlimited, inverse_node{}};
  function_node<random_image, double, rejecting> mean{graph, unlimited, mean_node{}};
  function_node<double> print{graph, unlimited, print_node{log_file}};

  // ex 1
  make_edge(src, limiter);
  make_edge(limiter, first_broadcast);

  // ex 2
  make_edge(first_broadcast, min);
  make_edge(first_broadcast, max);
  make_edge(first_broadcast, eq);

  // ex 3
  make_edge(first_broadcast, input_port<0>(join));
  make_edge(eq, input_port<1>(join));
  make_edge(min, input_port<2>(join));
  make_edge(max, input_port<3>(join));

  make_edge(join, highlight);

  // ex 4
  make_edge(highlight, second_broadcast);
  make_edge(second_broadcast, inverse);
  make_edge(second_broadcast, mean);
  make_edge(mean, print);
  make_edge(print, limiter.decrement);

  src.activate();
  graph.wait_for_all();
  log_file.close();
}
