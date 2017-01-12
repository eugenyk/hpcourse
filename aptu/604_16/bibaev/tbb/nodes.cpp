#include "nodes.h"

static const uint32_t IMAGE_HEIGHT = 1024;
static const uint32_t IMAGE_WIDTH = 1024;

generator_node::generator_node(size_t limit): _generated_count(0), _limit(limit) {}

bool generator_node::operator()(random_image& image) {
  if (_generated_count >= _limit) {
    return false;
  }

  image = random_image(IMAGE_HEIGHT, IMAGE_WIDTH);

  ++_generated_count;
  return true;
}

eq_node::eq_node(uint8_t target): _target_value(target) {}

std::vector<point> eq_node::operator()(random_image const& image) const {
  return image.find(_target_value);
}

std::vector<point> min_node::operator()(random_image const& image) const {
  return image.find(image.min());
}

std::vector<point> max_node::operator()(random_image const& image) const {
  return image.find(image.max());
}

highlight_node::highlight_node(uint8_t brightness): _specified_brightness(brightness) {}

random_image highlight_node::operator()(std::tuple<random_image, std::vector<point>, std::vector<point>, std::vector<point>> node_input) const {
  random_image image = std::get<0>(node_input);
  image.highlight(_specified_brightness, std::get<1>(node_input), std::get<2>(node_input), std::get<3>(node_input));
  return image;
}

double mean_node::operator()(random_image image) const {
  return image.mean();
}

print_node::print_node(std::ostream& log): _log(log) { }

tbb::flow::continue_msg print_node::operator()(double mean) const {
  _log << mean << std::endl;
  return tbb::flow::continue_msg();
}

random_image inverse_node::operator()(random_image image) const {
  random_image inversed(image);
  inversed.inverse();
  return inversed;
}
