#include "image.h"

#include <iostream>

image::image(size_t height, size_t width, std::unique_ptr<int[]> data, size_t id)
        : height_(height)
        , width_(width)
        , data_(std::move(data)) {
    static size_t static_id = 1;
    if (id == 0)
        id_ = static_id++;
    else
        id_ = id;
}

int image::at(size_t i, size_t j) const {
    return data_[i * width_ + j];
}

int& image::at(size_t i, size_t j) {
    return data_[i * width_ + j];
}

void image::set(size_t i, size_t j, int value) {
    if (i < height_ && j < width_)
        at(i, j) = value;
}

void image::square_around(size_t i, size_t j, int value, size_t shift) {
    for (size_t y = i - shift; y != i + shift; ++y) {
        set(y, j - shift, value);
        set(y, j + shift, value);
    }
    for (size_t x = j - shift; x != j + shift; ++x) {
        set(i - shift, x, value);
        set(i + shift, x, value);
    }
}

std::vector<std::pair<size_t, size_t>> image::min_positions(int multiplier) const {
    int min_value = 255;
    std::vector<std::pair<size_t, size_t>> result;

    for (size_t i = 0; i < height_; ++i) {
        for (size_t j = 0; j < width_; ++j) {
            int current = at(i, j) * multiplier;
            if (current < min_value) {
                min_value = current;
                result.clear();
            }
            if (current == min_value) {
                result.push_back(std::make_pair(i, j));
            }
        }
    }
    return result;
}

std::vector<std::pair<size_t, size_t>> image::min_positions() const {
    return min_positions(1);
}

std::vector<std::pair<size_t, size_t>> image::max_positions() const {
    return min_positions(-1);
}

std::vector<std::pair<size_t, size_t>> image::positions(size_t value) const {
    std::vector<std::pair<size_t, size_t>> result;
    for (size_t i = 0; i < height_; ++i) {
        for (size_t j = 0; j < width_; ++j) {
            if (at(i, j) == value) {
                result.push_back(std::make_pair(i, j));
            }
        }
    }
    return result;
}

float image::mean() const {
    int sum = 0;
    for (size_t i = 0; i < height_; ++i) {
        for (size_t j = 0; j < width_; ++j) {
            sum += at(i, j);
        }
    }
    return sum * 1.0f / height_ / width_;
}

size_t image::get_id() const {
    return id_;
}

std::shared_ptr<image> image::inverse() const {
    std::unique_ptr<int[]> inverse_data(new int[height_ * width_]);
    for (size_t i = 0; i < height_ * width_; ++i) {
        inverse_data[i] = 255 - data_[i];
    }
    return std::shared_ptr<image>(new image(height_, width_, std::move(inverse_data), id_));
}
