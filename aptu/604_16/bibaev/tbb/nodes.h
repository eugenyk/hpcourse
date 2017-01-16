#pragma once
#include <tbb/flow_graph.h>
#include <tuple>
#include <ostream>
#include "random_image.h"

struct generator_node {
  generator_node(size_t limit);
  bool operator()(random_image& image);

private:
  size_t _generated_count;
  size_t _limit;
};

struct eq_node {
  eq_node(uint8_t target);
  std::vector<point> operator()(random_image const& image) const;

private:
  uint8_t _target_value;
};

struct min_node {
  std::vector<point> operator()(random_image const& image) const;
};

struct max_node {
  std::vector<point> operator()(random_image const& image) const;
};

struct highlight_node {
  highlight_node(uint8_t brightness);

  // Contract: spec, min, max
  random_image operator()(std::tuple<random_image, std::vector<point>, std::vector<point>, std::vector<point>> node_input) const;

private:
  uint8_t _specified_brightness;
};

struct mean_node {
  double operator()(random_image image) const;
};

struct print_node {
  print_node(std::ostream& log);
  tbb::flow::continue_msg operator()(double mean) const;

private:
  std::ostream& _log;
};

struct inverse_node {
  random_image operator()(random_image image) const;
};
