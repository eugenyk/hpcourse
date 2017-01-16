#include "random_image.h"
#include <random>
#include <algorithm>

static std::random_device rd;
static std::mt19937 gen(rd());
static std::uniform_int_distribution<uint16_t> uniform(0, 255);

random_image::random_image() { }

random_image::random_image(uint32_t height, uint32_t width): _data(std::vector<std::vector<uint8_t>>(height, std::vector<uint8_t>(width))) {
  for (size_t i = 0; i < height; ++i) {
    for (size_t j = 0; j < width; ++j) {
      _data[i][j] = static_cast<uint8_t>(uniform(gen));
    }
  }
}

random_image& random_image::operator=(random_image const& other) {
  _data = other._data;
  return *this;
}

std::vector<point> random_image::find(uint8_t value) const {
  std::vector<point> result{};
  size_t height = _data.size();
  size_t width = _data[0].size();
  for (size_t i = 0; i < height; ++i) {
    for (size_t j = 0; j < width; ++j) {
      if (_data[i][j] == value) {
        result.push_back(std::make_pair(static_cast<uint8_t>(i), static_cast<uint8_t>(j)));
      }
    }
  }

  return result;
}

uint8_t random_image::min() const {
  uint8_t min = std::numeric_limits<uint8_t>().max();
  size_t height = _data.size();
  size_t width = _data[0].size();
  for (size_t i = 0; i < height; ++i) {
    for (size_t j = 0; j < width; ++j) {
      min = std::min(min, _data[i][j]);
    }
  }

  return min;
}

uint8_t random_image::max() const {
  uint8_t max = std::numeric_limits<uint8_t>().min();
  size_t height = _data.size();
  size_t width = _data[0].size();
  for (size_t i = 0; i < height; ++i) {
    for (size_t j = 0; j < width; ++j) {
      max = std::max(max, _data[i][j]);
    }
  }

  return max;
}

double random_image::mean() const {
  size_t height = _data.size();
  size_t width = _data[0].size();
  int64_t sum = 0;
  for (size_t i = 0; i < height; ++i) {
    for (size_t j = 0; j < width; ++j) {
      sum += _data[i][j];
    }
  }

  return static_cast<double>(sum) / (height * width);
}

void random_image::inverse() {
  size_t height = _data.size();
  size_t width = _data[0].size();
  for (size_t i = 0; i < height; ++i) {
    for (size_t j = 0; j < width; ++j) {
      _data[i][j] = MAX_BRIGHTNESS - _data[i][j];
    }
  }
}

void random_image::highlight(uint8_t specified_brightness, std::vector<point> const& specified_points, std::vector<point> const& min_points, std::vector<point> const& max_points) {
  uint8_t min = _data[min_points[0].first][min_points[0].second];
  uint8_t max = _data[max_points[0].first][max_points[0].second];
  for (point const& p : specified_points) {
    highlight_point(p.first, p.second, specified_brightness, max, min);
  }

  for (point const& p : min_points) {
    highlight_point(p.first, p.second, specified_brightness, max, min);
  }

  for (point const& p : max_points) {
    highlight_point(p.first, p.second, specified_brightness, max, min);
  }
}

void random_image::highlight_point(size_t i, size_t j, uint8_t spec_brightness, uint8_t max, uint8_t min) {
  size_t upper = i - HIGHLIGHTING_FRAME_SIZE;
  size_t lower = i + HIGHLIGHTING_FRAME_SIZE;
  size_t left = j - HIGHLIGHTING_FRAME_SIZE;
  size_t right = j + HIGHLIGHTING_FRAME_SIZE;
  for (size_t k = 0; k < HIGHLIGHTING_FRAME_SIZE + 2; ++k) {
    fill(upper, left + k, MAX_BRIGHTNESS);
    fill(lower, left + k, MAX_BRIGHTNESS);
    fill(upper + k, left, MAX_BRIGHTNESS);
    fill(upper + k, right, MAX_BRIGHTNESS);
  }
}

void random_image::fill(size_t i, size_t j, uint8_t value) {
  if (0 <= i && i < _data.size() && 0 <= j && j < _data[i].size()) {
    _data[i][j] = value;
  }
}
