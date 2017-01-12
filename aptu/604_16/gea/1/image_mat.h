//
// Created by eg on 12/11/16.
//

#ifndef GEA_1_IMAGE_MAT_H
#define GEA_1_IMAGE_MAT_H

#include <cstddef>
#include <cstring>
#include <stdexcept>
#include <iostream>
#include <functional>
#include <vector>
#include <numeric>
#include <algorithm>
#include <iomanip>
#include <memory>

struct img_pos {
    size_t row;
    size_t col;

    img_pos(size_t row, size_t col): row(row), col(col) {}

    friend std::ostream& operator<<(std::ostream& out, const img_pos& m) {
        std::cout << "(" << m.row << ", " << m.col << ")";
    }
};

template<typename T>
struct image_mat
{
private:
    size_t m_rows;
    size_t m_cols;
    size_t m_buf_size;
    std::vector<T> m_buffer;

    img_pos to_pos(size_t buf_idx) const {
        return img_pos(buf_idx / m_cols, buf_idx % m_cols);
    }
public:
    image_mat(): image_mat(0, 0) {
    }

    image_mat(size_t rows, size_t cols): m_rows(rows), m_cols(cols), m_buf_size(rows * cols) {
        m_buffer = std::vector<T>(m_buf_size);
    }

    image_mat(size_t rows, size_t cols, std::function<T(size_t, size_t)> filler): image_mat(rows, cols) {
        fill(filler);
    }

    void fill(std::function<T(size_t, size_t)> filler) {
        for (size_t i = 0; i < m_buf_size; ++i) {
            m_buffer[i] = filler(i / m_cols, i % m_cols);
        }
    }

    std::vector<img_pos> get_maxs() const {
        T max = *std::max_element(m_buffer.begin(), m_buffer.end());
        return get_equal_to(max);
    }

    std::vector<img_pos> get_mins() const {
        T min = *std::min_element(m_buffer.begin(), m_buffer.end());
        return get_equal_to(min);
    }

    std::vector<img_pos> get_equal_to(const T& elem) const {
        std::vector<img_pos> res;
        for (size_t i = 0; i < m_buf_size; ++i) {
            if (m_buffer[i] == elem) {
                res.push_back(to_pos(i));
            }
        }
        return res;
    }

    template<typename R>
    R get_mean(R init_val = R()) const {
        R sum = std::accumulate(m_buffer.begin(), m_buffer.end(), init_val);
        return sum / m_buf_size;
    }

    void highlight(std::vector<img_pos> positions, const T& val) {
        static const std::array<std::pair<int, int>, 8> to_fill_shifts =
                {{{-1, -1}, {-1, 0}, {0, -1}, {1, 1}, {1, 0}, {0, 1}, {1, -1}, {-1, 1}}};
        for (auto& p : positions) {
            for (auto& shift : to_fill_shifts) {
                if (p.row + shift.first < m_rows && p.row + shift.first >= 0
                    && p.col + shift.second < m_cols && p.col + shift.second >= 0) {
                    (*this)(p.row + shift.first, p.col + shift.second) = val;
                }
            }
        }
    }

    image_mat<T> invert() const {
        std::function<T(size_t, size_t)> filler = [this](size_t row, size_t col) {
            return ~(*this)(row, col);
        };
        return image_mat<T>(m_rows, m_cols, filler);
    }

    size_t rows() const {
        return m_rows;
    }

    size_t cols() const {
        return m_cols;
    }

    T& operator()(size_t i, size_t j) {
        if (i >= m_rows || j >= m_cols) {
            throw std::runtime_error("index out of bounds");
        }
        return m_buffer[m_cols * i + j];
    }

    const T& operator()(size_t i, size_t j) const {
        if (i >= m_rows || j >= m_cols) {
            throw std::runtime_error("index out of bounds");
        }
        return m_buffer[m_cols * i + j];
    }

    friend std::ostream& operator<<(std::ostream& out, const image_mat& m) {
        for (size_t i = 0; i < m.rows(); ++i) {
            for (size_t j = 0; j < m.cols(); ++j) {
                out << std::setw(6) << m(i, j) << " ";
            }
            out << std::endl;
        }
        return out;
    }
};

#endif //GEA_1_IMAGE_MAT_H
