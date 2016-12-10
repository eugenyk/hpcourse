#pragma once
#include <cstdint>
#include <vector>

using point = std::pair<uint8_t, uint8_t>;
const uint8_t MAX_BRIGHTNESS = 255;
const size_t HIGHLIGHTING_FRAME_SIZE = 1;

struct random_image {
  random_image();
  random_image(uint32_t height, uint32_t width);
  random_image& operator=(random_image const& other);

  std::vector<point> find(uint8_t value) const;

  uint8_t min() const;
  uint8_t max() const;
  double mean() const;

  void inverse();
  void highlight(uint8_t specified_brightness, std::vector<point> const& specified_points, std::vector<point> const& min_points, std::vector<point> const& max_points);

private:
  std::vector<std::vector<uint8_t>> _data;

  void highlight_point(size_t i, size_t j, uint8_t spec_brightness, uint8_t max, uint8_t min);
  void fill(size_t i, size_t j, uint8_t value);
};
