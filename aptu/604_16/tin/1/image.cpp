#include "image.hpp"
#include <random>
#include <functional>
#include <algorithm>

Image::Image() : rows_(0), cols_(0), id_(0), data_() {}
Image::Image(size_t rows, size_t cols, int id) : rows_(rows), cols_(cols), id_(id), data_(rows) {
    std::random_device rnd_device;
    std::mt19937 mersenne(rnd_device());
    std::uniform_int_distribution<int> dist(0, max_brightness_);
    auto gen = std::bind(dist, mersenne);
    for (auto& row: data_) {
        row.resize(cols_);
        std::generate(row.begin(), row.end(), gen);
    }
}
Image::Image(int id, const vector<vector<int>>& data)
        try : id_(id), data_(data), rows_(data.size()), cols_(data.at(0).size())
{}
catch (const std::exception& e) {
    std::cout << e.what() << std::endl;
}

void Image::Print() const {
    for (const auto& row: data_) {
        for (const auto& element: row) {
            std::cout << element << " ";
        }
        std::cout << std::endl;
    }
    std::cout << std::endl;
}
int Image::GetId() const {
    return id_;
}

size_t Image::GetCols() const {
    return cols_;
};

size_t Image::GetRows() const {
    return rows_;
};

vector<vector<int>> Image::GetData() const {
    return data_;
}

double Image::GetAverage() const {
    std::vector<int> row_sum;
    for (auto &row: data_){
        row_sum.push_back(std::accumulate(row.begin(), row.end(), 0));
    }
    int overall_sum = std::accumulate(row_sum.begin(), row_sum.end(), 0);
    size_t area = rows_ * cols_;
    return static_cast<double>(overall_sum) / area;
}

void Image::HighlightPixel(size_t rowidx, size_t colidx, size_t square_side) {

    size_t left_row = 0;
    size_t right_row = GetRows();
    size_t left_col = 0;
    size_t right_col = GetCols();
    if (square_side <= rowidx)             left_row  = rowidx - square_side;
    if (GetRows() > rowidx + square_side)  right_row = rowidx + square_side + 1;
    if (square_side <= colidx)             left_col  = colidx - square_side;
    if (GetCols() > colidx + square_side)  right_col = colidx + square_side + 1;
    for (size_t i = left_row; i < right_row; ++i) {
        for (size_t j = left_col; j < right_col; ++j) {
            data_[i][j] = max_brightness_;
        }
    }
}

Image Image::GetInverted() const {
    vector<vector<int>> new_data(rows_);
    for (size_t i = 0; i < rows_; ++i) {
        new_data[i].resize(cols_);
        for (size_t j = 0; j < cols_; ++j)
            new_data[i][j] = max_brightness_ - data_[i][j];
    }
    auto image = Image(id_, new_data);
    return image;
}
